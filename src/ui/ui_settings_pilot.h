#ifndef UI_SETTINGS_PILOT_H
#define UI_SETTINGS_PILOT_H

#include "lvgl.h"
#include "constants.h"
#include "UI_helper.h"
#include "lang.h"
#include "globals.h"
#include "src/params/params.h"

void ui_settings_show(void);

// Structure pour les widgets
typedef struct {
  lv_obj_t *ta_name;
  lv_obj_t *ta_firstname;
  lv_obj_t *ta_wing;
  lv_obj_t *ta_phone;
} pilot_widgets_t;

// Fonctions de sauvegarde/chargement
static void load_pilot_data(pilot_widgets_t *widgets) {
  if (widgets->ta_name)
    lv_textarea_set_text(widgets->ta_name, psram_str_get(params.pilot_name));
  if (widgets->ta_firstname)
    lv_textarea_set_text(widgets->ta_firstname, psram_str_get(params.pilot_firstname));
  if (widgets->ta_wing)
    lv_textarea_set_text(widgets->ta_wing, psram_str_get(params.pilot_wing));
  if (widgets->ta_phone)
    lv_textarea_set_text(widgets->ta_phone, psram_str_get(params.pilot_phone));

#ifdef DEBUG_MODE
  Serial.println("Pilot data loaded from params");
#endif
}

static void save_pilot_data(pilot_widgets_t *widgets) {
  // Sauvegarder depuis les textarea vers PSRAM
  psram_str_set(&params.pilot_name, lv_textarea_get_text(widgets->ta_name));
  psram_str_set(&params.pilot_firstname, lv_textarea_get_text(widgets->ta_firstname));
  psram_str_set(&params.pilot_wing, lv_textarea_get_text(widgets->ta_wing));
  psram_str_set(&params.pilot_phone, lv_textarea_get_text(widgets->ta_phone));

  params_save_pilot();

#ifdef DEBUG_MODE
  Serial.println("Pilot data saved to params");
#endif
}

// Callbacks clavier
static void ta_pilot_event_cb(lv_event_t *e) {
  lv_obj_t *ta = (lv_obj_t *)lv_event_get_target(e);

  if (ta_active != ta) {
    ta_active = ta;
    lv_keyboard_set_textarea(keyboard, ta);
    lv_obj_clear_flag(keyboard, LV_OBJ_FLAG_HIDDEN);
    lv_obj_move_foreground(keyboard);
    force_full_refresh();
  }
}

static void keyboard_pilot_event_cb(lv_event_t *e) {
  lv_event_code_t code = lv_event_get_code(e);
  if (code == LV_EVENT_READY || code == LV_EVENT_CANCEL) {
    lv_obj_add_flag(keyboard, LV_OBJ_FLAG_HIDDEN);
    ta_active = NULL;
    force_full_refresh();
  }
}

static void btn_save_pilot_cb(lv_event_t *e) {
  pilot_widgets_t *widgets = (pilot_widgets_t *)lv_event_get_user_data(e);

#ifdef DEBUG_MODE
  Serial.println("Save pilot data clicked");
#endif

  if (widgets == NULL) {
#ifdef DEBUG_MODE
    Serial.println("ERROR: widgets is NULL!");
#endif
    return;
  }

  if (widgets->ta_name == NULL || widgets->ta_firstname == NULL || widgets->ta_wing == NULL || widgets->ta_phone == NULL) {
#ifdef DEBUG_MODE
    Serial.println("ERROR: widget pointers are NULL!");
#endif
    return;
  }

  save_pilot_data(widgets);
  ui_settings_show();
}

static void btn_cancel_pilot_cb(lv_event_t *e) {
#ifdef DEBUG_MODE
  Serial.println("Cancel pilot settings clicked");
#endif
  ui_settings_show();
}

void ui_settings_pilot_init(void) {
  const TextStrings *txt = get_text();

  lv_obj_t *main_frame = ui_create_black_screen_with_frame(3, ROUND_FRANE_RADUIS_BIG, &main_screen);
  ui_create_main_frame(main_frame, false, txt->pilot_settings);

  // Widgets statiques
  static pilot_widgets_t widgets;

  // Nom
  lv_obj_t *name_row = ui_create_form_row(main_left, txt->pilot_name, PRE_LINE_HEADER_W, lv_color_hex(TITLE_COLOR));
  widgets.ta_name = ui_create_textarea(name_row, TXT_AREA_W, TXT_AREA_H, 32, true);
  lv_obj_add_event_cb(widgets.ta_name, ta_pilot_event_cb, LV_EVENT_FOCUSED, NULL);
  // Prenom
  lv_obj_t *firstname_row = ui_create_form_row(main_left, txt->pilot_firstname, PRE_LINE_HEADER_W, lv_color_hex(TITLE_COLOR));
  widgets.ta_firstname = ui_create_textarea(firstname_row, TXT_AREA_W, TXT_AREA_H, 32, true);
  lv_obj_add_event_cb(widgets.ta_firstname, ta_pilot_event_cb, LV_EVENT_FOCUSED, NULL);

  // Voile
  lv_obj_t *wing_row = ui_create_form_row(main_left, txt->pilot_wing, PRE_LINE_HEADER_W, lv_color_hex(TITLE_COLOR));
  widgets.ta_wing = ui_create_textarea(wing_row, TXT_AREA_W, TXT_AREA_H, 32, true);
  lv_obj_add_event_cb(widgets.ta_wing, ta_pilot_event_cb, LV_EVENT_FOCUSED, NULL);

  // Telephone
  lv_obj_t *phone_row = ui_create_form_row(main_left, txt->pilot_phone, PRE_LINE_HEADER_W, lv_color_hex(TITLE_COLOR));
  widgets.ta_phone = ui_create_textarea(phone_row, TXT_AREA_W, TXT_AREA_H, 32, true);
  lv_obj_add_event_cb(widgets.ta_phone, ta_pilot_event_cb, LV_EVENT_FOCUSED, NULL);

  // Clavier
  if (!keyboard) {
    keyboard = ui_create_keyboard(main_frame);
    lv_obj_add_event_cb(keyboard, keyboard_pilot_event_cb, LV_EVENT_ALL, NULL);
  }

  // Bouton Save
  lv_obj_t *btn_save_pilot = ui_create_button(btn_container, txt->save, LV_SYMBOL_SAVE, lv_color_hex(START_BTN_COLOR),
                                              PRE_BTN_W, PRE_BTN_H, INFO_FONT_S, INFO_FONT_BIG, btn_save_pilot_cb,
                                              &widgets, (lv_align_t)0, NULL, NULL);

  // Bouton Cancel
  lv_obj_t *btn_cancel_pilot = ui_create_button(btn_container, txt->cancel, LV_SYMBOL_BACKSPACE, lv_color_hex(CANCE_BTN_COLOR),
                                                PRE_BTN_W, PRE_BTN_H, INFO_FONT_S, INFO_FONT_BIG, btn_cancel_pilot_cb,
                                                NULL, (lv_align_t)0, NULL, NULL);

  // Charger les donnees
  load_pilot_data(&widgets);
  if (lvgl_port_lock(-1)) {
    lv_screen_load(main_screen);
    lvgl_port_unlock();
  }
  
#ifdef DEBUG_MODE
  Serial.println("Pilot settings screen initialized");
#endif
}

void ui_settings_pilot_show(void) {
  // Sauvegarder ancien écran
  lv_obj_t *old_screen = lv_scr_act();

  ui_settings_pilot_init();

  // Détruire l'ancien écran SI ce n'est pas le même
  if (old_screen != main_screen && old_screen != NULL) {
    lv_obj_del(old_screen);
#ifdef DEBUG_MODE
    Serial.println("[PRESTART] Old screen deleted");
#endif
  }
}

#endif