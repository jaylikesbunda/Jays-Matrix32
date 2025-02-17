#include <math.h>
#include "matrix_state.h"

// Global state definitions
pixel_color_t framebuffer[MATRIX_ROWS][MATRIX_COLS] = {{{0}}};
uint8_t current_brightness = DEFAULT_BRIGHTNESS;
pixel_color_t current_color = {255, 0, 0};
pixel_color_t secondary_color = {0, 0, 255};
display_mode_t current_mode = MODE_STATIC;
led_strip_handle_t strip = NULL;
SemaphoreHandle_t rmt_mutex = NULL;

uint8_t scale_brightness(uint8_t value) {
    return (value * current_brightness) / 255;
}

void hsv2rgb(float h, float s, float v, uint8_t *r, uint8_t *g, uint8_t *b) {
    float C = v * s;
    float X = C * (1 - fabsf(fmodf(h / 60.0f, 2) - 1));
    float m = v - C;
    float rp, gp, bp;
    if (h < 60) {
        rp = C; gp = X; bp = 0;
    } else if (h < 120) {
        rp = X; gp = C; bp = 0;
    } else if (h < 180) {
        rp = 0; gp = C; bp = X;
    } else if (h < 240) {
        rp = 0; gp = X; bp = C;
    } else if (h < 300) {
        rp = X; gp = 0; bp = C;
    } else {
        rp = C; gp = 0; bp = X;
    }
    *r = (uint8_t)((rp + m) * 255);
    *g = (uint8_t)((gp + m) * 255);
    *b = (uint8_t)((bp + m) * 255);
} 