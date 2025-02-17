#include <stdio.h>
#include "nvs_flash.h"
#include "esp_log.h"
#include "matrix_state.h"
#include "led_control.h"
#include "wifi_setup.h"
#include "web_server.h"

void app_main(void)
{
    // Initialize NVS for WiFi storage
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    
    rgb_init();
    wifi_init_softap();
    start_webserver();

    // Start the unified display update task
    xTaskCreate(mode_update_task, "mode_update", 4096, NULL, 5, NULL);
}