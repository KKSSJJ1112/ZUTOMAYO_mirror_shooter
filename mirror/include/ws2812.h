#ifndef WS2812_H
#define WS2812_H
#include "driver/rmt_tx.h"
#include <stdint.h>
#include <stdbool.h>

typedef struct {
    rmt_channel_handle_t rmt_channel;
    rmt_encoder_handle_t encoder;
    uint8_t *pixel_data;
    uint16_t num_pixel;
    uint8_t gpio;
} neopixel_handle_t;



neopixel_handle_t *neoPixel_create(uint8_t gpio, uint8_t num_leds);

void neoPixel_set_pixel(neopixel_handle_t* handle, uint8_t index, uint8_t R, uint8_t G, uint8_t B);

void neoPixel_clear(neopixel_handle_t *handle, bool mode);

void neoPixel_display(neopixel_handle_t *handle);

void neoPixel_destroy(neopixel_handle_t *handle);

#endif