#ifndef UI_SETTINGS_PILOT_H
#define UI_SETTINGS_PILOT_H

#include "lvgl.h"
#include "constants.h"
#include "UI_helper.h"
#include "lang.h"
#include "globals.h"

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
  prefs.begin("pilot", true);
  
  String name = prefs.getString("name", "");
  String firstname = prefs.getString("firstname", "");
  String wing = prefs.getString("wing", "");
  String phone = prefs.getString("phone", "");
  
  if (widgets->ta_name) lv_textarea_set_text(widgets->ta_name, name.c_str());
  if (widgets->ta_firstname) lv_textarea_set_text(widgets->ta_firstname, firstname.c_str());
  if (widgets->ta_wing) lv_textarea_set_text(widgets->ta_wing, wing.c_str());
  if (widgets->ta_phone) lv_textarea_set_text(widgets->ta_phone, phone.c_str());
  
  prefs.end();
  
#ifdef DEBUG_MODE
  Serial.println("Pilot data loaded");
#endif
}

static void save_pilot_data(pilot_widgets_t *widgets) {
  prefs.begin("pilot", false);
  
  prefs.putString("name", lv_textarea_get_text(widgets->ta_name));
  prefs.putString("firstname", lv_textarea_get_text(widgets->ta_firstname));
  prefs.putString("wing", lv_textarea_get_text(widgets->ta_wing));
  prefs.putString("phone", lv_textarea_get_text(widgets->ta_phone));
  
  prefs.end();
  
#ifdef DEBUG_MODE
  Serial.println("Pilot data saved");
#endif
}

// Callbacks clavier
static void ta_event_cb(lv_event_t *e) {
  lv_obj_t *ta = (lv_obj_t*)lv_event_get_target(e);
  
  if (ta_active != ta) {
    ta_active = ta;
    lv_keyboard_set_textarea(keyboard, ta);
    lv_obj_clear_flag(keyboard, LV_OBJ_FLAG_HIDDEN);
    lv_obj_move_foreground(keyboard);
    force_full_refresh();
  }
}

static void keyboard_event_cb(lv_event_t *e) {
  lv_event_code_t code = lv_event_get_code(e);
  if (code == LV_EVENT_READY || code == LV_EVENT_CANCEL) {
    lv_obj_add_flag(keyboard, LV_OBJ_FLAG_HIDDEN);
    ta_active = NULL;
    force_full_refresh();
  }
}

// Callbacks boutons
static void btn_save_cb(lv_event_t *e) {
  pilot_widgets_t *widgets = (pilot_widgets_t*)lv_event_get_user_data(e);
  
#ifdef DEBUG_MODE
  Serial.println("Save pilot data clicked");
#endif
  save_pilot_data(widgets);
  ui_settings_show();
}

static void btn_cancel_cb(lv_event_t *e) {
#ifdef DEBUG_MODE
  Serial.println("Cancel pilot settings clicked");
#endif
  ui_settings_show();
}

void ui_settings_pilot_init(void) {
  const TextStrings *txt = get_text();

  lv_obj_t *main_frame = ui_create_black_screen_with_frame(3, 20, &main_screen);

  // Titre
  lv_obj_t *label_title = ui_create_label(main_frame, txt->pilot_settings,
                                           &lv_font_montserrat_32, lv_color_hex(0x00d4ff));
  lv_obj_align(label_title, LV_ALIGN_TOP_MID, 0, 0);

  // Structure pour les widgets
  static pilot_widgets_t widgets;

  // Container pour champs (2 colonnes)
  lv_obj_t *fields_container = ui_create_flex_container(main_frame, LV_FLEX_FLOW_ROW);
  lv_obj_set_size(fields_container, lv_pct(100), 230);
  lv_obj_align(fields_container, LV_ALIGN_TOP_MID, 0, 45);
  lv_obj_set_style_pad_all(fields_container, 10, 0);
  lv_obj_set_flex_align(fields_container, LV_FLEX_ALIGN_SPACE_EVENLY, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
  lv_obj_set_style_pad_column(fields_container, 20, 0);

  // Colonne gauche
  lv_obj_t *col_left = ui_create_form_column(fields_container, 450);
  widgets.ta_name = ui_create_input_field(col_left, txt->pilot_name, "", 30);
  lv_obj_add_event_cb(widgets.ta_name, ta_event_cb, LV_EVENT_CLICKED, NULL);
  
  widgets.ta_firstname = ui_create_input_field(col_left, txt->pilot_firstname, "", 30);
  lv_obj_add_event_cb(widgets.ta_firstname, ta_event_cb, LV_EVENT_CLICKED, NULL);

  // Colonne droite
  lv_obj_t *col_right = ui_create_form_column(fields_container, 450);
  widgets.ta_wing = ui_create_input_field(col_right, txt->pilot_wing, "", 30);
  lv_obj_add_event_cb(widgets.ta_wing, ta_event_cb, LV_EVENT_CLICKED, NULL);
  
  widgets.ta_phone = ui_create_input_field(col_right, txt->pilot_phone, "", 30);
  lv_obj_add_event_cb(widgets.ta_phone, ta_event_cb, LV_EVENT_CLICKED, NULL);

  // Clavier
  keyboard = ui_create_keyboard(main_frame, LV_KEYBOARD_MODE_TEXT_UPPER);
  lv_obj_add_event_cb(keyboard, keyboard_event_cb, LV_EVENT_ALL, NULL);

  ui_button_pair_t buttons = ui_create_save_cancel_buttons(main_frame, txt->save, txt->cancel, nullptr, true, true, false, btn_save_cb, btn_cancel_cb, nullptr);

  load_pilot_data(&widgets);

  lv_screen_load(main_screen);

#ifdef DEBUG_MODE
  Serial.println("Pilot settings screen initialized");
#endif
}

void ui_settings_pilot_show(void) {
  ui_settings_pilot_init();

#ifdef DEBUG_MODE
  Serial.println("Pilot settings screen shown");
#endif
}

#endif