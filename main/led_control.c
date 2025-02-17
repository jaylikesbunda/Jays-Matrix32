#include <stdio.h>
#include <math.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/rmt_tx.h"
#include "esp_log.h"
#include "led_strip.h"
#include "matrix_state.h"
#include "led_control.h"

static const char *TAG = "matrix32";

void rgb_init(void)
{
    led_strip_config_t strip_config = {
        .strip_gpio_num = RGB_CONTROL_PIN,
        .max_leds = RGB_COUNT,
        .led_pixel_format = LED_PIXEL_FORMAT_GRB,
        .led_model = LED_MODEL_WS2812
    };
    
    led_strip_rmt_config_t rmt_config = {
        .resolution_hz = 10 * 1000 * 1000,
        .flags.with_dma = false
    };
    
    ESP_LOGI(TAG, "Initializing LED strip on GPIO %d", RGB_CONTROL_PIN);
    esp_err_t err = led_strip_new_rmt_device(&strip_config, &rmt_config, &strip);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "LED strip init failed: %s", esp_err_to_name(err));
    }
    rmt_mutex = xSemaphoreCreateMutex();
}

void update_display(void)
{
    ESP_LOGI(TAG, "Attempting display update");
    
    if (current_mode == MODE_STATIC) {
        for (int row = 0; row < MATRIX_ROWS; row++) {
            for (int col = 0; col < MATRIX_COLS; col++) {
                int led_index = row * MATRIX_COLS + col;
                pixel_color_t color = framebuffer[row][col];
                ESP_ERROR_CHECK(led_strip_set_pixel(strip, led_index,
                    scale_brightness(color.g),
                    scale_brightness(color.r),
                    scale_brightness(color.b)));
            }
        }
    }

    if (xSemaphoreTake(rmt_mutex, pdMS_TO_TICKS(100)) == pdTRUE) {
        ESP_LOGI(TAG, "Mutex acquired, refreshing strip");
        esp_err_t ret = led_strip_refresh(strip);
        xSemaphoreGive(rmt_mutex);
        
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "Refresh failed: %s", esp_err_to_name(ret));
        }
    } else {
        ESP_LOGW(TAG, "Failed to acquire RMT mutex");
    }
}

void mode_update_task(void *param) {
    ESP_LOGI(TAG, "Mode task started");
    float hue_offset = 0;
    while (1) {
        ESP_LOGI(TAG, "Mode loop iteration - current mode: %d", current_mode);
        switch (current_mode) {
            case MODE_STATIC:
                vTaskDelay(100 / portTICK_PERIOD_MS);
                break;
            case MODE_RAINBOW:
                for (int i = 0; i < RGB_COUNT; i++) {
                    float h = fmodf(hue_offset + (i * (360.0f / RGB_COUNT)), 360.0f);
                    uint8_t r_val, g_val, b_val;
                    hsv2rgb(h, 1.0f, 1.0f, &r_val, &g_val, &b_val);
                    ESP_ERROR_CHECK(led_strip_set_pixel(strip, i, 
                        scale_brightness(r_val),
                        scale_brightness(g_val),
                        scale_brightness(b_val)));
                }
                ESP_ERROR_CHECK(led_strip_refresh(strip));
                hue_offset += 2.0f;
                if (hue_offset >= 360.0f) hue_offset -= 360.0f;
                vTaskDelay(50 / portTICK_PERIOD_MS);
                break;
            case MODE_CHECKERBOARD:
                for (int row = 0; row < MATRIX_ROWS; row++) {
                    for (int col = 0; col < MATRIX_COLS; col++) {
                        int led_index = row * MATRIX_COLS + col;
                        pixel_color_t color;
                        if ((row + col) % 2 == 0) {
                            color = current_color;
                        } else {
                            color = secondary_color;
                        }
                        ESP_ERROR_CHECK(led_strip_set_pixel(strip, led_index,
                            scale_brightness(color.g),
                            scale_brightness(color.r),
                            scale_brightness(color.b)));
                    }
                }
                ESP_ERROR_CHECK(led_strip_refresh(strip));
                vTaskDelay(500 / portTICK_PERIOD_MS);
                break;
            case MODE_GRADIENT: {
                static uint8_t gradient_offset = 0;
                for (int row = 0; row < MATRIX_ROWS; row++) {
                    for (int col = 0; col < MATRIX_COLS; col++) {
                        int led_index = row * MATRIX_COLS + col;
                        float factor = ((float)((col + gradient_offset) % MATRIX_COLS)) / (MATRIX_COLS - 1);
                        uint8_t r = (uint8_t)(current_color.r * (1 - factor) + secondary_color.r * factor);
                        uint8_t g = (uint8_t)(current_color.g * (1 - factor) + secondary_color.g * factor);
                        uint8_t b = (uint8_t)(current_color.b * (1 - factor) + secondary_color.b * factor);
                        r = scale_brightness(r);
                        g = scale_brightness(g);
                        b = scale_brightness(b);
                        ESP_ERROR_CHECK(led_strip_set_pixel(strip, led_index, r, g, b));
                    }
                }
                gradient_offset = (gradient_offset + 1) % MATRIX_COLS;
                ESP_ERROR_CHECK(led_strip_refresh(strip));
                vTaskDelay(100 / portTICK_PERIOD_MS);
                break;
            }
            case MODE_RANDOM:
                for (int i = 0; i < RGB_COUNT; i++) {
                    uint8_t r_rand = (uint8_t)(rand() % 256);
                    uint8_t g_rand = (uint8_t)(rand() % 256);
                    uint8_t b_rand = (uint8_t)(rand() % 256);
                    ESP_ERROR_CHECK(led_strip_set_pixel(strip, i,
                        scale_brightness(r_rand),
                        scale_brightness(g_rand),
                        scale_brightness(b_rand)));
                }
                ESP_ERROR_CHECK(led_strip_refresh(strip));
                vTaskDelay(200 / portTICK_PERIOD_MS);
                break;
        }
    }
} 