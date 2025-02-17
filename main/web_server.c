#include <string.h>
#include "esp_log.h"
#include "cJSON.h"
#include "matrix_ui.h"
#include "matrix_state.h"
#include "led_control.h"
#include "web_server.h"

static const char *TAG = "matrix32_web";
static httpd_handle_t server = NULL;

esp_err_t pixel_handler(httpd_req_t *req) {
    ESP_LOGI(TAG, "Pixel handler triggered");
    
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    httpd_resp_set_hdr(req, "Access-Control-Allow-Methods", "POST, GET, OPTIONS");
    
    if (req->method == HTTP_OPTIONS) {
        httpd_resp_send(req, NULL, 0);
        return ESP_OK;
    }
    
    char content[256];
    size_t recv_size = (req->content_len < sizeof(content)) ? req->content_len : sizeof(content);
    
    int ret = httpd_req_recv(req, content, recv_size);
    if (ret <= 0) { return ESP_FAIL; }
    
    cJSON *root = cJSON_Parse(content);
    if (!root) {
        ESP_LOGE(TAG, "Failed to parse JSON: %s", content);
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Invalid JSON");
        return ESP_FAIL;
    }
    
    // Check for fill parameter first
    cJSON *fill = cJSON_GetObjectItem(root, "fill");
    if (fill && cJSON_IsString(fill) && strcmp(fill->valuestring, "yes") == 0) {
        // Get the RGB values
        cJSON *r = cJSON_GetObjectItem(root, "r");
        cJSON *g = cJSON_GetObjectItem(root, "g");
        cJSON *b = cJSON_GetObjectItem(root, "b");
        
        if (r && g && b) {
            // Fill all pixels with the same color
            for (int row = 0; row < MATRIX_ROWS; row++) {
                for (int col = 0; col < MATRIX_COLS; col++) {
                    framebuffer[row][col] = (pixel_color_t){
                        r->valueint,
                        g->valueint,
                        b->valueint
                    };
                }
            }
            ESP_LOGI(TAG, "Filled all pixels with RGB(%d,%d,%d)", 
                    r->valueint, g->valueint, b->valueint);
        }
    } else {
        // Handle normal pixel updates array
        cJSON *updates = cJSON_GetObjectItem(root, "updates");
        if (!updates) {
            ESP_LOGE(TAG, "No 'updates' key in JSON");
            cJSON_Delete(root);
            httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Missing updates");
            return ESP_FAIL;
        }
        
        cJSON *update = NULL;
        cJSON_ArrayForEach(update, updates) {
            uint8_t row = cJSON_GetObjectItem(update, "row")->valueint;
            uint8_t col = cJSON_GetObjectItem(update, "col")->valueint;
            uint8_t r = cJSON_GetObjectItem(update, "r")->valueint;
            uint8_t g = cJSON_GetObjectItem(update, "g")->valueint;
            uint8_t b = cJSON_GetObjectItem(update, "b")->valueint;
            
            framebuffer[row][col] = (pixel_color_t){r, g, b};
            ESP_LOGI(TAG, "Updating pixel (%d,%d) to RGB(%d,%d,%d)", row, col, r, g, b);
        }
    }
    
    cJSON_Delete(root);
    ESP_LOGI(TAG, "Calling update_display");
    update_display();
    
    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, "{\"status\":\"ok\"}", -1);
    return ESP_OK;
}

esp_err_t set_brightness_handler(httpd_req_t *req)
{
    char buf[100];
    int ret = httpd_req_recv(req, buf, sizeof(buf) - 1);
    if (ret <= 0) return ESP_FAIL;
    
    buf[ret] = '\0';
    
    cJSON *root = cJSON_Parse(buf);
    if (root) {
        cJSON *brightness = cJSON_GetObjectItem(root, "brightness");
        if (brightness) {
            current_brightness = (brightness->valueint * 63) / 100;
            update_display();
        }
        cJSON_Delete(root);
    }
    
    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, "{\"status\":\"ok\"}", -1);
    return ESP_OK;
}

esp_err_t set_mode_handler(httpd_req_t *req)
{
    char buf[100];
    int ret = httpd_req_recv(req, buf, sizeof(buf) - 1);
    if (ret <= 0) return ESP_FAIL;
    
    buf[ret] = '\0';
    cJSON *root = cJSON_Parse(buf);
    if (root) {
        cJSON *modeItem = cJSON_GetObjectItem(root, "mode");
        if (modeItem && cJSON_IsString(modeItem)) {
            const char *mode_str = modeItem->valuestring;
            if (strcmp(mode_str, "static") == 0) {
                current_mode = MODE_STATIC;
            } else if (strcmp(mode_str, "rainbow") == 0) {
                current_mode = MODE_RAINBOW;
            } else if (strcmp(mode_str, "checkerboard") == 0) {
                current_mode = MODE_CHECKERBOARD;
            } else if (strcmp(mode_str, "gradient") == 0) {
                current_mode = MODE_GRADIENT;
            } else if (strcmp(mode_str, "random") == 0) {
                current_mode = MODE_RANDOM;
            }
        }
        cJSON_Delete(root);
    }
    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, "{\"status\":\"ok\"}", -1);
    return ESP_OK;
}

esp_err_t set_secondary_color_handler(httpd_req_t *req)
{
    char buf[100];
    int ret = httpd_req_recv(req, buf, sizeof(buf) - 1);
    if (ret <= 0) return ESP_FAIL;
    
    buf[ret] = '\0';
    cJSON *root = cJSON_Parse(buf);
    if (root) {
        cJSON *r_item = cJSON_GetObjectItem(root, "r");
        cJSON *g_item = cJSON_GetObjectItem(root, "g");
        cJSON *b_item = cJSON_GetObjectItem(root, "b");
        if (r_item && g_item && b_item) {
            secondary_color.r = r_item->valueint;
            secondary_color.g = g_item->valueint;
            secondary_color.b = b_item->valueint;
        }
        cJSON_Delete(root);
    }
    
    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, "{\"status\":\"ok\"}", -1);
    return ESP_OK;
}

esp_err_t get_pixels_handler(httpd_req_t *req)
{
    cJSON *root = cJSON_CreateArray();
    
    for(int row=0; row<MATRIX_ROWS; row++) {
        for(int col=0; col<MATRIX_COLS; col++) {
            cJSON *pixel = cJSON_CreateObject();
            cJSON_AddNumberToObject(pixel, "row", row);
            cJSON_AddNumberToObject(pixel, "col", col);
            cJSON_AddNumberToObject(pixel, "r", framebuffer[row][col].r);
            cJSON_AddNumberToObject(pixel, "g", framebuffer[row][col].g);
            cJSON_AddNumberToObject(pixel, "b", framebuffer[row][col].b);
            cJSON_AddItemToArray(root, pixel);
        }
    }
    
    const char *json_str = cJSON_PrintUnformatted(root);
    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, json_str, strlen(json_str));
    
    cJSON_Delete(root);
    free((void*)json_str);
    return ESP_OK;
}

esp_err_t root_handler(httpd_req_t *req)
{
    httpd_resp_set_type(req, "text/html");
    httpd_resp_send(req, MATRIX_UI_HTML, strlen(MATRIX_UI_HTML));
    return ESP_OK;
}

httpd_handle_t start_webserver(void)
{
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    
    httpd_uri_t root = {
        .uri       = "/",
        .method    = HTTP_GET,
        .handler   = root_handler,
        .user_ctx  = NULL
    };
    
    httpd_uri_t pixel = {
        .uri       = "/pixel",
        .method    = HTTP_POST,
        .handler   = pixel_handler,
        .user_ctx  = NULL
    };
    
    httpd_uri_t brightness = {
        .uri       = "/brightness",
        .method    = HTTP_POST,
        .handler   = set_brightness_handler,
        .user_ctx  = NULL
    };
    
    httpd_uri_t mode = {
        .uri       = "/mode",
        .method    = HTTP_POST,
        .handler   = set_mode_handler,
        .user_ctx  = NULL
    };
    
    httpd_uri_t secondary_color_uri = {
        .uri       = "/secondarycolor",
        .method    = HTTP_POST,
        .handler   = set_secondary_color_handler,
        .user_ctx  = NULL
    };

    httpd_uri_t pixels_get_uri = {
        .uri = "/pixels",
        .method = HTTP_GET,
        .handler = get_pixels_handler
    };

    if (httpd_start(&server, &config) == ESP_OK) {
        httpd_register_uri_handler(server, &root);
        httpd_register_uri_handler(server, &pixel);
        httpd_register_uri_handler(server, &brightness);
        httpd_register_uri_handler(server, &mode);
        httpd_register_uri_handler(server, &secondary_color_uri);
        httpd_register_uri_handler(server, &pixels_get_uri);
        return server;
    }
    return NULL;
} 