#ifndef UI_SETTINGS_PILOT_H
#define UI_SETTINGS_PILOT_H

#include "lvgl.h"
#include "constants.h"
#include "UI_helper.h"
#include "lang.h"
#include "globals.h"

void ui_settings_show(void);

static lv_obj_t *screen_settings_pilot = NULL;
static lv_obj_t *ta_name = NULL;
static lv_obj_t *ta_firstname = NULL;
static lv_obj_t *ta_wing = NULL;
static lv_obj_t *ta_phone = NULL;

// Fonctions de sauvegarde/chargement
static void load_pilot_data(void) {
  prefs.begin("pilot", true);
  
  String name = prefs.getString("name", "");
  String firstname = prefs.getString("firstname", "");
  String wing = prefs.getString("wing", "");
  String phone = prefs.getString("phone", "");
  
  if (ta_name) lv_textarea_set_text(ta_name, name.c_str());
  if (ta_firstname) lv_textarea_set_text(ta_firstname, firstname.c_str());
  if (ta_wing) lv_textarea_set_text(ta_wing, wing.c_str());
  if (ta_phone) lv_textarea_set_text(ta_phone, phone.c_str());
  
  prefs.end();
  
#ifdef DEBUG_MODE
  Serial.println("Pilot data loaded");
#endif
}

static void save_pilot_data(void) {
  prefs.begin("pilot", false);
  
  prefs.putString("name", lv_textarea_get_text(ta_name));
  prefs.putString("firstname", lv_textarea_get_text(ta_firstname));
  prefs.putString("wing", lv_textarea_get_text(ta_wing));
  prefs.putString("phone", lv_textarea_get_text(ta_phone));
  
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
#ifdef DEBUG_MODE
  Serial.println("Save pilot data clicked");
#endif
  save_pilot_data();
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

  // Ecran et frame
  screen_settings_pilot = ui_create_screen();
  lv_obj_t *main_frame = ui_create_main_frame(screen_settings_pilot);
  lv_obj_clear_flag(main_frame, LV_OBJ_FLAG_SCROLLABLE);

  // Titre
  lv_obj_t *label_title = ui_create_label(main_frame, txt->pilot_settings,
                                           &lv_font_montserrat_32, lv_color_hex(0x00d4ff));
  lv_obj_align(label_title, LV_ALIGN_TOP_MID, 0, 0);

  // Container pour champs (2 colonnes)
  lv_obj_t *fields_container = ui_create_flex_container(main_frame, LV_FLEX_FLOW_ROW);
  lv_obj_set_size(fields_container, lv_pct(100), 230);
  lv_obj_align(fields_container, LV_ALIGN_TOP_MID, 0, 45);
  lv_obj_set_style_pad_all(fields_container, 10, 0);
  lv_obj_set_flex_align(fields_container, LV_FLEX_ALIGN_SPACE_EVENLY, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
  lv_obj_set_style_pad_column(fields_container, 20, 0);

  // Colonne gauche
  lv_obj_t *col_left = ui_create_form_column(fields_container, 450);
  ta_name = ui_create_input_field(col_left, txt->pilot_name, "", 30);
  lv_obj_add_event_cb(ta_name, ta_event_cb, LV_EVENT_CLICKED, NULL);
  
  ta_firstname = ui_create_input_field(col_left, txt->pilot_firstname, "", 30);
  lv_obj_add_event_cb(ta_firstname, ta_event_cb, LV_EVENT_CLICKED, NULL);

  // Colonne droite
  lv_obj_t *col_right = ui_create_form_column(fields_container, 450);
  ta_wing = ui_create_input_field(col_right, txt->pilot_wing, "", 30);
  lv_obj_add_event_cb(ta_wing, ta_event_cb, LV_EVENT_CLICKED, NULL);
  
  ta_phone = ui_create_input_field(col_right, txt->pilot_phone, "", 30);
  lv_obj_add_event_cb(ta_phone, ta_event_cb, LV_EVENT_CLICKED, NULL);

  // Clavier
  keyboard = ui_create_keyboard(main_frame, LV_KEYBOARD_MODE_TEXT_UPPER);
  lv_obj_add_event_cb(keyboard, keyboard_event_cb, LV_EVENT_ALL, NULL);

  // Boutons Save/Cancel
  lv_obj_t *btn_container = ui_create_flex_container(main_frame, LV_FLEX_FLOW_ROW);
  lv_obj_set_size(btn_container, lv_pct(100), LV_SIZE_CONTENT);
  lv_obj_align(btn_container, LV_ALIGN_BOTTOM_MID, 0, -5);
  lv_obj_set_flex_align(btn_container, LV_FLEX_ALIGN_SPACE_EVENLY, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

  ui_button_pair_t buttons = ui_create_save_cancel_buttons(btn_container, txt->save, txt->cancel);
  lv_obj_add_event_cb(buttons.save, btn_save_cb, LV_EVENT_CLICKED, NULL);
  lv_obj_add_event_cb(buttons.cancel, btn_cancel_cb, LV_EVENT_CLICKED, NULL);

  load_pilot_data();

#ifdef DEBUG_MODE
  Serial.println("Pilot settings screen initialized");
#endif
}

void ui_settings_pilot_show(void) {
  if (screen_settings_pilot == NULL) {
    ui_settings_pilot_init();
  }
  lv_screen_load(screen_settings_pilot);

#ifdef DEBUG_MODE
  Serial.println("Pilot settings screen shown");
#endif
}

#endif