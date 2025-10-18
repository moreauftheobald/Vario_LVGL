#ifndef UI_SETTINGS_H
#define UI_SETTINGS_H

#include "lvgl.h"
#include "constants.h"
#include "UI_helper.h"
#include "lang.h"
#include "ui_settings_pilot.h"
#include "ui_settings_wifi.h"
#include "ui_settings_map.h"

// Forward declarations
void ui_prestart_show(void);
void ui_settings_pilot_show(void);
void ui_settings_wifi_show(void);
void ui_settings_screen_show(void);
void ui_settings_vario_show(void);
void ui_settings_map_show(void);
void ui_settings_system_show(void);

static lv_obj_t *screen_settings = NULL;

// Callbacks
static void btn_back_cb(lv_event_t *e) {
#ifdef DEBUG_MODE
  Serial.println("Back button clicked");
#endif
  ui_prestart_show();
}

static void btn_pilot_cb(lv_event_t *e) {
#ifdef DEBUG_MODE
  Serial.println("Pilot settings clicked");
#endif
  ui_settings_pilot_show();
}

static void btn_wifi_cb(lv_event_t *e) {
#ifdef DEBUG_MODE
  Serial.println("WiFi settings clicked");
#endif
  ui_settings_wifi_show();
}

static void btn_screen_cb(lv_event_t *e) {
#ifdef DEBUG_MODE
  Serial.println("Screen calibration clicked");
#endif
  ui_settings_screen_show();
}

static void btn_vario_cb(lv_event_t *e) {
#ifdef DEBUG_MODE
  Serial.println("Vario settings clicked");
#endif
  ui_settings_vario_show();
}

static void btn_map_cb(lv_event_t *e) {
#ifdef DEBUG_MODE
  Serial.println("Map settings clicked");
#endif
  ui_settings_map_show();
}

static void btn_system_cb(lv_event_t *e) {
#ifdef DEBUG_MODE
  Serial.println("System settings clicked");
#endif
  // TODO: ui_settings_system_show();
}

void ui_settings_init(void) {
  const TextStrings *txt = get_text();

  // Ecran et frame
  lv_obj_t *main_frame = ui_create_black_screen_with_frame(3, 20, &screen_settings);

  // Titre
  ui_create_title(main_frame, txt->settings);

  // Container pour les boutons (2 colonnes)
  lv_obj_t *buttons_container = ui_create_flex_container(main_frame, LV_FLEX_FLOW_ROW_WRAP);
  lv_obj_set_size(buttons_container, 940, 450);
  lv_obj_align(buttons_container, LV_ALIGN_CENTER, 0, 15);
  lv_obj_set_flex_align(buttons_container, LV_FLEX_ALIGN_SPACE_EVENLY, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER);
  lv_obj_set_style_pad_column(buttons_container, 20, 0);
  lv_obj_set_style_pad_row(buttons_container, 15, 0);

  // Boutons de parametres
  lv_obj_t *btn_pilot = ui_create_button(buttons_container, txt->pilot_settings,
                                                  LV_SYMBOL_HOME, lv_color_hex(0x5856d6), 460,80, (lv_align_t)0, NULL, NULL);
  lv_obj_add_event_cb(btn_pilot, btn_pilot_cb, LV_EVENT_CLICKED, NULL);

  lv_obj_t *btn_wifi = ui_create_button(buttons_container, txt->wifi_settings,
                                                 LV_SYMBOL_WIFI, lv_color_hex(0x007aff),460,80, (lv_align_t)0, NULL, NULL);
  lv_obj_add_event_cb(btn_wifi, btn_wifi_cb, LV_EVENT_CLICKED, NULL);

  lv_obj_t *btn_screen = ui_create_button(buttons_container, txt->screen_calibration,
                                                   LV_SYMBOL_SETTINGS, lv_color_hex(0xff9500),460,80, (lv_align_t)0, NULL, NULL);
  lv_obj_add_event_cb(btn_screen, btn_screen_cb, LV_EVENT_CLICKED, NULL);

  lv_obj_t *btn_vario = ui_create_button(buttons_container, txt->vario_settings,
                                                  LV_SYMBOL_UP, lv_color_hex(0x4cd964),460,80, (lv_align_t)0, NULL, NULL);
  lv_obj_add_event_cb(btn_vario, btn_vario_cb, LV_EVENT_CLICKED, NULL);

  lv_obj_t *btn_map = ui_create_button(buttons_container, txt->map_settings,
                                                LV_SYMBOL_GPS, lv_color_hex(0x34c759),460,80, (lv_align_t)0, NULL, NULL);
  lv_obj_add_event_cb(btn_map, btn_map_cb, LV_EVENT_CLICKED, NULL);

  lv_obj_t *btn_system = ui_create_button(buttons_container, txt->system_settings,
                                                   LV_SYMBOL_LIST, lv_color_hex(0x8e8e93),460,80, (lv_align_t)0, NULL, NULL);
  lv_obj_add_event_cb(btn_system, btn_system_cb, LV_EVENT_CLICKED, NULL);

  // Bouton retour
  lv_obj_t *btn_back = ui_create_button(main_frame, txt->back, NULL,lv_color_hex(0xff3b30), 300, 70, LV_ALIGN_BOTTOM_MID, 0, -15);
  lv_obj_add_event_cb(btn_back, btn_back_cb, LV_EVENT_CLICKED, NULL);

#ifdef DEBUG_MODE
  Serial.println("Settings screen initialized");
#endif
}

void ui_settings_show(void) {
  if (screen_settings == NULL) {
    if (lvgl_port_lock(-1)) {
      ui_settings_init();
      lvgl_port_unlock();
    }
  }

  if (lvgl_port_lock(-1)) {
    lv_screen_load(screen_settings);
    force_full_refresh();
    lvgl_port_unlock();
  }

#ifdef DEBUG_MODE
  Serial.println("Settings screen displayed");
#endif
}

#endif