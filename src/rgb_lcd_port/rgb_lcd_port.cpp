#include <Arduino.h>
#include <stdint.h>
#include <stdbool.h>
#include "rgb_lcd_port.h"
#include "src/lvgl_port/lvgl_port.h"
#include "constants.h"

const char *TAG = "example";

// Semaphores pour synchronisation VSYNC
SemaphoreHandle_t sem_vsync_end = NULL;
SemaphoreHandle_t sem_gui_ready = NULL;

static esp_lcd_panel_handle_t panel_handle = NULL;

// Callback VSYNC - appelé par le hardware lors du vertical blank
static bool IRAM_ATTR on_vsync_event(esp_lcd_panel_handle_t panel,
                                     const esp_lcd_rgb_panel_event_data_t *event_data,
                                     void *user_ctx) {
  BaseType_t high_task_awoken = pdFALSE;
  
  // Vérifier si LVGL a un frame prêt
  if (xSemaphoreTakeFromISR(sem_gui_ready, &high_task_awoken) == pdTRUE) {
    // Signal que le swap peut se faire
    xSemaphoreGiveFromISR(sem_vsync_end, &high_task_awoken);
  }
  
  return high_task_awoken == pdTRUE;
}

IRAM_ATTR static bool rgb_lcd_on_vsync_event(esp_lcd_panel_handle_t panel, const esp_lcd_rgb_panel_event_data_t *edata, void *user_ctx) {
  return lvgl_port_notify_rgb_vsync();
}

esp_lcd_panel_handle_t waveshare_esp32_s3_rgb_lcd_init() {
#ifdef DEBUG_MODE
  ESP_LOGI(TAG, "Install RGB LCD panel driver");
#endif

  esp_lcd_rgb_panel_config_t panel_config = {
    .clk_src = LCD_CLK_SRC_DEFAULT,
    .timings = {
      .pclk_hz = LCD_PIXEL_CLOCK_HZ,
      .h_res = LCD_H_RES,
      .v_res = LCD_V_RES,
      .hsync_pulse_width = 162,
      .hsync_back_porch = 152,
      .hsync_front_porch = 48,
      .vsync_pulse_width = 45,
      .vsync_back_porch = 13,
      .vsync_front_porch = 3,
      .flags = {
        .pclk_active_neg = 1,
      },
    },
    .data_width = RGB_DATA_WIDTH,
    .bits_per_pixel = RGB_BIT_PER_PIXEL,
    .num_fbs = LCD_RGB_BUFFER_NUMS,
    .bounce_buffer_size_px = RGB_BOUNCE_BUFFER_SIZE,
    .sram_trans_align = 4,
    .psram_trans_align = 64,
    .hsync_gpio_num = LCD_IO_RGB_HSYNC,
    .vsync_gpio_num = LCD_IO_RGB_VSYNC,
    .de_gpio_num = LCD_IO_RGB_DE,
    .pclk_gpio_num = LCD_IO_RGB_PCLK,
    .disp_gpio_num = LCD_IO_RGB_DISP,
    .data_gpio_nums = {
      LCD_IO_RGB_DATA0,
      LCD_IO_RGB_DATA1,
      LCD_IO_RGB_DATA2,
      LCD_IO_RGB_DATA3,
      LCD_IO_RGB_DATA4,
      LCD_IO_RGB_DATA5,
      LCD_IO_RGB_DATA6,
      LCD_IO_RGB_DATA7,
      LCD_IO_RGB_DATA8,
      LCD_IO_RGB_DATA9,
      LCD_IO_RGB_DATA10,
      LCD_IO_RGB_DATA11,
      LCD_IO_RGB_DATA12,
      LCD_IO_RGB_DATA13,
      LCD_IO_RGB_DATA14,
      LCD_IO_RGB_DATA15,
    },
    .flags = {
      .fb_in_psram = 1,
    },
  };

  ESP_ERROR_CHECK(esp_lcd_new_rgb_panel(&panel_config, &panel_handle));

#ifdef DEBUG_MODE
  ESP_LOGI(TAG, "Initialize RGB LCD panel");
#endif

  ESP_ERROR_CHECK(esp_lcd_panel_init(panel_handle));

  esp_lcd_rgb_panel_event_callbacks_t cbs = {};
#if RGB_BOUNCE_BUFFER_SIZE > 0
  cbs.on_bounce_frame_finish = rgb_lcd_on_vsync_event;
#else
  cbs.on_vsync = rgb_lcd_on_vsync_event;
#endif

  ESP_ERROR_CHECK(esp_lcd_rgb_panel_register_event_callbacks(panel_handle, &cbs, NULL));

  return panel_handle;
}

void wavesahre_rgb_lcd_display_window(int16_t Xstart, int16_t Ystart, int16_t Xend, int16_t Yend, uint8_t *Image) {
  if (Xstart < 0) Xstart = 0;
  else if (Xend > LCD_H_RES) Xend = LCD_H_RES;

  if (Ystart < 0) Ystart = 0;
  else if (Yend > LCD_V_RES) Yend = LCD_V_RES;

  int crop_width = Xend - Xstart;
  int crop_height = Yend - Ystart;

  uint8_t *dst_data = (uint8_t *)heap_caps_malloc(crop_width * crop_height * 2, MALLOC_CAP_SPIRAM);
  if (!dst_data) {
#ifdef DEBUG_MODE
    printf("Error: Failed to allocate memory for cropped bitmap.\n");
#endif
    return;
  }

  for (int y = 0; y < crop_height; y++) {
    const uint8_t *src_row = Image + ((Ystart + y) * LCD_H_RES + Xstart) * 2;
    uint8_t *dst_row = dst_data + y * crop_width * 2;
    memcpy(dst_row, src_row, crop_width * 2);
  }

  esp_lcd_panel_draw_bitmap(panel_handle, Xstart, Ystart, Xend, Yend, dst_data);

  free(dst_data);
}

void wavesahre_rgb_lcd_display(uint8_t *Image) {
  esp_lcd_panel_draw_bitmap(panel_handle, 0, 0, LCD_H_RES, LCD_V_RES, Image);
}

void waveshare_get_frame_buffer(void **buf1, void **buf2) {
  ESP_ERROR_CHECK(esp_lcd_rgb_panel_get_frame_buffer(panel_handle, 2, buf1, buf2));
}

void wavesahre_rgb_lcd_set_brightness(uint8_t brightness) {
  // Limiter la valeur entre 0 et 100
  if (brightness > 100) {
    brightness = 100;
  }
  
  // Appliquer le PWM via l'IO extension
  IO_EXTENSION_Pwm_Output(100 - brightness);
  
#ifdef DEBUG_MODE
  Serial.printf("Backlight set to %d%%\n", brightness);
#endif
}

void wavesahre_rgb_lcd_bl_on() {
  wavesahre_rgb_lcd_set_brightness(100);
}

void wavesahre_rgb_lcd_bl_off() {
  wavesahre_rgb_lcd_set_brightness(0);
}