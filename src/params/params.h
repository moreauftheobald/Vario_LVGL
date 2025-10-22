#ifndef PARAMS_H
#define PARAMS_H

#include <Preferences.h>
#include "lang.h"
#include "esp_heap_caps.h"

// Helper pour allouer une string en PSRAM
static inline char* psram_strdup(const char* str) {
    if (!str) return nullptr;
    
    size_t len = strlen(str) + 1;
    char* ptr = (char*)heap_caps_malloc(len, MALLOC_CAP_SPIRAM);
    if (ptr) {
        strcpy(ptr, str);
    }
    return ptr;
}

// Helper pour liberer et reallouer
static inline void psram_str_set(char** dest, const char* src) {
    if (*dest) {
        heap_caps_free(*dest);
        *dest = nullptr;
    }
    if (src && src[0] != '\0') {
        *dest = psram_strdup(src);
    }
}

// Helper pour obtenir la string (retourne "" si NULL)
static inline const char* psram_str_get(const char* str) {
    return str ? str : "";
}

// Structure pour tous les parametres du vario
typedef struct {
    // Parametres pilote - alloues en PSRAM
    char* pilot_name;
    char* pilot_firstname;
    char* pilot_wing;
    char* pilot_phone;
    
    // Parametres WiFi (4 priorites) - alloues en PSRAM
    char* wifi_ssid[4];
    char* wifi_password[4];
    
    // Parametres vario
    int vario_integration_period;  // secondes
    uint16_t vario_audio_frequencies[16];  // Hz pour chaque vitesse vario
    
    // Parametres carte
    int map_zoom;
    int map_tile_server;
    int map_track_points;
    bool map_vario_colors;
    
    // Parametres systeme
    int system_brightness;  // 0-100%
    Language system_language;
    
    // Parametres calibration ecran
    float touch_offset_x;
    float touch_offset_y;
    float touch_scale_x;
    float touch_scale_y;
    
} VarioParams;

// Variable globale des parametres
static VarioParams params = {0};

// Valeurs par defaut en PROGMEM
static const uint16_t default_audio_frequencies[16] PROGMEM = {
  640, 664, 691, 727, 759, 789, 842, 920,
  998, 1060, 1097, 1121, 1144, 1161, 1170, 1175
};

// ============================================================================
// FONCTIONS
// ============================================================================

static inline void params_reset_to_defaults(void) {
    // Liberer les anciennes allocations
    psram_str_set(&params.pilot_name, "");
    psram_str_set(&params.pilot_firstname, "");
    psram_str_set(&params.pilot_wing, "");
    psram_str_set(&params.pilot_phone, "");
    
    for (int i = 0; i < 4; i++) {
        psram_str_set(&params.wifi_ssid[i], "");
        psram_str_set(&params.wifi_password[i], "");
    }
    
    // Vario - Copier depuis PROGMEM
    params.vario_integration_period = 5;
    for (int i = 0; i < 16; i++) {
        params.vario_audio_frequencies[i] = pgm_read_word(&default_audio_frequencies[i]);
    }
    
    // Carte
    params.map_zoom = 13;
    params.map_tile_server = 0;
    params.map_track_points = 200;
    params.map_vario_colors = true;
    
    // Systeme
    params.system_brightness = 80;
    params.system_language = LANG_FR;
    
    // Calibration ecran
    params.touch_offset_x = 0.0f;
    params.touch_offset_y = 0.0f;
    params.touch_scale_x = 1.0f;
    params.touch_scale_y = 1.0f;
}

// ============================================================================
// INIT
// ============================================================================

static inline void params_init(void) {
#ifdef DEBUG_MODE
    Serial.println("Initializing params...");
#endif

    // Initialiser les pointeurs a NULL
    params.pilot_name = nullptr;
    params.pilot_firstname = nullptr;
    params.pilot_wing = nullptr;
    params.pilot_phone = nullptr;
    for (int i = 0; i < 4; i++) {
        params.wifi_ssid[i] = nullptr;
        params.wifi_password[i] = nullptr;
    }

    prefs.begin("vario", false);
    
    // Pilote - Charger depuis preferences en PSRAM
    String temp = prefs.getString("pilot_name", "");
    psram_str_set(&params.pilot_name, temp.c_str());
    
    temp = prefs.getString("pilot_fname", "");
    psram_str_set(&params.pilot_firstname, temp.c_str());
    
    temp = prefs.getString("pilot_wing", "");
    psram_str_set(&params.pilot_wing, temp.c_str());
    
    temp = prefs.getString("pilot_phone", "");
    psram_str_set(&params.pilot_phone, temp.c_str());
    
    // WiFi
    for (int i = 0; i < 4; i++) {
        String ssid_key = "wifi_ssid" + String(i);
        String pass_key = "wifi_pass" + String(i);
        
        temp = prefs.getString(ssid_key.c_str(), "");
        psram_str_set(&params.wifi_ssid[i], temp.c_str());
        
        temp = prefs.getString(pass_key.c_str(), "");
        psram_str_set(&params.wifi_password[i], temp.c_str());
    }
    
    // Vario
    params.vario_integration_period = prefs.getInt("vario_int", 5);
    
    for (int i = 0; i < 16; i++) {
        String key = "vario_freq" + String(i);
        params.vario_audio_frequencies[i] = prefs.getUShort(key.c_str(), 
            pgm_read_word(&default_audio_frequencies[i]));
    }
    
    // Carte
    params.map_zoom = prefs.getInt("map_zoom", 13);
    params.map_tile_server = prefs.getInt("map_tile_srv", 0);
    params.map_track_points = prefs.getInt("map_track_pts", 200);
    params.map_vario_colors = prefs.getBool("map_vario_col", true);
    
    // Systeme
    params.system_brightness = prefs.getInt("sys_bright", 80);
    params.system_language = (Language)prefs.getInt("sys_lang", LANG_FR);
    
    // Calibration
    params.touch_offset_x = prefs.getFloat("touch_off_x", 0.0f);
    params.touch_offset_y = prefs.getFloat("touch_off_y", 0.0f);
    params.touch_scale_x = prefs.getFloat("touch_scl_x", 1.0f);
    params.touch_scale_y = prefs.getFloat("touch_scl_y", 1.0f);

#ifdef DEBUG_MODE
    Serial.println("Params initialized");
    Serial.printf("Free PSRAM after params init: %u\n", ESP.getFreePsram());
#endif
}

// ============================================================================
// SAVE
// ============================================================================

static inline void params_save_pilot(void) {
    prefs.putString("pilot_name", psram_str_get(params.pilot_name));
    prefs.putString("pilot_fname", psram_str_get(params.pilot_firstname));
    prefs.putString("pilot_wing", psram_str_get(params.pilot_wing));
    prefs.putString("pilot_phone", psram_str_get(params.pilot_phone));
    
#ifdef DEBUG_MODE
    Serial.println("Pilot params saved");
#endif
}

static inline void params_save_wifi(void) {
    for (int i = 0; i < 4; i++) {
        String ssid_key = "wifi_ssid" + String(i);
        String pass_key = "wifi_pass" + String(i);
        prefs.putString(ssid_key.c_str(), psram_str_get(params.wifi_ssid[i]));
        prefs.putString(pass_key.c_str(), psram_str_get(params.wifi_password[i]));
    }
    
#ifdef DEBUG_MODE
    Serial.println("WiFi params saved");
#endif
}

static inline void params_save_vario(void) {
    prefs.putInt("vario_int", params.vario_integration_period);
    
    for (int i = 0; i < 16; i++) {
        String key = "vario_freq" + String(i);
        prefs.putUShort(key.c_str(), params.vario_audio_frequencies[i]);
    }
    
#ifdef DEBUG_MODE
    Serial.println("Vario params saved");
#endif
}

static inline void params_save_map(void) {
    prefs.putInt("map_zoom", params.map_zoom);
    prefs.putInt("map_tile_srv", params.map_tile_server);
    prefs.putInt("map_track_pts", params.map_track_points);
    prefs.putBool("map_vario_col", params.map_vario_colors);
    
#ifdef DEBUG_MODE
    Serial.println("Map params saved");
#endif
}

static inline void params_save_system(void) {
    prefs.putInt("sys_bright", params.system_brightness);
    prefs.putInt("sys_lang", (int)params.system_language);
    
#ifdef DEBUG_MODE
    Serial.println("System params saved");
#endif
}

static inline void params_save_calibration(void) {
    prefs.putFloat("touch_off_x", params.touch_offset_x);
    prefs.putFloat("touch_off_y", params.touch_offset_y);
    prefs.putFloat("touch_scl_x", params.touch_scale_x);
    prefs.putFloat("touch_scl_y", params.touch_scale_y);
    
#ifdef DEBUG_MODE
    Serial.println("Calibration params saved");
#endif
}

#endif