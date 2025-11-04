#ifndef UI_FLIGHT_DISPLAY_H
#define UI_FLIGHT_DISPLAY_H

#include "lvgl.h"
#include "UI_helper.h"
#include "graphical.h"
#include "src/flight_data.h"

// Enumeration pour les modes d'affichage altitude
typedef enum {
  ALT_MODE_QNH = 0,      // Altitude QNH (calage reglementaire)
  ALT_MODE_BARO = 1,     // Altitude baro brute (1013.25 hPa)
  ALT_MODE_AGL = 2,      // Hauteur sol (Above Ground Level)
  ALT_MODE_GPS = 3       // Altitude GPS
} altitude_mode_t;

// Enumeration pour les modes d'affichage vario
typedef enum {
  VARIO_MODE_INT = 0,    // Vario integre
  VARIO_MODE_RAW = 1     // Vario instantane
} vario_mode_t;

// Variables globales pour les widgets altitude
static lv_obj_t *altitude_zone = NULL;
static lv_obj_t *tab_qnh = NULL;
static lv_obj_t *tab_baro = NULL;
static lv_obj_t *tab_agl = NULL;
static lv_obj_t *tab_gps = NULL;
static lv_obj_t *label_altitude_main = NULL;
static lv_obj_t *label_altitude_sub = NULL;

static altitude_mode_t current_altitude_mode = ALT_MODE_QNH;

// Variables globales pour les widgets vario
static lv_obj_t *vario_zone = NULL;
static lv_obj_t *tab_vario_int = NULL;
static lv_obj_t *tab_vario_raw = NULL;
static lv_obj_t *label_vario_main = NULL;
static lv_obj_t *vario_bar = NULL;
static lv_obj_t *vario_bar_fill = NULL;

static vario_mode_t current_vario_mode = VARIO_MODE_INT;

// Callback pour changement d'onglet altitude
static void altitude_tab_event_cb(lv_event_t *e) {
  lv_obj_t *tab = (lv_obj_t *)lv_event_get_target(e);
  
  // Determiner quel onglet a ete clique
  if (tab == tab_qnh) {
    current_altitude_mode = ALT_MODE_QNH;
  } else if (tab == tab_baro) {
    current_altitude_mode = ALT_MODE_BARO;
  } else if (tab == tab_agl) {
    current_altitude_mode = ALT_MODE_AGL;
  } else if (tab == tab_gps) {
    current_altitude_mode = ALT_MODE_GPS;
  }
  
  // Mettre a jour les styles des onglets
  lv_obj_set_style_bg_color(tab_qnh, 
    (current_altitude_mode == ALT_MODE_QNH) ? lv_color_hex(UI_COLOR_BTN_VARIO) : lv_color_hex(0x333333), 0);
  lv_obj_set_style_bg_color(tab_baro, 
    (current_altitude_mode == ALT_MODE_BARO) ? lv_color_hex(UI_COLOR_BTN_VARIO) : lv_color_hex(0x333333), 0);
  lv_obj_set_style_bg_color(tab_agl, 
    (current_altitude_mode == ALT_MODE_AGL) ? lv_color_hex(UI_COLOR_BTN_VARIO) : lv_color_hex(0x333333), 0);
  lv_obj_set_style_bg_color(tab_gps, 
    (current_altitude_mode == ALT_MODE_GPS) ? lv_color_hex(UI_COLOR_BTN_VARIO) : lv_color_hex(0x333333), 0);
  
  // Mettre a jour les couleurs de texte
  lv_obj_t *label_qnh = lv_obj_get_child(tab_qnh, 0);
  lv_obj_t *label_baro = lv_obj_get_child(tab_baro, 0);
  lv_obj_t *label_agl = lv_obj_get_child(tab_agl, 0);
  lv_obj_t *label_gps = lv_obj_get_child(tab_gps, 0);
  
  lv_obj_set_style_text_color(label_qnh, 
    (current_altitude_mode == ALT_MODE_QNH) ? lv_color_white() : lv_color_hex(0xAAAAAA), 0);
  lv_obj_set_style_text_color(label_baro, 
    (current_altitude_mode == ALT_MODE_BARO) ? lv_color_white() : lv_color_hex(0xAAAAAA), 0);
  lv_obj_set_style_text_color(label_agl, 
    (current_altitude_mode == ALT_MODE_AGL) ? lv_color_white() : lv_color_hex(0xAAAAAA), 0);
  lv_obj_set_style_text_color(label_gps, 
    (current_altitude_mode == ALT_MODE_GPS) ? lv_color_white() : lv_color_hex(0xAAAAAA), 0);

#ifdef DEBUG_MODE
  const char *mode_names[] = {"QNH", "BARO", "AGL", "GPS"};
  Serial.printf("[FLIGHT_DISPLAY] Altitude mode changed to: %s\n", mode_names[current_altitude_mode]);
#endif
}

// Creation d'un onglet
static lv_obj_t *create_altitude_tab(lv_obj_t *parent, const char *text, bool active) {
  lv_obj_t *tab = lv_obj_create(parent);
  lv_obj_set_size(tab, 105, 30);
  lv_obj_set_style_bg_color(tab, active ? lv_color_hex(UI_COLOR_BTN_VARIO) : lv_color_hex(0x333333), 0);
  lv_obj_set_style_border_width(tab, 0, 0);
  lv_obj_set_style_radius(tab, 8, 0);
  lv_obj_set_style_pad_all(tab, 0, 0);
  lv_obj_add_flag(tab, LV_OBJ_FLAG_CLICKABLE);
  lv_obj_clear_flag(tab, LV_OBJ_FLAG_SCROLLABLE);
  
  lv_obj_t *label = lv_label_create(tab);
  lv_label_set_text(label, text);
  lv_obj_set_style_text_font(label, UI_FONT_NORMAL, 0);
  lv_obj_set_style_text_color(label, active ? lv_color_white() : lv_color_hex(0xAAAAAA), 0);
  lv_obj_center(label);
  
  lv_obj_add_event_cb(tab, altitude_tab_event_cb, LV_EVENT_CLICKED, NULL);
  
  return tab;
}

// Callback pour changement d'onglet vario
static void vario_tab_event_cb(lv_event_t *e) {
  lv_obj_t *tab = (lv_obj_t *)lv_event_get_target(e);
  
  if (tab == tab_vario_int) {
    current_vario_mode = VARIO_MODE_INT;
  } else if (tab == tab_vario_raw) {
    current_vario_mode = VARIO_MODE_RAW;
  }
  
  // Mettre a jour les styles
  lv_obj_set_style_bg_color(tab_vario_int, 
    (current_vario_mode == VARIO_MODE_INT) ? lv_color_hex(UI_COLOR_BTN_VARIO) : lv_color_hex(0x333333), 0);
  lv_obj_set_style_bg_color(tab_vario_raw, 
    (current_vario_mode == VARIO_MODE_RAW) ? lv_color_hex(UI_COLOR_BTN_VARIO) : lv_color_hex(0x333333), 0);
  
  lv_obj_t *label_int = lv_obj_get_child(tab_vario_int, 0);
  lv_obj_t *label_raw = lv_obj_get_child(tab_vario_raw, 0);
  
  lv_obj_set_style_text_color(label_int, 
    (current_vario_mode == VARIO_MODE_INT) ? lv_color_white() : lv_color_hex(0xAAAAAA), 0);
  lv_obj_set_style_text_color(label_raw, 
    (current_vario_mode == VARIO_MODE_RAW) ? lv_color_white() : lv_color_hex(0xAAAAAA), 0);

#ifdef DEBUG_MODE
  const char *mode_names[] = {"INT", "RAW"};
  Serial.printf("[FLIGHT_DISPLAY] Vario mode changed to: %s\n", mode_names[current_vario_mode]);
#endif
}

// Creation d'un onglet vario
static lv_obj_t *create_vario_tab(lv_obj_t *parent, const char *text, bool active) {
  lv_obj_t *tab = lv_obj_create(parent);
  lv_obj_set_size(tab, 215, 30);
  lv_obj_set_style_bg_color(tab, active ? lv_color_hex(UI_COLOR_BTN_VARIO) : lv_color_hex(0x333333), 0);
  lv_obj_set_style_border_width(tab, 0, 0);
  lv_obj_set_style_radius(tab, 8, 0);
  lv_obj_set_style_pad_all(tab, 0, 0);
  lv_obj_add_flag(tab, LV_OBJ_FLAG_CLICKABLE);
  lv_obj_clear_flag(tab, LV_OBJ_FLAG_SCROLLABLE);
  
  lv_obj_t *label = lv_label_create(tab);
  lv_label_set_text(label, text);
  lv_obj_set_style_text_font(label, UI_FONT_NORMAL, 0);
  lv_obj_set_style_text_color(label, active ? lv_color_white() : lv_color_hex(0xAAAAAA), 0);
  lv_obj_center(label);
  
  lv_obj_add_event_cb(tab, vario_tab_event_cb, LV_EVENT_CLICKED, NULL);
  
  return tab;
}

// Fonction pour obtenir couleur vario selon valeur
static lv_color_t get_vario_color(float vario) {
  if (vario <= -10.0f) return lv_color_hex(0x00004D);      // Bleu tres fonce
  if (vario <= -5.0f) return lv_color_hex(0x0000AA);       // Bleu fonce
  if (vario <= 0.0f) return lv_color_hex(0x0055FF);        // Bleu
  if (vario <= 2.0f) return lv_color_hex(0x00DD00);        // Vert
  if (vario <= 5.0f) return lv_color_hex(0xFF8800);        // Orange
  return lv_color_hex(0xDD0000);                            // Rouge fonce
}

// Creation de la zone vario complete
lv_obj_t *ui_create_vario_zone(lv_obj_t *parent) {
  // Conteneur principal (189px de hauteur)
  vario_zone = lv_obj_create(parent);
  lv_obj_set_size(vario_zone, lv_pct(100), 189);
  lv_obj_set_style_bg_color(vario_zone, lv_color_hex(UI_COLOR_SURFACE), 0);
  lv_obj_set_style_border_width(vario_zone, 2, 0);
  lv_obj_set_style_border_color(vario_zone, lv_color_hex(UI_COLOR_BORDER_PRIMARY), 0);
  lv_obj_set_style_radius(vario_zone, UI_RADIUS_SMALL, 0);
  lv_obj_set_style_pad_all(vario_zone, 8, 0);
  lv_obj_clear_flag(vario_zone, LV_OBJ_FLAG_SCROLLABLE);
  
  // Conteneur des onglets (30px)
  lv_obj_t *tabs_container = lv_obj_create(vario_zone);
  lv_obj_set_size(tabs_container, lv_pct(100), 30);
  lv_obj_align(tabs_container, LV_ALIGN_TOP_MID, 0, 0);
  lv_obj_set_style_bg_opa(tabs_container, LV_OPA_TRANSP, 0);
  lv_obj_set_style_border_width(tabs_container, 0, 0);
  lv_obj_set_style_pad_all(tabs_container, 0, 0);
  lv_obj_set_flex_flow(tabs_container, LV_FLEX_FLOW_ROW);
  lv_obj_set_flex_align(tabs_container, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
  lv_obj_clear_flag(tabs_container, LV_OBJ_FLAG_SCROLLABLE);
  
  // Creation des 2 onglets
  tab_vario_int = create_vario_tab(tabs_container, "INT", true);
  tab_vario_raw = create_vario_tab(tabs_container, "RAW", false);
  
  // Label vario principal (70px)
  label_vario_main = lv_label_create(vario_zone);
  lv_label_set_text(label_vario_main, "---");
  lv_obj_set_style_text_font(label_vario_main, &lv_font_montserrat_48, 0);
  lv_obj_set_style_text_color(label_vario_main, lv_color_hex(UI_COLOR_TEXT_PRIMARY), 0);
  lv_obj_align(label_vario_main, LV_ALIGN_TOP_MID, 0, 40);
  
  // Barre de fond (40px)
  vario_bar = lv_obj_create(vario_zone);
  lv_obj_set_size(vario_bar, lv_pct(95), 40);
  lv_obj_align(vario_bar, LV_ALIGN_BOTTOM_MID, 0, -15);
  lv_obj_set_style_bg_color(vario_bar, lv_color_hex(0x222222), 0);
  lv_obj_set_style_border_width(vario_bar, 1, 0);
  lv_obj_set_style_border_color(vario_bar, lv_color_hex(0x555555), 0);
  lv_obj_set_style_radius(vario_bar, 5, 0);
  lv_obj_set_style_pad_all(vario_bar, 2, 0);
  lv_obj_clear_flag(vario_bar, LV_OBJ_FLAG_SCROLLABLE);
  
  // Barre de remplissage
  vario_bar_fill = lv_obj_create(vario_bar);
  lv_obj_set_size(vario_bar_fill, 0, lv_pct(100));
  lv_obj_align(vario_bar_fill, LV_ALIGN_LEFT_MID, 0, 0);
  lv_obj_set_style_bg_color(vario_bar_fill, lv_color_hex(0x00DD00), 0);
  lv_obj_set_style_border_width(vario_bar_fill, 0, 0);
  lv_obj_set_style_radius(vario_bar_fill, 3, 0);
  lv_obj_clear_flag(vario_bar_fill, LV_OBJ_FLAG_SCROLLABLE);
  
  // Graduations (15px)
  lv_obj_t *label_grad = lv_label_create(vario_zone);
  lv_label_set_text(label_grad, "-10    -5     0     +5    +10");
  lv_obj_set_style_text_font(label_grad, UI_FONT_SMALL, 0);
  lv_obj_set_style_text_color(label_grad, lv_color_hex(UI_COLOR_TEXT_SECONDARY), 0);
  lv_obj_align(label_grad, LV_ALIGN_BOTTOM_MID, 0, -2);
  
  return vario_zone;
}

// Mise a jour de l'affichage vario
void ui_update_vario_display(void) {
  extern flight_data_t g_flight_data;
  extern SemaphoreHandle_t flight_data_mutex;
  
  if (!flight_data_mutex || !label_vario_main || !vario_bar_fill) {
    return;
  }
  
  flight_data_t data;
  if (xSemaphoreTake(flight_data_mutex, pdMS_TO_TICKS(5))) {
    data = g_flight_data;
    xSemaphoreGive(flight_data_mutex);
  } else {
    return;
  }
  
  // Selectionner la valeur selon le mode
  float vario_value = (current_vario_mode == VARIO_MODE_INT) ? 
                      data.vario_integrated : data.vario_raw;
  
  // Mettre a jour le label
  static char vario_text[32];
  if (data.valid) {
    snprintf(vario_text, sizeof(vario_text), "%+.1f m/s", vario_value);
    lv_label_set_text(label_vario_main, vario_text);
    
    // Couleur du texte selon valeur
    lv_obj_set_style_text_color(label_vario_main, get_vario_color(vario_value), 0);
  } else {
    lv_label_set_text(label_vario_main, "--- m/s");
    lv_obj_set_style_text_color(label_vario_main, lv_color_hex(UI_COLOR_TEXT_PRIMARY), 0);
  }
  
  // Mettre a jour la barre (toujours avec vario integre pour stabilite visuelle)
  if (data.valid) {
    float vario_bar_value = data.vario_integrated;
    
    // Limiter entre -10 et +10
    if (vario_bar_value < -10.0f) vario_bar_value = -10.0f;
    if (vario_bar_value > 10.0f) vario_bar_value = 10.0f;
    
    // Convertir en pourcentage (0 = centre)
    int bar_width = lv_obj_get_width(vario_bar) - 4;
    int center = bar_width / 2;
    int offset = (int)(vario_bar_value * center / 10.0f);
    
    if (vario_bar_value >= 0) {
      // Montee : depuis centre vers droite
      lv_obj_set_width(vario_bar_fill, offset);
      lv_obj_align(vario_bar_fill, LV_ALIGN_LEFT_MID, center, 0);
    } else {
      // Descente : depuis centre vers gauche
      lv_obj_set_width(vario_bar_fill, -offset);
      lv_obj_align(vario_bar_fill, LV_ALIGN_LEFT_MID, center + offset, 0);
    }
    
    // Couleur de la barre
    lv_obj_set_style_bg_color(vario_bar_fill, get_vario_color(vario_bar_value), 0);
  } else {
    lv_obj_set_width(vario_bar_fill, 0);
  }
}

// Creation de la zone altitude complete
lv_obj_t *ui_create_altitude_zone(lv_obj_t *parent) {
  // Conteneur principal (216px de hauteur)
  altitude_zone = lv_obj_create(parent);
  lv_obj_set_size(altitude_zone, lv_pct(100), 216);
  lv_obj_set_style_bg_color(altitude_zone, lv_color_hex(UI_COLOR_SURFACE), 0);
  lv_obj_set_style_border_width(altitude_zone, 2, 0);
  lv_obj_set_style_border_color(altitude_zone, lv_color_hex(UI_COLOR_BORDER_PRIMARY), 0);
  lv_obj_set_style_radius(altitude_zone, UI_RADIUS_SMALL, 0);
  lv_obj_set_style_pad_all(altitude_zone, 8, 0);
  lv_obj_clear_flag(altitude_zone, LV_OBJ_FLAG_SCROLLABLE);
  
  // Conteneur des onglets (30px)
  lv_obj_t *tabs_container = lv_obj_create(altitude_zone);
  lv_obj_set_size(tabs_container, lv_pct(100), 30);
  lv_obj_align(tabs_container, LV_ALIGN_TOP_MID, 0, 0);
  lv_obj_set_style_bg_opa(tabs_container, LV_OPA_TRANSP, 0);
  lv_obj_set_style_border_width(tabs_container, 0, 0);
  lv_obj_set_style_pad_all(tabs_container, 0, 0);
  lv_obj_set_flex_flow(tabs_container, LV_FLEX_FLOW_ROW);
  lv_obj_set_flex_align(tabs_container, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
  lv_obj_clear_flag(tabs_container, LV_OBJ_FLAG_SCROLLABLE);
  
  // Creation des 4 onglets
  tab_qnh = create_altitude_tab(tabs_container, "QNH", true);
  tab_baro = create_altitude_tab(tabs_container, "BARO", false);
  tab_agl = create_altitude_tab(tabs_container, "AGL", false);
  tab_gps = create_altitude_tab(tabs_container, "GPS", false);
  
  // Label altitude principale (100px)
  label_altitude_main = lv_label_create(altitude_zone);
  lv_label_set_text(label_altitude_main, "---");
  lv_obj_set_style_text_font(label_altitude_main, &lv_font_montserrat_48, 0);
  lv_obj_set_style_text_color(label_altitude_main, lv_color_hex(UI_COLOR_TEXT_PRIMARY), 0);
  lv_obj_align(label_altitude_main, LV_ALIGN_CENTER, 0, -15);
  
  // Label sous-valeurs (20px)
  label_altitude_sub = lv_label_create(altitude_zone);
  lv_label_set_text(label_altitude_sub, "BARO:--- AGL:--- GPS:---");
  lv_obj_set_style_text_font(label_altitude_sub, UI_FONT_SMALL, 0);
  lv_obj_set_style_text_color(label_altitude_sub, lv_color_hex(UI_COLOR_TEXT_SECONDARY), 0);
  lv_obj_align(label_altitude_sub, LV_ALIGN_BOTTOM_MID, 0, -5);
  
  return altitude_zone;
}

// Mise a jour de l'affichage altitude
void ui_update_altitude_display(void) {
  extern flight_data_t g_flight_data;
  extern SemaphoreHandle_t flight_data_mutex;
  
  // Verifications critiques
  if (!flight_data_mutex || !label_altitude_main || !label_altitude_sub) {
#ifdef DEBUG_MODE
    static bool error_logged = false;
    if (!error_logged) {
      Serial.println("[UI] ERROR: Labels not initialized!");
      error_logged = true;
    }
#endif
    return;
  }
  
  flight_data_t data;
  if (xSemaphoreTake(flight_data_mutex, pdMS_TO_TICKS(5))) {
    data = g_flight_data;
    xSemaphoreGive(flight_data_mutex);
  } else {
    return;
  }
  
  // Mettre a jour la valeur principale selon le mode
  float main_value = 0.0f;
  const char *unit = "m";
  
  switch (current_altitude_mode) {
    case ALT_MODE_QNH:
      main_value = data.altitude_qnh;
      break;
    case ALT_MODE_BARO:
      main_value = data.altitude_qne;  // Baro brut = QNE (1013.25)
      break;
    case ALT_MODE_AGL:
      main_value = data.altitude_agl;
      break;
    case ALT_MODE_GPS:
      main_value = data.altitude_gps;
      break;
    default:
      main_value = data.altitude_qnh;
      break;
  }
  
  // Buffer statique pour eviter problemes memoire
  static char alt_text[32];
  static char sub_text[64];
  
  if (data.valid) {
    snprintf(alt_text, sizeof(alt_text), "%.0f %s", main_value, unit);
    lv_label_set_text(label_altitude_main, alt_text);
  } else {
    lv_label_set_text(label_altitude_main, "--- m");
  }
  
  // Mettre a jour les sous-valeurs
  if (data.valid) {
    snprintf(sub_text, sizeof(sub_text), 
             "BARO:%.0f  AGL:%.0f  GPS:%.0f",
             data.altitude_qne,
             data.altitude_agl,
             data.altitude_gps);
    lv_label_set_text(label_altitude_sub, sub_text);
  } else {
    lv_label_set_text(label_altitude_sub, "BARO:---  AGL:---  GPS:---");
  }
}

#endif // UI_FLIGHT_DISPLAY_H