#include "constants.h"
#include "globals.h"
#include "src/params/params.h"
#include "src/sd_card.h"
#include "src/sensors_i2c_task.h"
#include "src/lvgl_port/lvgl_port.h"
#include "src/rgb_lcd_port/rgb_lcd_port.h"
#include "src/wifi_task.h"
#include "src/metar_task.h"
#include "src/ui/UI_helper.h"
#include "src/ui/ui_settings_pilot.h"
#include "src/ui/ui_settings_wifi.h"
#include "src/ui/ui_settings_screen.h"
#include "src/ui/ui_settings_vario.h"
#include "src/ui/ui_settings_map.h"
#include "src/ui/ui_settings_system.h"
#include "src/ui/ui_settings.h"
#include "src/ui/ui_file_transfer.h"
#include "src/ui/ui_main_screens.h"
#include "src/ui/ui_prestart.h"
#include "src/ui/ui_splash.h"
#include "src/test_logger_task.h"
#include "src/kalman_task.h"

bool mainscreen_active = false;

void setup() {
  Serial.begin(115200);
#ifdef DEBUG_MODE
  Serial.setDebugOutput(true);
#endif

  // 1. Bus I2C + IO Extender
  DEV_I2C_Init();
  IO_EXTENSION_Init();
  delay(10);
  IO_EXTENSION_Output(IO_EXTENSION_IO_4, 1);

  // 2. Params + SD Card
  params_init();

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

  // 3. Capteurs I2C (BMP390, BNO080, GPS)
  sensors_i2c_start();

  // 4. Kalman
  kalman_start();

  metar_start();

  // 5. Ecran + Touch
  esp_lcd_touch_handle_t tp_handle = touch_gt911_init();
  esp_lcd_panel_handle_t panel_handle = waveshare_esp32_s3_rgb_lcd_init();
  wavesahre_rgb_lcd_set_brightness(params.system_brightness);

  // 6. Init LVGL
  esp_err_t ret = lvgl_port_init(panel_handle, tp_handle);
  if (ret != ESP_OK) {
#ifdef DEBUG_MODE
    Serial.println("LVGL init failed!");
#endif
    while (1) vTaskDelay(pdMS_TO_TICKS(1000));
  }

#ifdef TEST_MODE
  test_logger_start();
#ifdef DEBUG_MODE
  Serial.println("Test logger task started");
#endif
#endif

  // 7. Afficher splash (3s) -> puis ui_prestart_show()
  ui_splash_show();

#ifdef DEBUG_MODE
  Serial.println("Setup complete");
#endif
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
      Serial.println("WARNING: SRAM critically low!");
    }
    if (largest < 5000) {
      Serial.println("WARNING: Severe heap fragmentation!");
    }
    Serial.println("=== After UI Init ===");
    Serial.printf("SRAM Free: %u\n", ESP.getFreeHeap());
    Serial.printf("PSRAM Free: %u\n", ESP.getFreePsram());

    last_print = millis();
  }
#endif

  vTaskDelay(pdMS_TO_TICKS(1000));
}