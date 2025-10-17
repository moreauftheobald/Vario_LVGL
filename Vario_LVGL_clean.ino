#define DEBUG_MODE
#define LV_CONF_INCLUDE_SIMPLE

#include "lv_conf.h"
#include "src/lvgl_port/lvgl_port.h"
#include "src/rgb_lcd_port/rgb_lcd_port.h"
#include "constants.h"
#include "lang.h"
#include "globals.h"
#include "src/ui/ui_splash.h"
#include "src/ui/ui_prestart.h"
#include "src/ui/ui_settings.h"
#include "src/ui/ui_file_transfer.h"
#include "src/ui/ui_settings_pilot.h"
#include "src/ui/ui_settings_screen.h"
#include "src/ui/ui_settings_wifi.h"
#include "src/ui/ui_settings_map.h"

// Definition des variables globales
Preferences prefs;
lv_obj_t *ta_active = NULL;
lv_obj_t *keyboard = NULL;

void force_full_refresh(void) {
    lv_obj_invalidate(lv_screen_active());
    lv_refr_now(NULL);
    vTaskDelay(pdMS_TO_TICKS(10));
}

void setup() {
#ifdef DEBUG_MODE
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  Serial.println("Starting Vario...");
  Serial.printf("Project: %s\n", VARIO_NAME);
  Serial.printf("Version: %s\n", VARIO_VERSION);
  Serial.printf("Free heap: %d bytes\n", ESP.getFreeHeap());
  Serial.printf("Free PSRAM: %d bytes\n", ESP.getFreePsram());
#endif

  // Initialize hardware
  static esp_lcd_panel_handle_t panel_handle = NULL;
  static esp_lcd_touch_handle_t tp_handle = NULL;

  tp_handle = touch_gt911_init();
  panel_handle = waveshare_esp32_s3_rgb_lcd_init();
  wavesahre_rgb_lcd_bl_on();

#ifdef DEBUG_MODE
  Serial.printf("Free heap after LCD init: %d bytes\n", ESP.getFreeHeap());
#endif

  esp_err_t ret = lvgl_port_init(panel_handle, tp_handle);
  if (ret != ESP_OK) {
#ifdef DEBUG_MODE
    Serial.printf("LVGL init failed with error: 0x%x\n", ret);
    Serial.printf("Free heap: %d bytes\n", ESP.getFreeHeap());
    Serial.printf("Free PSRAM: %d bytes\n", ESP.getFreePsram());
#endif
    while (1) vTaskDelay(pdMS_TO_TICKS(1000));
  }

#ifdef DEBUG_MODE
  Serial.println("Hardware initialized");
  Serial.printf("Free heap after LVGL init: %d bytes\n", ESP.getFreeHeap());
  Serial.printf("Free PSRAM: %d bytes\n", ESP.getFreePsram());
#endif

  // Create splash UI
  if (lvgl_port_lock(-1)) {
    ui_splash_show();
    lvgl_port_unlock();
  }

#ifdef DEBUG_MODE
  Serial.println("Setup complete");
#endif
}

void loop() {
#ifdef DEBUG_MODE
  static unsigned long last_print = 0;
  if (millis() - last_print > 5000) {
    //Serial.printf("Free heap: %d bytes\n", ESP.getFreeHeap());
    //Serial.printf("Min free heap: %d bytes\n", ESP.getMinFreeHeap());
    //Serial.printf("Free PSRAM: %d bytes\n", ESP.getFreePsram());
    last_print = millis();
  }
#endif
  vTaskDelay(pdMS_TO_TICKS(200));
}