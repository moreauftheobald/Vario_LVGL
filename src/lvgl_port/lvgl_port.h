#ifndef LVGL_PORT_H
#define LVGL_PORT_H

#include <stdint.h>
#include "esp_err.h"
#include "esp_lcd_types.h"
#include "src/touch/touch.h"
#include "lvgl.h"
#include "src/rgb_lcd_port/rgb_lcd_port.h"
#include "src/gt911/gt911.h"
#include "constants.h"

esp_err_t lvgl_port_init(esp_lcd_panel_handle_t lcd_handle, esp_lcd_touch_handle_t tp_handle);

bool lvgl_port_lock(int timeout_ms);

void lvgl_port_unlock(void);

bool lvgl_port_notify_rgb_vsync(void);

#endif