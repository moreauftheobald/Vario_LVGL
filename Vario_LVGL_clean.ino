// Dans votre fichier .ino principal

#include "constants.h"
#include "globals.h"
#include "src/params/params.h"
#include "src/sd_card.h"
#include "src/sensors_i2c_task.h"
#include "src/lvgl_port/lvgl_port.h"
#include "src/rgb_lcd_port/rgb_lcd_port.h"
#include "src/ui/UIIncludes.h"

// Instances globales
UIScreenSplash* splash_screen = nullptr;
UIScreenPrestart* prestart_screen = nullptr;
UIScreenFileTransfer* file_transfer_screen = nullptr;
UIScreenSettings* settings_screen = nullptr;

// Callback de transition splash -> prestart
void onSplashComplete() {
#ifdef DEBUG_MODE
    Serial.println("[MAIN] Splash complete, showing prestart");
#endif
    
    if (lvgl_port_lock(-1)) {
        // Liberer splash
        if (splash_screen) {
            delete splash_screen;
            splash_screen = nullptr;
        }
        
        // Creer et afficher prestart
        prestart_screen = new UIScreenPrestart();
        prestart_screen->create();
        prestart_screen->load();
        
        lvgl_port_unlock();
        
#ifdef DEBUG_MODE
        Serial.println("[MAIN] Prestart screen shown");
#endif
    }
}

void setup() {
    Serial.begin(115200);
#ifdef DEBUG_MODE
    Serial.setDebugOutput(true);
#endif

    // 1. I2C et IO_EXTENSION en PREMIER
    DEV_I2C_Init();
    sensors_i2c_start();
    
    IO_EXTENSION_Init();
    delay(10);
    IO_EXTENSION_Output(IO_EXTENSION_IO_4, 1);
    
    params_init();
    
    // 2. SD Card
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
    
    // 3. Hardware affichage
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
    
    // 5. Creer et afficher splash screen
    if (lvgl_port_lock(-1)) {
        splash_screen = new UIScreenSplash();
        splash_screen->setOnCompleteCallback(onSplashComplete);
        splash_screen->create();
        splash_screen->load();
        lvgl_port_unlock();
        
#ifdef DEBUG_MODE
        Serial.println("[MAIN] Splash screen initialized");
#endif
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
        
        last_print = millis();
    }
#endif
    
    vTaskDelay(pdMS_TO_TICKS(1000));
}