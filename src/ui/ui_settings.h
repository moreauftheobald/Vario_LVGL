#ifndef UI_SETTINGS_H
#define UI_SETTINGS_H

#include "lvgl.h"
#include "constants.h"
#include "ui_settings_pilot.h"
#include "ui_settings_wifi.h"

// Forward declarations
void ui_prestart_show(void);
void ui_settings_pilot_show(void);
void ui_settings_wifi_show(void);
void ui_settings_screen_show(void);
void ui_settings_vario_show(void);
void ui_settings_map_show(void);
void ui_settings_system_show(void);

static lv_obj_t *screen_settings = NULL;

// Callback pour bouton retour
static void btn_back_cb(lv_event_t *e) {
#ifdef DEBUG_MODE
  Serial.println("Back button clicked");
#endif
  ui_prestart_show();
}

// Callbacks pour sous-menus
static void btn_pilot_cb(lv_event_t *e) {
#ifdef DEBUG_MODE
  Serial.println("Pilot settings clicked");
#endif
  ui_settings_pilot_show();  // Décommenter cette ligne
}

static void btn_wifi_cb(lv_event_t *e) {
#ifdef DEBUG_MODE
  Serial.println("WiFi settings clicked");
#endif
  ui_settings_wifi_show();  // Décommenter cette ligne
}

static void btn_screen_cb(lv_event_t *e) {
#ifdef DEBUG_MODE
  Serial.println("Screen calibration clicked");
#endif
  ui_settings_screen_show();  // <- Decommentez cette ligne
}

static void btn_vario_cb(lv_event_t *e) {
#ifdef DEBUG_MODE
  Serial.println("Vario settings clicked");
#endif
  // TODO: ui_settings_vario_show();
}

static void btn_map_cb(lv_event_t *e) {
#ifdef DEBUG_MODE
  Serial.println("Map settings clicked");
#endif
  // TODO: ui_settings_map_show();
}

static void btn_system_cb(lv_event_t *e) {
#ifdef DEBUG_MODE
  Serial.println("System settings clicked");
#endif
  // TODO: ui_settings_system_show();
}

/**
 * @brief Creer bouton de menu parametres
 */
static lv_obj_t *create_settings_button(lv_obj_t *parent, const char *text, const char *icon, lv_color_t color) {
  lv_obj_t *btn = lv_button_create(parent);
  lv_obj_set_size(btn, 440, 80);
  lv_obj_set_style_bg_color(btn, color, 0);
  lv_obj_set_style_radius(btn, 12, 0);
  lv_obj_set_style_shadow_width(btn, 5, 0);
  lv_obj_set_style_shadow_color(btn, lv_color_black(), 0);
  lv_obj_set_style_shadow_opa(btn, LV_OPA_20, 0);
  lv_obj_set_style_bg_color(btn, lv_color_darken(color, 20), LV_STATE_PRESSED);

  // Icon
  lv_obj_t *icon_label = lv_label_create(btn);
  lv_label_set_text(icon_label, icon);
  lv_obj_set_style_text_font(icon_label, &lv_font_montserrat_24, 0);
  lv_obj_set_style_text_color(icon_label, lv_color_white(), 0);
  lv_obj_align(icon_label, LV_ALIGN_LEFT_MID, 20, 0);

  // Text
  lv_obj_t *text_label = lv_label_create(btn);
  lv_label_set_text(text_label, text);
  lv_obj_set_style_text_font(text_label, &lv_font_montserrat_20, 0);
  lv_obj_set_style_text_color(text_label, lv_color_white(), 0);
  lv_obj_align(text_label, LV_ALIGN_LEFT_MID, 70, 0);

  return btn;
}

void ui_settings_init(void) {
  const TextStrings *txt = get_text();

  // Ecran avec gradient
  screen_settings = lv_obj_create(NULL);
  lv_obj_set_style_bg_color(screen_settings, lv_color_hex(0x0a0e27), 0);
  lv_obj_set_style_bg_grad_color(screen_settings, lv_color_hex(0x1a1f3a), 0);
  lv_obj_set_style_bg_grad_dir(screen_settings, LV_GRAD_DIR_VER, 0);

  // Container principal
  lv_obj_t *main_frame = lv_obj_create(screen_settings);
  lv_obj_set_size(main_frame, SCREEN_WIDTH - 10, SCREEN_HEIGHT - 10);
  lv_obj_center(main_frame);
  lv_obj_set_style_bg_color(main_frame, lv_color_hex(0x151932), 0);
  lv_obj_set_style_bg_opa(main_frame, LV_OPA_90, 0);
  lv_obj_set_style_border_width(main_frame, 3, 0);
  lv_obj_set_style_border_color(main_frame, lv_color_hex(0x6080a0), 0);
  lv_obj_set_style_radius(main_frame, 20, 0);
  lv_obj_set_style_pad_all(main_frame, 20, 0);
  lv_obj_set_style_shadow_width(main_frame, 30, 0);
  lv_obj_set_style_shadow_color(main_frame, lv_color_black(), 0);
  lv_obj_set_style_shadow_opa(main_frame, LV_OPA_40, 0);

  // Titre
  lv_obj_t *label_title = lv_label_create(main_frame);
  lv_label_set_text(label_title, txt->settings);
  lv_obj_set_style_text_font(label_title, &lv_font_montserrat_48, 0);
  lv_obj_set_style_text_color(label_title, lv_color_hex(0x00d4ff), 0);
  lv_obj_align(label_title, LV_ALIGN_TOP_MID, 0, 5);

  // Container pour les boutons (2 colonnes)
  lv_obj_t *buttons_container = lv_obj_create(main_frame);
  lv_obj_set_size(buttons_container, 940, 450);
  lv_obj_align(buttons_container, LV_ALIGN_CENTER, 0, 15);
  lv_obj_set_style_bg_opa(buttons_container, LV_OPA_TRANSP, 0);
  lv_obj_set_style_border_width(buttons_container, 0, 0);
  lv_obj_set_style_pad_all(buttons_container, 0, 0);
  lv_obj_set_flex_flow(buttons_container, LV_FLEX_FLOW_ROW_WRAP);
  lv_obj_set_flex_align(buttons_container, LV_FLEX_ALIGN_SPACE_EVENLY, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER);
  lv_obj_set_style_pad_column(buttons_container, 20, 0);
  lv_obj_set_style_pad_row(buttons_container, 15, 0);
  lv_obj_clear_flag(buttons_container, LV_OBJ_FLAG_SCROLLABLE);

  // Boutons de parametres
  lv_obj_t *btn_pilot = create_settings_button(buttons_container, txt->pilot_settings,
                                               LV_SYMBOL_HOME, lv_color_hex(0x5856d6));
  lv_obj_add_event_cb(btn_pilot, btn_pilot_cb, LV_EVENT_CLICKED, NULL);

  lv_obj_t *btn_wifi = create_settings_button(buttons_container, txt->wifi_settings,
                                              LV_SYMBOL_WIFI, lv_color_hex(0x007aff));
  lv_obj_add_event_cb(btn_wifi, btn_wifi_cb, LV_EVENT_CLICKED, NULL);

  lv_obj_t *btn_screen = create_settings_button(buttons_container, txt->screen_calibration,
                                                LV_SYMBOL_IMAGE, lv_color_hex(0xff9500));
  lv_obj_add_event_cb(btn_screen, btn_screen_cb, LV_EVENT_CLICKED, NULL);

  lv_obj_t *btn_vario = create_settings_button(buttons_container, txt->vario_settings,
                                               LV_SYMBOL_CHARGE, lv_color_hex(0x34c759));
  lv_obj_add_event_cb(btn_vario, btn_vario_cb, LV_EVENT_CLICKED, NULL);

  lv_obj_t *btn_map = create_settings_button(buttons_container, txt->map_settings,
                                             LV_SYMBOL_GPS, lv_color_hex(0x00c7be));
  lv_obj_add_event_cb(btn_map, btn_map_cb, LV_EVENT_CLICKED, NULL);

  lv_obj_t *btn_system = create_settings_button(buttons_container, txt->system_settings,
                                                LV_SYMBOL_SETTINGS, lv_color_hex(0x8e8e93));
  lv_obj_add_event_cb(btn_system, btn_system_cb, LV_EVENT_CLICKED, NULL);

  // Bouton retour
  lv_obj_t *btn_back = lv_button_create(main_frame);
  lv_obj_set_size(btn_back, 300, 70);
  lv_obj_align(btn_back, LV_ALIGN_BOTTOM_MID, 0, -15);
  lv_obj_set_style_bg_color(btn_back, lv_color_hex(0xff3b30), 0);
  lv_obj_set_style_radius(btn_back, 15, 0);
  lv_obj_set_style_shadow_width(btn_back, 5, 0);
  lv_obj_set_style_shadow_color(btn_back, lv_color_black(), 0);
  lv_obj_set_style_shadow_opa(btn_back, LV_OPA_20, 0);
  lv_obj_set_style_bg_color(btn_back, lv_color_darken(lv_color_hex(0xff3b30), 20), LV_STATE_PRESSED);
  lv_obj_add_event_cb(btn_back, btn_back_cb, LV_EVENT_CLICKED, NULL);

  lv_obj_t *back_icon = lv_label_create(btn_back);
  lv_label_set_text(back_icon, LV_SYMBOL_LEFT);
  lv_obj_set_style_text_font(back_icon, &lv_font_montserrat_24, 0);
  lv_obj_set_style_text_color(back_icon, lv_color_white(), 0);
  lv_obj_align(back_icon, LV_ALIGN_LEFT_MID, 25, 0);

  lv_obj_t *back_label = lv_label_create(btn_back);
  lv_label_set_text(back_label, txt->back);
  lv_obj_set_style_text_font(back_label, &lv_font_montserrat_24, 0);
  lv_obj_set_style_text_color(back_label, lv_color_white(), 0);
  lv_obj_align(back_label, LV_ALIGN_CENTER, 10, 0);

#ifdef DEBUG_MODE
  Serial.println("Settings screen initialized");
#endif
}

void ui_settings_show(void) {
  if (screen_settings == NULL) {
    ui_settings_init();
  }
  lv_screen_load(screen_settings);

#ifdef DEBUG_MODE
  Serial.println("Settings screen shown");
#endif
}

#endif