SET_LOOP_TASK_STACK_SIZE(3 * 1024);

#include "constants.h"
#include "globals.h"
#include "lv_conf.h"
#include "src/lvgl_port/lvgl_port.h"
#include "src/rgb_lcd_port/rgb_lcd_port.h"
#include "lang.h"
#include "src/ui/ui_splash.h"
#include "src/ui/ui_prestart.h"
#include "src/ui/ui_settings.h"
#include "src/ui/ui_settings_pilot.h"
#include "src/ui/ui_settings_screen.h"
#include "src/ui/ui_settings_wifi.h"
#include "src/ui/ui_settings_map.h"
#include "src/ui/ui_settings_vario.h"
#include "src/ui/ui_main_screens.h"
#include "src/ui/ui_settings_system.h"
#include "src/params/params.h"
#include "src/sd_card.h"
#include "src/sensors_i2c_task.h"

void setup() {
  Serial.begin(115200);
#ifdef DEBUG_MODE
  Serial.setDebugOutput(true);
#endif

  // 1. I2C et IO_EXTENSION en PREMIER
  DEV_I2C_Init();

  // Start capteurs I2C
  sensors_i2c_start();

  IO_EXTENSION_Init();
  delay(10);
  IO_EXTENSION_Output(IO_EXTENSION_IO_4, 1);

  params_init();

  // 2. SD Card AVANT le LCD
#ifdef DEBUG_MODE
  Serial.println("Initialisation SD...");
#endif
  if (!sd_init()) {
#ifdef DEBUG_MODE
    Serial.println("SD Failed");
#endif
  }

#ifdef DEBUG_MODE
  Serial.println("Starting Vario...");
  Serial.printf("Project: %s\n", VARIO_NAME);
  Serial.printf("Version: %s\n", VARIO_VERSION);
#endif

  // 3. Initialiser le hardware d'affichage
  static esp_lcd_panel_handle_t panel_handle = NULL;
  static esp_lcd_touch_handle_t tp_handle = NULL;

  tp_handle = touch_gt911_init();
  panel_handle = waveshare_esp32_s3_rgb_lcd_init();
  wavesahre_rgb_lcd_set_brightness(params.system_brightness);

  // 4. Init LVGL
  esp_err_t ret = lvgl_port_init(panel_handle, tp_handle);
  if (ret != ESP_OK) {
    while (1) vTaskDelay(pdMS_TO_TICKS(1000));
  }

  // 5. UI
  if (lvgl_port_lock(-1)) {
    ui_splash_show();
    lvgl_port_unlock();
  }
}

void loop() {
#ifdef DEBUG_MODE
  static unsigned long last_print = 0;

  if (millis() - last_print > 5000) {
    Serial.println("=== Memory Status ===");

    // SRAM
    size_t free_heap = ESP.getFreeHeap();
    size_t total_heap = ESP.getHeapSize();
    size_t min_free = ESP.getMinFreeHeap();
    size_t largest = ESP.getMaxAllocHeap();

    Serial.printf("SRAM:  Used: %6u / %6u (%.1f%%) | Free: %6u | Min: %6u | Largest: %6u\n",
                  total_heap - free_heap, total_heap,
                  ((total_heap - free_heap) * 100.0) / total_heap,
                  free_heap, min_free, largest);

    // PSRAM
    size_t free_psram = ESP.getFreePsram();
    size_t total_psram = ESP.getPsramSize();

    Serial.printf("PSRAM: Used: %6u / %6u (%.1f%%) | Free: %6u\n",
                  total_psram - free_psram, total_psram,
                  ((total_psram - free_psram) * 100.0) / total_psram,
                  free_psram);

    // ALERTE si critique
    if (free_heap < 10000) {
      Serial.println("⚠️  WARNING: SRAM critically low!");
    }
    if (largest < 5000) {
      Serial.println("⚠️  WARNING: Severe heap fragmentation!");
    }

    Serial.println("====================");

    print_all_tasks_stack_usage();

    last_print = millis();
  }
#endif

  vTaskDelay(pdMS_TO_TICKS(200));
}