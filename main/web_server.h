#ifndef WEB_SERVER_H
#define WEB_SERVER_H

#include "esp_http_server.h"

httpd_handle_t start_webserver(void);
esp_err_t pixel_handler(httpd_req_t *req);
esp_err_t set_brightness_handler(httpd_req_t *req);
esp_err_t set_mode_handler(httpd_req_t *req);
esp_err_t set_secondary_color_handler(httpd_req_t *req);
esp_err_t get_pixels_handler(httpd_req_t *req);
esp_err_t root_handler(httpd_req_t *req);

#endif // WEB_SERVER_H 