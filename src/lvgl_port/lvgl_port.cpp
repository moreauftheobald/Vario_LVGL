#include "lvgl_port.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "src/rgb_lcd_port/rgb_lcd_port.h"

static const char *TAG = "lvgl_port";

// Variables globales
static SemaphoreHandle_t lvgl_mux = NULL;
static esp_lcd_panel_handle_t panel_handle = NULL;
static esp_lcd_touch_handle_t touch_handle = NULL;
static lv_display_t *lvgl_disp = NULL;
static lv_indev_t *lvgl_touch_indev = NULL;

// Callback LVGL flush avec synchronisation VSYNC
static void flush_callback(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map) {
  esp_lcd_panel_handle_t panel = (esp_lcd_panel_handle_t)lv_display_get_user_data(disp);
  
  // Synchronisation VSYNC
  if (sem_gui_ready != NULL) {
    xSemaphoreGive(sem_gui_ready);
    xSemaphoreTake(sem_vsync_end, portMAX_DELAY);
  }
  
  // Swap framebuffer (safe pendant vertical blank)
  int x1 = area->x1;
  int y1 = area->y1;
  int x2 = area->x2 + 1;
  int y2 = area->y2 + 1;
  
  esp_lcd_panel_draw_bitmap(panel, x1, y1, x2, y2, px_map);
  
  lv_display_flush_ready(disp);
}

// Callback lecture tactile
static void touchpad_read(lv_indev_t *indev, lv_indev_data_t *data) {
  esp_lcd_touch_handle_t tp = (esp_lcd_touch_handle_t)lv_indev_get_user_data(indev);
  
  uint16_t touch_x = 0;
  uint16_t touch_y = 0;
  uint8_t touch_cnt = 0;
  
  esp_lcd_touch_read_data(tp);
  bool pressed = esp_lcd_touch_get_coordinates(tp, &touch_x, &touch_y, NULL, &touch_cnt, 1);
  
  if (pressed && touch_cnt > 0) {
    data->point.x = touch_x;
    data->point.y = touch_y;
    data->state = LV_INDEV_STATE_PRESSED;
  } else {
    data->state = LV_INDEV_STATE_RELEASED;
  }
}

// Tâche LVGL principale
static void lvgl_port_task(void *arg) {
#ifdef DEBUG_MODE
  ESP_LOGI(TAG, "LVGL task started");
#endif
  
  while (1) {
    // Lock pour protéger LVGL
    if (lvgl_port_lock(LVGL_PORT_TASK_MAX_DELAY_MS)) {
      uint32_t task_delay_ms = lv_timer_handler();
      lvgl_port_unlock();
      
      // Délai adaptatif selon le temps de traitement LVGL
      if (task_delay_ms > LVGL_PORT_TASK_MAX_DELAY_MS) {
        task_delay_ms = LVGL_PORT_TASK_MAX_DELAY_MS;
      } else if (task_delay_ms < LVGL_PORT_TASK_MIN_DELAY_MS) {
        task_delay_ms = LVGL_PORT_TASK_MIN_DELAY_MS;
      }
      
      vTaskDelay(pdMS_TO_TICKS(task_delay_ms));
    }
  }
}

// Tick timer LVGL
static void increase_lvgl_tick(void *arg) {
  lv_tick_inc(LVGL_PORT_TICK_PERIOD_MS);
}

// Initialisation du display LVGL
static lv_display_t *lvgl_port_display_init(esp_lcd_panel_handle_t panel) {
#ifdef DEBUG_MODE
  ESP_LOGI(TAG, "Initialize LVGL display (Direct Mode, 1024x600)");
#endif
  
  // Obtenir les 2 framebuffers depuis le driver RGB
  void *buf1 = NULL;
  void *buf2 = NULL;
  ESP_ERROR_CHECK(esp_lcd_rgb_panel_get_frame_buffer(panel, 2, &buf1, &buf2));
  
#ifdef DEBUG_MODE
  ESP_LOGI(TAG, "Framebuffer 1: %p", buf1);
  ESP_LOGI(TAG, "Framebuffer 2: %p", buf2);
#endif
  
  // Créer le display LVGL
  lv_display_t *disp = lv_display_create(LVGL_PORT_H_RES, LVGL_PORT_V_RES);
  
  // Configuration: Mode direct avec double framebuffer
  size_t buffer_size = LVGL_PORT_H_RES * LVGL_PORT_V_RES;
  lv_display_set_buffers(disp, buf1, buf2, 
                         buffer_size * sizeof(lv_color_t),
                         LV_DISPLAY_RENDER_MODE_DIRECT);
  
  lv_display_set_flush_cb(disp, flush_callback);
  lv_display_set_user_data(disp, panel);
  
#ifdef DEBUG_MODE
  ESP_LOGI(TAG, "LVGL display configured: %dx%d, Direct mode, 2 framebuffers",
           LVGL_PORT_H_RES, LVGL_PORT_V_RES);
#endif
  
  return disp;
}

// Initialisation du périphérique tactile LVGL
static lv_indev_t *lvgl_port_indev_init(esp_lcd_touch_handle_t tp) {
#ifdef DEBUG_MODE
  ESP_LOGI(TAG, "Initialize LVGL input device");
#endif
  
  lv_indev_t *indev = lv_indev_create();
  lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER);
  lv_indev_set_read_cb(indev, touchpad_read);
  lv_indev_set_user_data(indev, tp);
  
  return indev;
}

// API publique: Initialisation complète
esp_err_t lvgl_port_init(esp_lcd_panel_handle_t lcd_handle, esp_lcd_touch_handle_t tp_handle) {
#ifdef DEBUG_MODE
  ESP_LOGI(TAG, "=== LVGL Port Initialization ===");
#endif
  
  // Sauvegarder les handles
  panel_handle = lcd_handle;
  touch_handle = tp_handle;
  
  // Créer le mutex LVGL
  lvgl_mux = xSemaphoreCreateRecursiveMutex();
  if (lvgl_mux == NULL) {
    ESP_LOGE(TAG, "Failed to create LVGL mutex");
    return ESP_FAIL;
  }
  
  // Initialiser LVGL
  lv_init();
  
#ifdef DEBUG_MODE
  ESP_LOGI(TAG, "LVGL initialized (version %d.%d.%d)",
           lv_version_major(), lv_version_minor(), lv_version_patch());
#endif
  
  // Créer le display
  lvgl_disp = lvgl_port_display_init(panel_handle);
  if (lvgl_disp == NULL) {
    ESP_LOGE(TAG, "Failed to create LVGL display");
    return ESP_FAIL;
  }
  
  // Créer l'input device
  if (touch_handle != NULL) {
    lvgl_touch_indev = lvgl_port_indev_init(touch_handle);
    if (lvgl_touch_indev == NULL) {
      ESP_LOGE(TAG, "Failed to create LVGL input device");
      return ESP_FAIL;
    }
  }
  
  // Créer le timer pour le tick LVGL
  const esp_timer_create_args_t lvgl_tick_timer_args = {
    .callback = &increase_lvgl_tick,
    .name = "lvgl_tick"
  };
  esp_timer_handle_t lvgl_tick_timer = NULL;
  ESP_ERROR_CHECK(esp_timer_create(&lvgl_tick_timer_args, &lvgl_tick_timer));
  ESP_ERROR_CHECK(esp_timer_start_periodic(lvgl_tick_timer, LVGL_PORT_TICK_PERIOD_MS * 1000));
  
#ifdef DEBUG_MODE
  ESP_LOGI(TAG, "LVGL tick timer started (%d ms period)", LVGL_PORT_TICK_PERIOD_MS);
#endif
  
  // Créer la tâche LVGL
  BaseType_t ret = xTaskCreatePinnedToCore(
    lvgl_port_task,
    "LVGL",
    LVGL_PORT_TASK_STACK_SIZE,
    NULL,
    LVGL_PORT_TASK_PRIORITY,
    NULL,
    LVGL_PORT_TASK_CORE
  );
  
  if (ret != pdPASS) {
    ESP_LOGE(TAG, "Failed to create LVGL task");
    return ESP_FAIL;
  }
  
#ifdef DEBUG_MODE
  ESP_LOGI(TAG, "LVGL task created (stack: %d, priority: %d, core: %d)",
           LVGL_PORT_TASK_STACK_SIZE, LVGL_PORT_TASK_PRIORITY, LVGL_PORT_TASK_CORE);
  ESP_LOGI(TAG, "=== LVGL Port Ready ===");
#endif
  
  return ESP_OK;
}

// API publique: Lock LVGL
bool lvgl_port_lock(int timeout_ms) {
  const TickType_t timeout_ticks = (timeout_ms < 0) ? portMAX_DELAY : pdMS_TO_TICKS(timeout_ms);
  return xSemaphoreTakeRecursive(lvgl_mux, timeout_ticks) == pdTRUE;
}

// API publique: Unlock LVGL
void lvgl_port_unlock(void) {
  xSemaphoreGiveRecursive(lvgl_mux);
}

// Notification VSYNC (pas utilisé dans notre config mais gardé pour compatibilité)
bool lvgl_port_notify_rgb_vsync(void) {
  return false;
}