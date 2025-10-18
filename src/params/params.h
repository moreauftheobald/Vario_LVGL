#ifndef PARAMS_H
#define PARAMS_H

#include <Preferences.h>
#include "lang.h"

extern Preferences prefs;

// Structure pour tous les parametres du vario
typedef struct {
    // Parametres pilote
    String pilot_name;
    String pilot_firstname;
    String pilot_wing;
    String pilot_phone;
    
    // Parametres WiFi (4 priorites)
    String wifi_ssid[4];
    String wifi_password[4];
    
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
static VarioParams params;

// Valeurs par defaut
static const uint16_t default_audio_frequencies[16] = {
  640, 664, 691, 727, 759, 789, 842, 920,
  998, 1060, 1097, 1121, 1144, 1161, 1170, 1175
};

// ============================================================================
// FONCTIONS
// ============================================================================

static inline void params_reset_to_defaults(void) {
    // Pilote
    params.pilot_name = "";
    params.pilot_firstname = "";
    params.pilot_wing = "";
    params.pilot_phone = "";
    
    // WiFi
    for (int i = 0; i < 4; i++) {
        params.wifi_ssid[i] = "";
        params.wifi_password[i] = "";
    }
    
    // Vario
    params.vario_integration_period = 5;
    for (int i = 0; i < 16; i++) {
        params.vario_audio_frequencies[i] = default_audio_frequencies[i];
    }
    
    // Carte
    params.map_zoom = 15;
    params.map_tile_server = 0;
    params.map_track_points = 50;
    params.map_vario_colors = true;
    
    // Systeme
    params.system_brightness = 80;
    params.system_language = LANG_FR;
    
    // Calibration
    params.touch_offset_x = 0.0f;
    params.touch_offset_y = 0.0f;
    params.touch_scale_x = 1.0f;
    params.touch_scale_y = 1.0f;
    
#ifdef DEBUG_MODE
    Serial.println("Parameters reset to defaults");
#endif
}

static inline void params_load_all(void) {
    // === PILOTE ===
    prefs.begin("pilot", true);
    params.pilot_name = prefs.getString("name", "");
    params.pilot_firstname = prefs.getString("firstname", "");
    params.pilot_wing = prefs.getString("wing", "");
    params.pilot_phone = prefs.getString("phone", "");
    prefs.end();
    
    // === WIFI ===
    prefs.begin("wifi", true);
    for (int i = 0; i < 4; i++) {
        String ssid_key = "ssid" + String(i);
        String pass_key = "pass" + String(i);
        params.wifi_ssid[i] = prefs.getString(ssid_key.c_str(), "");
        params.wifi_password[i] = prefs.getString(pass_key.c_str(), "");
    }
    prefs.end();
    
    // === VARIO ===
    prefs.begin("vario", true);
    params.vario_integration_period = prefs.getInt("integration", 5);
    
    // Charger les frequences audio
    for (int i = 0; i < 16; i++) {
        String key = "freq" + String(i);
        params.vario_audio_frequencies[i] = prefs.getUShort(key.c_str(), default_audio_frequencies[i]);
    }
    prefs.end();
    
    // === CARTE ===
    prefs.begin("map", true);
    params.map_zoom = prefs.getInt("zoom", 15);
    params.map_tile_server = prefs.getInt("server", 0);
    params.map_track_points = prefs.getInt("track_pts", 50);
    params.map_vario_colors = prefs.getBool("vario_col", true);
    prefs.end();
    
    // === SYSTEME ===
    prefs.begin("system", true);
    params.system_brightness = prefs.getInt("brightness", 80);
    params.system_language = (Language)prefs.getInt("language", LANG_FR);
    prefs.end();
    
    // Appliquer la langue
    current_language = params.system_language;
    
    // === CALIBRATION TACTILE ===
    prefs.begin("touch_calib", true);
    params.touch_offset_x = prefs.getFloat("offset_x", 0.0f);
    params.touch_offset_y = prefs.getFloat("offset_y", 0.0f);
    params.touch_scale_x = prefs.getFloat("scale_x", 1.0f);
    params.touch_scale_y = prefs.getFloat("scale_y", 1.0f);
    prefs.end();
    
#ifdef DEBUG_MODE
    Serial.println("All parameters loaded from ROM");
#endif
}

static inline void params_save_pilot(void) {
    prefs.begin("pilot", false);
    prefs.putString("name", params.pilot_name);
    prefs.putString("firstname", params.pilot_firstname);
    prefs.putString("wing", params.pilot_wing);
    prefs.putString("phone", params.pilot_phone);
    prefs.end();
    
#ifdef DEBUG_MODE
    Serial.println("Pilot parameters saved");
#endif
}

static inline void params_save_wifi(void) {
    prefs.begin("wifi", false);
    for (int i = 0; i < 4; i++) {
        String ssid_key = "ssid" + String(i);
        String pass_key = "pass" + String(i);
        prefs.putString(ssid_key.c_str(), params.wifi_ssid[i]);
        prefs.putString(pass_key.c_str(), params.wifi_password[i]);
    }
    prefs.end();
    
#ifdef DEBUG_MODE
    Serial.println("WiFi parameters saved");
#endif
}

static inline void params_save_vario(void) {
    prefs.begin("vario", false);
    prefs.putInt("integration", params.vario_integration_period);
    
    for (int i = 0; i < 16; i++) {
        String key = "freq" + String(i);
        prefs.putUShort(key.c_str(), params.vario_audio_frequencies[i]);
    }
    prefs.end();
    
#ifdef DEBUG_MODE
    Serial.println("Vario parameters saved");
#endif
}

static inline void params_save_map(void) {
    prefs.begin("map", false);
    prefs.putInt("zoom", params.map_zoom);
    prefs.putInt("server", params.map_tile_server);
    prefs.putInt("track_pts", params.map_track_points);
    prefs.putBool("vario_col", params.map_vario_colors);
    prefs.end();
    
#ifdef DEBUG_MODE
    Serial.println("Map parameters saved");
#endif
}

static inline void params_save_system(void) {
    prefs.begin("system", false);
    prefs.putInt("brightness", params.system_brightness);
    prefs.putInt("language", (int)params.system_language);
    prefs.end();
    
    // Appliquer la langue
    current_language = params.system_language;
    
#ifdef DEBUG_MODE
    Serial.println("System parameters saved");
#endif
}

static inline void params_save_touch_calibration(void) {
    prefs.begin("touch_calib", false);
    prefs.putFloat("offset_x", params.touch_offset_x);
    prefs.putFloat("offset_y", params.touch_offset_y);
    prefs.putFloat("scale_x", params.touch_scale_x);
    prefs.putFloat("scale_y", params.touch_scale_y);
    prefs.end();
    
#ifdef DEBUG_MODE
    Serial.println("Touch calibration saved");
#endif
}

static inline void params_init(void) {
    // Initialiser avec valeurs par defaut
    params_reset_to_defaults();
    // Charger depuis ROM
    params_load_all();
}

#endif