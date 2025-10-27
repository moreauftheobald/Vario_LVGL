#ifndef UI_SETTINGS_H
#define UI_SETTINGS_H

#include "lvgl.h"
#include "constants.h"
#include "UI_helper.h"
#include "lang.h"
#include "ui_settings_pilot.h"
#include "ui_settings_wifi.h"
#include "ui_settings_map.h"
#include "ui_settings_system.h"

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
  ui_settings_system_show();  // Enlève le commentaire TODO
}

void ui_settings_init(void) {
  const TextStrings *txt = get_text();

  // Ecran et frame
  lv_obj_t *main_frame = ui_create_black_screen_with_frame(3, ROUND_FRANE_RADUIS_BIG, &screen_settings);

  ui_create_main_frame(main_frame, true, txt->settings);

  // Boutons de parametres
  lv_obj_t *btn_pilot = ui_create_button(main_left, txt->pilot_settings, LV_SYMBOL_HOME, lv_color_hex(FILES_BTN_COLOR),
                                         STP_BTN_W, STP_BTN_H, INFO_FONT_BIG, SETUP_SYMB, btn_pilot_cb,
                                         NULL, (lv_align_t)0, NULL, NULL);

  lv_obj_t *btn_screen = ui_create_button(main_left, txt->screen_calibration, LV_SYMBOL_SETTINGS, lv_color_hex(RESET_BTN_COLOR),
                                          STP_BTN_W, STP_BTN_H, INFO_FONT_BIG, SETUP_SYMB, btn_screen_cb,
                                          NULL, (lv_align_t)0, NULL, NULL);

  lv_obj_t *btn_map = ui_create_button(main_left, txt->map_settings, LV_SYMBOL_GPS, lv_color_hex(START_BTN_COLOR),
                                       STP_BTN_W, STP_BTN_H, INFO_FONT_BIG, SETUP_SYMB, btn_map_cb,
                                       NULL, (lv_align_t)0, NULL, NULL);

  lv_obj_t *btn_wifi = ui_create_button(main_right, txt->wifi_settings, LV_SYMBOL_WIFI, lv_color_hex(WIFI_BTN_COLOR),
                                        STP_BTN_W, STP_BTN_H, INFO_FONT_BIG, SETUP_SYMB, btn_wifi_cb,
                                        NULL, (lv_align_t)0, NULL, NULL);

  lv_obj_t *btn_vario = ui_create_button(main_right, txt->vario_settings, LV_SYMBOL_UP, lv_color_hex(VARIO_BTN_COLOR),
                                         STP_BTN_W, STP_BTN_H, INFO_FONT_BIG, SETUP_SYMB, btn_vario_cb,
                                         NULL, (lv_align_t)0, NULL, NULL);

  lv_obj_t *btn_system = ui_create_button(main_right, txt->system_settings, LV_SYMBOL_LIST, lv_color_hex(SETUP_BTN_COLOR),
                                          STP_BTN_W, STP_BTN_H, INFO_FONT_BIG, SETUP_SYMB, btn_system_cb,
                                          NULL, (lv_align_t)0, NULL, NULL);

  // Bouton retour
  lv_obj_t *btn_back = ui_create_button(btn_container, txt->back, LV_SYMBOL_BACKSPACE, lv_color_hex(CANCE_BTN_COLOR),
                                        PRE_BTN_W, PRE_BTN_H, INFO_FONT_BIG, INFO_FONT_BIG, btn_back_cb,
                                        NULL, LV_ALIGN_BOTTOM_MID, NULL, NULL);

#ifdef DEBUG_MODE
  Serial.println("Settings screen initialized");
#endif
}

void ui_settings_show(void) {
  // Sauvegarder ancien écran
  lv_obj_t *old_screen = lv_scr_act();

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

  // Détruire l'ancien écran SI ce n'est pas le même
  if (old_screen != main_screen && old_screen != NULL) {
    lv_obj_del(old_screen);
#ifdef DEBUG_MODE
    Serial.println("[PRESTART] Old screen deleted");
#endif
  }

#ifdef DEBUG_MODE
  Serial.println("Settings screen displayed");
#endif
}

#endif