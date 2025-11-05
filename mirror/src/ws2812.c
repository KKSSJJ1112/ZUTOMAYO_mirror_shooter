#include "ws2812.h"
#include "driver/rmt_tx.h"
#include "esp_log.h"
#include <string.h>
#include <stdlib.h>

static const char *TAG = "WS2812";




neopixel_handle_t *neoPixel_create(uint8_t gpio, uint8_t num_leds){

    neopixel_handle_t *handle = (neopixel_handle_t*)malloc(sizeof(neopixel_handle_t));
    if(handle == NULL){
        ESP_LOGE(TAG, "failed allocate memory");
        return NULL;
    }

    handle->num_pixel = num_leds;
    handle->gpio = gpio;

    handle->pixel_data = (uint8_t*)calloc(num_leds * 3, sizeof(uint8_t));
    if(handle->pixel_data == NULL){
        ESP_LOGE(TAG, "pixel data allocate failed");
        free(handle);
        return NULL;
    }    // RMT 채널 설정
    rmt_tx_channel_config_t tx_config = {
        .gpio_num = gpio,
        .clk_src = RMT_CLK_SRC_DEFAULT,
        .resolution_hz = 10000000,
        .mem_block_symbols = 48,
        .trans_queue_depth = 1,
    };



    
    esp_err_t ret = rmt_new_tx_channel(&tx_config, &handle->rmt_channel);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "RMT 채널 생성 실패");
        free(handle->pixel_data);
        free(handle);
        return NULL;
    }
    
    // 바이트 인코더 설정
    rmt_bytes_encoder_config_t encoder_config = {
        .bit0 = {
            .level0 = 1,
            .duration0 = 3,
            .level1 = 0,
            .duration1 = 9,
        },
        .bit1 = {
            .level0 = 1,
            .duration0 = 9,
            .level1 = 0,
            .duration1 = 3,
        },
        .flags.msb_first = 1
    };
    
    ret = rmt_new_bytes_encoder(&encoder_config, &handle->encoder);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "인코더 생성 실패");
        rmt_del_channel(handle->rmt_channel);
        free(handle->pixel_data);
        free(handle);
        return NULL;
    }
    
    // RMT 채널 활성화
    ret = rmt_enable(handle->rmt_channel);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "RMT 활성화 실패");
        return NULL;
    }
    
    ESP_LOGI(TAG, "네오픽셀 생성: GPIO %d, %d LEDs", gpio, num_leds);
    return handle;


}




void neoPixel_set_pixel(neopixel_handle_t* handle, uint8_t index, uint8_t R, uint8_t G, uint8_t B){

    if (handle == NULL || index >= handle->num_pixel) {
        return;
    }
    
    // GRB 순서로 저장
    handle->pixel_data[index * 3 + 0] = G;
    handle->pixel_data[index * 3 + 1] = R;
    handle->pixel_data[index * 3 + 2] = B;

}




void neoPixel_display(neopixel_handle_t *handle){

    if (handle == NULL) return;

    rmt_transmit_config_t tx_config = {
        .loop_count =0,
    };

    esp_err_t ret = rmt_transmit(
        handle->rmt_channel,
        handle->encoder,
        handle->pixel_data,
        handle->num_pixel * 3,
        &tx_config
    );

    if(ret != ESP_OK){
        ESP_LOGE(TAG, "tx failed");
        return;
    }

    rmt_tx_wait_all_done(handle->rmt_channel, 50);

}



void neoPixel_clear(neopixel_handle_t *handle, bool mode){

    if (handle == NULL) return;

    memset(handle->pixel_data, 0, handle->num_pixel*3);

    if(mode){
        neoPixel_display(handle);
    }

}




void neoPixel_destroy(neopixel_handle_t *handle){

    if(handle==NULL) return;

    rmt_disable(handle->rmt_channel);
    rmt_del_encoder(handle->encoder);
    rmt_del_channel(handle->rmt_channel);
    free(handle->pixel_data);
    free(handle);
}