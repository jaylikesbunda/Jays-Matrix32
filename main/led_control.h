#ifndef LED_CONTROL_H
#define LED_CONTROL_H

#include "esp_err.h"

void rgb_init(void);
void update_display(void);
void mode_update_task(void *param);

#endif // LED_CONTROL_H 