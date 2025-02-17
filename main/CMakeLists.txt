idf_component_register(SRCS "wifi_setup.c" "web_server.c" "led_control.c" "matrix_state.c" "main.c"
                    INCLUDE_DIRS "."
                    REQUIRES "driver" "led_strip" "esp_wifi" "esp_http_server" "nvs_flash" "json" "mdns") 