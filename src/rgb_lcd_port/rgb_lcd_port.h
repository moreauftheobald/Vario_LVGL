#ifndef _RGB_LCD_H_
#define _RGB_LCD_H_

#include "Constants.h"
#include "esp_log.h"
#include "esp_heap_caps.h"
#include "src/io_extension/io_extension.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_panel_rgb.h"

extern SemaphoreHandle_t sem_vsync_end;
extern SemaphoreHandle_t sem_gui_ready;

esp_lcd_panel_handle_t waveshare_esp32_s3_rgb_lcd_init();

void wavesahre_rgb_lcd_bl_on();

void wavesahre_rgb_lcd_bl_off();

void wavesahre_rgb_lcd_set_brightness(uint8_t brightness); 

void wavesahre_rgb_lcd_display_window(int16_t Xstart, int16_t Ystart, int16_t Xend, int16_t Yend, uint8_t *Image);

void wavesahre_rgb_lcd_display(uint8_t *Image);

void waveshare_get_frame_buffer(void **buf1, void **buf2);

#endif