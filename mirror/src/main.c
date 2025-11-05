#include <stdio.h>
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_log.h"
#include "ws2812.h"

#define STRIP_LED_NUM 6
#define STRIP_GPIO 4
#define STRIP_POWER 20

#define TRI_LED_NUM 9
#define TRI_GPIO 3
#define TRI_POWER 7

#define BUTTON_GPIO 5

#define LASER_POWER 21


static const char *TAG = "LOGGG";

static void button_task(void* arg){
    int led_state = 0;
    int last_button_state = 1;  // 풀업이므로 기본 HIGH

    while(1){
        int current_state = gpio_get_level(BUTTON_GPIO);
        
        // 변화 감지 (HIGH→LOW)`
        if(last_button_state == 1 && current_state == 0){
            
            // 디바운싱: 20ms 후 재확인
            vTaskDelay(pdMS_TO_TICKS(20));
            
            // 여전히 눌려있는지 확인
            if(gpio_get_level(BUTTON_GPIO) == 0){
                
                led_state = !led_state;
                gpio_set_level(LASER_POWER, led_state);
                
            }
        }
        
        last_button_state = current_state;
        vTaskDelay(pdMS_TO_TICKS(10));  // 10ms마다 체크
    }
}


void app_main(void)
{   

    vTaskDelay(pdMS_TO_TICKS(100));
    
    ESP_LOGI("MAIN", "=== Start ===");
    
    // GPIO 리셋부터
    gpio_reset_pin(LASER_POWER);
    gpio_reset_pin(STRIP_POWER);
    gpio_reset_pin(TRI_POWER);
    gpio_reset_pin(BUTTON_GPIO);

    ESP_LOGI(TAG,"LASER_POWER PIN %d", LASER_POWER);
    ESP_LOGI(TAG,"TRI_POWER PIN %d", TRI_POWER);
    ESP_LOGI(TAG,"STRIP_POWER PIN %d", STRIP_POWER);
    ESP_LOGI(TAG,"BTN PIN %d", BUTTON_GPIO);

    
    // 출력 설정
    gpio_set_direction(LASER_POWER, GPIO_MODE_OUTPUT);
    gpio_set_direction(STRIP_POWER, GPIO_MODE_OUTPUT);
    gpio_set_direction(TRI_POWER, GPIO_MODE_OUTPUT);
    
    // 명확히 LOW로
    gpio_set_level(LASER_POWER, 0);
    gpio_set_level(STRIP_POWER, 0);
    gpio_set_level(TRI_POWER, 0);



    neopixel_handle_t *strip = neoPixel_create(STRIP_GPIO, STRIP_LED_NUM);
    neopixel_handle_t *tri = neoPixel_create(TRI_GPIO, TRI_LED_NUM);

    neoPixel_clear(strip, true);
    neoPixel_clear(tri, true);

    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << BUTTON_GPIO),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };

    gpio_config(&io_conf);

    xTaskCreate(button_task, "button", 2048, NULL, 10, NULL);


    gpio_set_level(STRIP_POWER, 1);
    gpio_set_level(TRI_POWER, 1);
    gpio_set_level(LASER_POWER, 0);

    while(1){

        for(int i =0; i<STRIP_LED_NUM; i++){
            neoPixel_set_pixel(strip, i, 170, 30 , 50);
        }
        
        neoPixel_display(strip);
       

        for(int i =0; i<TRI_LED_NUM; i++){
            neoPixel_set_pixel(tri, i, 170, 30 , 50);
        }

        neoPixel_display(tri);
   

    }


}
