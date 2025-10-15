#if defined(ARDUINO)
#include <Arduino.h>
#endif
#ifndef __cplusplus
#error "This file must be compiled as C++"
#endif

#include <stdint.h>
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/task.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_panel_rgb.h"
#include "esp_timer.h"
#include "esp_log.h"
#include "src/lvgl_port/lvgl_port.h"

// static const char *TAG = "lv_port";
static SemaphoreHandle_t lvgl_mux;
static TaskHandle_t lvgl_task_handle = NULL;

#if EXAMPLE_LVGL_PORT_ROTATION_DEGREE != 0

static void *get_next_frame_buffer(esp_lcd_panel_handle_t panel_handle) {
  static void *next_fb = NULL;
  static void *fb[2] = { NULL };
  if (next_fb == NULL) {
    ESP_ERROR_CHECK(esp_lcd_rgb_panel_get_frame_buffer(panel_handle, 2, &fb[0], &fb[1]));
    next_fb = fb[1];
  } else {
    next_fb = (next_fb == fb[0]) ? fb[1] : fb[0];
  }
  return next_fb;
}

IRAM_ATTR static void rotate_copy_pixel(const uint16_t *from, uint16_t *to, uint16_t x_start, uint16_t y_start, uint16_t x_end, uint16_t y_end, uint16_t w, uint16_t h, uint16_t rotation) {
  int from_index = 0;
  int to_index = 0;
  int to_index_const = 0;

  switch (rotation) {
    case 90:
      to_index_const = (w - x_start - 1) * h;
      for (int from_y = y_start; from_y < y_end + 1; from_y++) {
        from_index = from_y * w + x_start;
        to_index = to_index_const + from_y;
        for (int from_x = x_start; from_x < x_end + 1; from_x++) {
          *(to + to_index) = *(from + from_index);
          from_index += 1;
          to_index -= h;
        }
      }
      break;
    case 180:
      to_index_const = h * w - x_start - 1;
      for (int from_y = y_start; from_y < y_end + 1; from_y++) {
        from_index = from_y * w + x_start;
        to_index = to_index_const - from_y * w;
        for (int from_x = x_start; from_x < x_end + 1; from_x++) {
          *(to + to_index) = *(from + from_index);
          from_index += 1;
          to_index -= 1;
        }
      }
      break;
    case 270:
      to_index_const = (x_start + 1) * h - 1;
      for (int from_y = y_start; from_y < y_end + 1; from_y++) {
        from_index = from_y * w + x_start;
        to_index = to_index_const - from_y;
        for (int from_x = x_start; from_x < x_end + 1; from_x++) {
          *(to + to_index) = *(from + from_index);
          from_index += 1;
          to_index += h;
        }
      }
      break;
    default:
      break;
  }
}
#endif

#if LVGL_PORT_AVOID_TEAR_ENABLE
#if LVGL_PORT_DIRECT_MODE
#if EXAMPLE_LVGL_PORT_ROTATION_DEGREE != 0

typedef struct {
  uint16_t inv_p;
  uint8_t inv_area_joined[LV_INV_BUF_SIZE];
  lv_area_t inv_areas[LV_INV_BUF_SIZE];
} lv_port_dirty_area_t;

typedef enum {
  FLUSH_STATUS_PART,
  FLUSH_STATUS_FULL
} lv_port_flush_status_t;

typedef enum {
  FLUSH_PROBE_PART_COPY,
  FLUSH_PROBE_SKIP_COPY,
  FLUSH_PROBE_FULL_COPY,
} lv_port_flush_probe_t;

static lv_port_dirty_area_t dirty_area;

static void flush_dirty_save(lv_port_dirty_area_t *dirty_area) {
  lv_display_t *disp = lv_display_get_default();
  if (disp == NULL) return;

  dirty_area->inv_p = 0;
}

static lv_port_flush_probe_t flush_copy_probe(lv_display_t *disp, const lv_area_t *area) {
  static lv_port_flush_status_t prev_status = FLUSH_STATUS_PART;
  lv_port_flush_status_t cur_status;
  lv_port_flush_probe_t probe_result;

  int32_t hor_res = lv_display_get_horizontal_resolution(disp);
  int32_t ver_res = lv_display_get_vertical_resolution(disp);

  uint32_t flush_ver = area->y2 - area->y1 + 1;
  uint32_t flush_hor = area->x2 - area->x1 + 1;

  cur_status = ((flush_ver == ver_res) && (flush_hor == hor_res)) ? (FLUSH_STATUS_FULL) : (FLUSH_STATUS_PART);

  if (prev_status == FLUSH_STATUS_FULL) {
    if ((cur_status == FLUSH_STATUS_PART)) {
      probe_result = FLUSH_PROBE_FULL_COPY;
    } else {
      probe_result = FLUSH_PROBE_SKIP_COPY;
    }
  } else {
    probe_result = FLUSH_PROBE_PART_COPY;
  }
  prev_status = cur_status;

  return probe_result;
}

static inline void *flush_get_next_buf(void *panel_handle) {
  return get_next_frame_buffer(panel_handle);
}

static void flush_dirty_copy(void *dst, void *src, lv_port_dirty_area_t *dirty_area) {
  lv_coord_t x_start, x_end, y_start, y_end;
  for (int i = 0; i < dirty_area->inv_p; i++) {
    if (dirty_area->inv_area_joined[i] == 0) {
      x_start = dirty_area->inv_areas[i].x1;
      x_end = dirty_area->inv_areas[i].x2;
      y_start = dirty_area->inv_areas[i].y1;
      y_end = dirty_area->inv_areas[i].y2;

      rotate_copy_pixel((uint16_t *)src, (uint16_t *)dst, x_start, y_start, x_end, y_end, LV_HOR_RES, LV_VER_RES, EXAMPLE_LVGL_PORT_ROTATION_DEGREE);
    }
  }
}

static void flush_callback(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map) {
  esp_lcd_panel_handle_t panel_handle = (esp_lcd_panel_handle_t)lv_display_get_user_data(disp);
  lv_color_t *color_map = (lv_color_t *)px_map;

  const int offsetx1 = area->x1;
  const int offsetx2 = area->x2;
  const int offsety1 = area->y1;
  const int offsety2 = area->y2;
  void *next_fb = NULL;
  lv_port_flush_probe_t probe_result = FLUSH_PROBE_PART_COPY;

  if (lv_display_flush_is_last(disp)) {
    static bool full_refresh_pending = false;

    if (full_refresh_pending) {
      full_refresh_pending = false;

      next_fb = flush_get_next_buf(panel_handle);
      rotate_copy_pixel((uint16_t *)color_map, (uint16_t *)next_fb, offsetx1, offsety1, offsetx2, offsety2, LV_HOR_RES, LV_VER_RES, EXAMPLE_LVGL_PORT_ROTATION_DEGREE);

      esp_lcd_panel_draw_bitmap(panel_handle, offsetx1, offsety1, offsetx2 + 1, offsety2 + 1, next_fb);

      ulTaskNotifyValueClear(NULL, ULONG_MAX);
      ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

      flush_dirty_copy(flush_get_next_buf(panel_handle), color_map, &dirty_area);
      flush_get_next_buf(panel_handle);
    } else {
      probe_result = flush_copy_probe(disp, area);

      if (probe_result == FLUSH_PROBE_FULL_COPY) {
        flush_dirty_save(&dirty_area);
        full_refresh_pending = true;
        lv_display_flush_ready(disp);
        lv_obj_invalidate(lv_screen_active());
        return;
      } else {
        next_fb = flush_get_next_buf(panel_handle);
        flush_dirty_save(&dirty_area);
        flush_dirty_copy(next_fb, color_map, &dirty_area);

        esp_lcd_panel_draw_bitmap(panel_handle, offsetx1, offsety1, offsetx2 + 1, offsety2 + 1, next_fb);

        ulTaskNotifyValueClear(NULL, ULONG_MAX);
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

        if (probe_result == FLUSH_PROBE_PART_COPY) {
          flush_dirty_save(&dirty_area);
          flush_dirty_copy(flush_get_next_buf(panel_handle), color_map, &dirty_area);
          flush_get_next_buf(panel_handle);
        }
      }
    }
  }

  lv_display_flush_ready(disp);
}

#else

static void flush_callback(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map) {
  esp_lcd_panel_handle_t panel_handle = (esp_lcd_panel_handle_t)lv_display_get_user_data(disp);

  if (lv_display_flush_is_last(disp)) {
    esp_lcd_panel_draw_bitmap(panel_handle, area->x1, area->y1, area->x2 + 1, area->y2 + 1, px_map);

    ulTaskNotifyValueClear(NULL, ULONG_MAX);
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
  }

  lv_display_flush_ready(disp);
}

#endif

#elif LVGL_PORT_FULL_REFRESH && LVGL_PORT_LCD_RGB_BUFFER_NUMS == 2

static void flush_callback(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map) {
  esp_lcd_panel_handle_t panel_handle = (esp_lcd_panel_handle_t)lv_display_get_user_data(disp);

  esp_lcd_panel_draw_bitmap(panel_handle, area->x1, area->y1, area->x2 + 1, area->y2 + 1, px_map);

  ulTaskNotifyValueClear(NULL, ULONG_MAX);
  ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

  lv_display_flush_ready(disp);
}

#elif LVGL_PORT_FULL_REFRESH && LVGL_PORT_LCD_RGB_BUFFER_NUMS == 3

#if EXAMPLE_LVGL_PORT_ROTATION_DEGREE == 0
static void *lvgl_port_rgb_last_buf = NULL;
static void *lvgl_port_rgb_next_buf = NULL;
static void *lvgl_port_flush_next_buf = NULL;
#endif

static void flush_callback(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map) {
  esp_lcd_panel_handle_t panel_handle = (esp_lcd_panel_handle_t)lv_display_get_user_data(disp);
  lv_color_t *color_map = (lv_color_t *)px_map;

  const int offsetx1 = area->x1;
  const int offsety1 = area->y1;
  const int offsetx2 = area->x2;
  const int offsety2 = area->y2;

#if EXAMPLE_LVGL_PORT_ROTATION_DEGREE != 0
  void *next_fb = get_next_frame_buffer(panel_handle);

  rotate_copy_pixel((uint16_t *)color_map, (uint16_t *)next_fb, offsetx1, offsety1, offsetx2, offsety2, LV_HOR_RES, LV_VER_RES, EXAMPLE_LVGL_PORT_ROTATION_DEGREE);

  esp_lcd_panel_draw_bitmap(panel_handle, offsetx1, offsety1, offsetx2 + 1, offsety2 + 1, next_fb);
#else
  lvgl_port_flush_next_buf = color_map;

  esp_lcd_panel_draw_bitmap(panel_handle, offsetx1, offsety1, offsetx2 + 1, offsety2 + 1, color_map);

  lvgl_port_rgb_next_buf = color_map;
#endif

  lv_display_flush_ready(disp);
}

#endif

#else

static void flush_callback(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map) {
  esp_lcd_panel_handle_t panel_handle = (esp_lcd_panel_handle_t)lv_display_get_user_data(disp);

  esp_lcd_panel_draw_bitmap(panel_handle, area->x1, area->y1, area->x2 + 1, area->y2 + 1, px_map);

  lv_display_flush_ready(disp);
}

#endif

static lv_display_t *display_init(esp_lcd_panel_handle_t panel_handle) {
  assert(panel_handle);

  void *buf1 = NULL;
  void *buf2 = NULL;
  int buffer_size = 0;

#ifdef DEBUG_MODE
  ESP_LOGD(TAG, "Malloc memory for LVGL buffer");
#endif

#if LVGL_PORT_AVOID_TEAR_ENABLE
  buffer_size = LVGL_PORT_H_RES * LVGL_PORT_V_RES;
#if (LVGL_PORT_LCD_RGB_BUFFER_NUMS == 3) && (EXAMPLE_LVGL_PORT_ROTATION_DEGREE == 0) && LVGL_PORT_FULL_REFRESH
  ESP_ERROR_CHECK(esp_lcd_rgb_panel_get_frame_buffer(panel_handle, 3, &lvgl_port_rgb_last_buf, &buf1, &buf2));
  lvgl_port_rgb_next_buf = lvgl_port_rgb_last_buf;
  lvgl_port_flush_next_buf = buf2;
#elif (LVGL_PORT_LCD_RGB_BUFFER_NUMS == 3) && (EXAMPLE_LVGL_PORT_ROTATION_DEGREE != 0)
  void *fbs[3];
  ESP_ERROR_CHECK(esp_lcd_rgb_panel_get_frame_buffer(panel_handle, 3, &fbs[0], &fbs[1], &fbs[2]));
  buf1 = fbs[2];
#else
  ESP_ERROR_CHECK(esp_lcd_rgb_panel_get_frame_buffer(panel_handle, 2, &buf1, &buf2));
#endif
#else
  buffer_size = LVGL_PORT_H_RES * LVGL_PORT_BUFFER_HEIGHT;
  buf1 = heap_caps_malloc(buffer_size * sizeof(lv_color_t), LVGL_PORT_BUFFER_MALLOC_CAPS);
  assert(buf1);
#ifdef DEBUG_MODE
  ESP_LOGI(TAG, "LVGL buffer size: %dKB", buffer_size * sizeof(lv_color_t) / 1024);
#endif
#endif

#ifdef DEBUG_MODE
  ESP_LOGD(TAG, "Register display driver to LVGL");
#endif

#if EXAMPLE_LVGL_PORT_ROTATION_90 || EXAMPLE_LVGL_PORT_ROTATION_270
  lv_display_t *disp = lv_display_create(LVGL_PORT_V_RES, LVGL_PORT_H_RES);
#else
  lv_display_t *disp = lv_display_create(LVGL_PORT_H_RES, LVGL_PORT_V_RES);
#endif

  lv_display_set_flush_cb(disp, flush_callback);
  lv_display_set_buffers(disp, buf1, buf2, buffer_size * sizeof(lv_color_t), LV_DISPLAY_RENDER_MODE_PARTIAL);
  lv_display_set_user_data(disp, panel_handle);

#if LVGL_PORT_FULL_REFRESH
  lv_display_set_render_mode(disp, LV_DISPLAY_RENDER_MODE_FULL);
#elif LVGL_PORT_DIRECT_MODE
  lv_display_set_render_mode(disp, LV_DISPLAY_RENDER_MODE_DIRECT);
#endif

  return disp;
}

static void touchpad_read(lv_indev_t *indev, lv_indev_data_t *data) {
  esp_lcd_touch_handle_t tp = (esp_lcd_touch_handle_t)lv_indev_get_user_data(indev);
  assert(tp);

  uint16_t touchpad_x;
  uint16_t touchpad_y;
  uint8_t touchpad_cnt = 0;

  esp_lcd_touch_read_data(tp);

  bool touchpad_pressed = esp_lcd_touch_get_coordinates(tp, &touchpad_x, &touchpad_y, NULL, &touchpad_cnt, 1);
  if (touchpad_pressed && touchpad_cnt > 0) {
    data->point.x = touchpad_x;
    data->point.y = touchpad_y;
    data->state = LV_INDEV_STATE_PRESSED;
#ifdef DEBUG_MODE
    ESP_LOGD(TAG, "Touch position: %d,%d", touchpad_x, touchpad_y);
#endif
  } else {
    data->state = LV_INDEV_STATE_RELEASED;
  }
}

static lv_indev_t *indev_init(esp_lcd_touch_handle_t tp) {
  assert(tp);

  lv_indev_t *indev = lv_indev_create();
  lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER);
  lv_indev_set_read_cb(indev, touchpad_read);
  lv_indev_set_user_data(indev, tp);

  return indev;
}

static void tick_increment(void *arg) {
  lv_tick_inc(LVGL_PORT_TICK_PERIOD_MS);
}

static esp_err_t tick_init(void) {
  esp_timer_create_args_t lvgl_tick_timer_args = {};
  lvgl_tick_timer_args.callback = &tick_increment;
  lvgl_tick_timer_args.name = "LVGL tick";

  esp_timer_handle_t lvgl_tick_timer = NULL;
  ESP_ERROR_CHECK(esp_timer_create(&lvgl_tick_timer_args, &lvgl_tick_timer));
  return esp_timer_start_periodic(lvgl_tick_timer, LVGL_PORT_TICK_PERIOD_MS * 1000);
}

static void lvgl_port_task(void *arg) {
#ifdef DEBUG_MODE
  ESP_LOGD(TAG, "Starting LVGL task");
#endif

  uint32_t task_delay_ms = LVGL_PORT_TASK_MAX_DELAY_MS;
  while (1) {
    if (lvgl_port_lock(-1)) {
      task_delay_ms = lv_timer_handler();
      lvgl_port_unlock();
    }
    if (task_delay_ms > LVGL_PORT_TASK_MAX_DELAY_MS) {
      task_delay_ms = LVGL_PORT_TASK_MAX_DELAY_MS;
    } else if (task_delay_ms < LVGL_PORT_TASK_MIN_DELAY_MS) {
      task_delay_ms = LVGL_PORT_TASK_MIN_DELAY_MS;
    }
    vTaskDelay(pdMS_TO_TICKS(task_delay_ms));
  }
}

esp_err_t lvgl_port_init(esp_lcd_panel_handle_t lcd_handle, esp_lcd_touch_handle_t tp_handle) {
  lv_init();
  ESP_ERROR_CHECK(tick_init());

  lv_display_t *disp = display_init(lcd_handle);
  assert(disp);

  if (tp_handle) {
    lv_indev_t *indev = indev_init(tp_handle);
    assert(indev);

#if EXAMPLE_LVGL_PORT_ROTATION_90
    esp_lcd_touch_set_swap_xy(tp_handle, true);
    esp_lcd_touch_set_mirror_y(tp_handle, true);
#elif EXAMPLE_LVGL_PORT_ROTATION_180
    esp_lcd_touch_set_mirror_x(tp_handle, true);
    esp_lcd_touch_set_mirror_y(tp_handle, true);
#elif EXAMPLE_LVGL_PORT_ROTATION_270
    esp_lcd_touch_set_swap_xy(tp_handle, true);
    esp_lcd_touch_set_mirror_x(tp_handle, true);
#endif
  }

  lvgl_mux = xSemaphoreCreateRecursiveMutex();
  assert(lvgl_mux);

#ifdef DEBUG_MODE
  ESP_LOGI(TAG, "Create LVGL task");
#endif

  BaseType_t core_id = (LVGL_PORT_TASK_CORE < 0) ? tskNO_AFFINITY : LVGL_PORT_TASK_CORE;
  BaseType_t ret = xTaskCreatePinnedToCore(lvgl_port_task, "lvgl", LVGL_PORT_TASK_STACK_SIZE, NULL,
                                           LVGL_PORT_TASK_PRIORITY, &lvgl_task_handle, core_id);
  if (ret != pdPASS) {
#ifdef DEBUG_MODE
    ESP_LOGE(TAG, "Failed to create LVGL task");
#endif
    return ESP_FAIL;
  }

  return ESP_OK;
}

bool lvgl_port_lock(int timeout_ms) {
  assert(lvgl_mux && "lvgl_port_init must be called first");

  const TickType_t timeout_ticks = (timeout_ms < 0) ? portMAX_DELAY : pdMS_TO_TICKS(timeout_ms);
  return xSemaphoreTakeRecursive(lvgl_mux, timeout_ticks) == pdTRUE;
}

void lvgl_port_unlock(void) {
  assert(lvgl_mux && "lvgl_port_init must be called first");
  xSemaphoreGiveRecursive(lvgl_mux);
}

bool lvgl_port_notify_rgb_vsync(void) {
  BaseType_t need_yield = pdFALSE;
#if LVGL_PORT_FULL_REFRESH && (LVGL_PORT_LCD_RGB_BUFFER_NUMS == 3) && (EXAMPLE_LVGL_PORT_ROTATION_DEGREE == 0)
  if (lvgl_port_rgb_next_buf != lvgl_port_rgb_last_buf) {
    lvgl_port_flush_next_buf = lvgl_port_rgb_last_buf;
    lvgl_port_rgb_last_buf = lvgl_port_rgb_next_buf;
  }
#elif LVGL_PORT_AVOID_TEAR_ENABLE
  xTaskNotifyFromISR(lvgl_task_handle, ULONG_MAX, eNoAction, &need_yield);
#endif
  return (need_yield == pdTRUE);
}