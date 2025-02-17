#ifndef MATRIX_STATE_H
#define MATRIX_STATE_H

#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "led_strip.h"

#define RGB_CONTROL_PIN   14
#define MATRIX_ROWS      8
#define MATRIX_COLS      8
#define RGB_COUNT        64
#define DEFAULT_BRIGHTNESS 12.8  // 5% of 255

typedef struct {
    uint8_t r;
    uint8_t g;
    uint8_t b;
} pixel_color_t;

typedef enum {
    MODE_STATIC = 0,
    MODE_RAINBOW,
    MODE_CHECKERBOARD,
    MODE_GRADIENT,
    MODE_RANDOM
} display_mode_t;

// Global state declarations
extern pixel_color_t framebuffer[MATRIX_ROWS][MATRIX_COLS];
extern uint8_t current_brightness;
extern pixel_color_t current_color;
extern pixel_color_t secondary_color;
extern display_mode_t current_mode;
extern led_strip_handle_t strip;
extern SemaphoreHandle_t rmt_mutex;

// Utility functions
uint8_t scale_brightness(uint8_t value);
void hsv2rgb(float h, float s, float v, uint8_t *r, uint8_t *g, uint8_t *b);

#endif // MATRIX_STATE_H 