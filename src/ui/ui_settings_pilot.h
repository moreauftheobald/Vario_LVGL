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
  lv_obj_t *ta = (lv_obj_t*)lv_event_get_target(e);
  
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
  pilot_widgets_t *widgets = (pilot_widgets_t*)lv_event_get_user_data(e);
  
#ifdef DEBUG_MODE
  Serial.println("Save pilot data clicked");
#endif

  if (widgets == NULL) {
#ifdef DEBUG_MODE
    Serial.println("ERROR: widgets is NULL!");
#endif
    return;
  }
  
  if (widgets->ta_name == NULL || widgets->ta_firstname == NULL || 
      widgets->ta_wing == NULL || widgets->ta_phone == NULL) {
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
  
  lv_obj_t *main_frame = ui_create_black_screen_with_frame(3, 20, &main_screen);
  
  // Titre
  lv_obj_t *label_title = ui_create_label(main_frame, txt->pilot_settings,
                                           &lv_font_montserrat_32, lv_color_hex(0x00d4ff));
  lv_obj_align(label_title, LV_ALIGN_TOP_MID, 0, 0);
  
  // Container pour les champs
  lv_obj_t *fields_container = lv_obj_create(main_frame);
  lv_obj_set_size(fields_container, 980, 380);
  lv_obj_align(fields_container, LV_ALIGN_TOP_MID, 0, 60);
  lv_obj_set_style_bg_color(fields_container, lv_color_hex(0x1a2035), 0);
  lv_obj_set_style_bg_opa(fields_container, LV_OPA_80, 0);
  lv_obj_set_style_border_width(fields_container, 2, 0);
  lv_obj_set_style_border_color(fields_container, lv_color_hex(0x6080a0), 0);
  lv_obj_set_style_radius(fields_container, 15, 0);
  lv_obj_set_style_pad_all(fields_container, 20, 0);
  lv_obj_set_flex_flow(fields_container, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_flex_align(fields_container, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
  lv_obj_set_style_pad_row(fields_container, 15, 0);
  lv_obj_clear_flag(fields_container, LV_OBJ_FLAG_SCROLLABLE);
  
  // Widgets statiques
  static pilot_widgets_t widgets;
  
  // Nom
  lv_obj_t *name_row = ui_create_form_row(fields_container, txt->pilot_name, 
                                           200, lv_color_hex(0x00d4ff));
  widgets.ta_name = lv_textarea_create(name_row);
  lv_obj_set_size(widgets.ta_name, 700, 50);
  lv_textarea_set_one_line(widgets.ta_name, true);
  lv_textarea_set_max_length(widgets.ta_name, 32);
  lv_obj_set_style_bg_color(widgets.ta_name, lv_color_hex(0x0f1520), 0);
  lv_obj_set_style_border_color(widgets.ta_name, lv_color_hex(0x4080a0), 0);
  lv_obj_set_style_border_width(widgets.ta_name, 2, 0);
  lv_obj_set_style_radius(widgets.ta_name, 8, 0);
  lv_obj_set_style_text_color(widgets.ta_name, lv_color_white(), 0);
  lv_obj_set_style_text_font(widgets.ta_name, &lv_font_montserrat_20, 0);
  lv_obj_add_event_cb(widgets.ta_name, ta_pilot_event_cb, LV_EVENT_FOCUSED, NULL);
  
  // Prenom
  lv_obj_t *firstname_row = ui_create_form_row(fields_container, txt->pilot_firstname,
                                                 200, lv_color_hex(0x00d4ff));
  widgets.ta_firstname = lv_textarea_create(firstname_row);
  lv_obj_set_size(widgets.ta_firstname, 700, 50);
  lv_textarea_set_one_line(widgets.ta_firstname, true);
  lv_textarea_set_max_length(widgets.ta_firstname, 32);
  lv_obj_set_style_bg_color(widgets.ta_firstname, lv_color_hex(0x0f1520), 0);
  lv_obj_set_style_border_color(widgets.ta_firstname, lv_color_hex(0x4080a0), 0);
  lv_obj_set_style_border_width(widgets.ta_firstname, 2, 0);
  lv_obj_set_style_radius(widgets.ta_firstname, 8, 0);
  lv_obj_set_style_text_color(widgets.ta_firstname, lv_color_white(), 0);
  lv_obj_set_style_text_font(widgets.ta_firstname, &lv_font_montserrat_20, 0);
  lv_obj_add_event_cb(widgets.ta_firstname, ta_pilot_event_cb, LV_EVENT_FOCUSED, NULL);
  
  // Voile
  lv_obj_t *wing_row = ui_create_form_row(fields_container, txt->pilot_wing,
                                           200, lv_color_hex(0x00d4ff));
  widgets.ta_wing = lv_textarea_create(wing_row);
  lv_obj_set_size(widgets.ta_wing, 700, 50);
  lv_textarea_set_one_line(widgets.ta_wing, true);
  lv_textarea_set_max_length(widgets.ta_wing, 32);
  lv_obj_set_style_bg_color(widgets.ta_wing, lv_color_hex(0x0f1520), 0);
  lv_obj_set_style_border_color(widgets.ta_wing, lv_color_hex(0x4080a0), 0);
  lv_obj_set_style_border_width(widgets.ta_wing, 2, 0);
  lv_obj_set_style_radius(widgets.ta_wing, 8, 0);
  lv_obj_set_style_text_color(widgets.ta_wing, lv_color_white(), 0);
  lv_obj_set_style_text_font(widgets.ta_wing, &lv_font_montserrat_20, 0);
  lv_obj_add_event_cb(widgets.ta_wing, ta_pilot_event_cb, LV_EVENT_FOCUSED, NULL);
  
  // Telephone
  lv_obj_t *phone_row = ui_create_form_row(fields_container, txt->pilot_phone,
                                            200, lv_color_hex(0x00d4ff));
  widgets.ta_phone = lv_textarea_create(phone_row);
  lv_obj_set_size(widgets.ta_phone, 700, 50);
  lv_textarea_set_one_line(widgets.ta_phone, true);
  lv_textarea_set_max_length(widgets.ta_phone, 20);
  lv_obj_set_style_bg_color(widgets.ta_phone, lv_color_hex(0x0f1520), 0);
  lv_obj_set_style_border_color(widgets.ta_phone, lv_color_hex(0x4080a0), 0);
  lv_obj_set_style_border_width(widgets.ta_phone, 2, 0);
  lv_obj_set_style_radius(widgets.ta_phone, 8, 0);
  lv_obj_set_style_text_color(widgets.ta_phone, lv_color_white(), 0);
  lv_obj_set_style_text_font(widgets.ta_phone, &lv_font_montserrat_20, 0);
  lv_obj_add_event_cb(widgets.ta_phone, ta_pilot_event_cb, LV_EVENT_FOCUSED, NULL);
  
  // Clavier
  if (!keyboard) {
    keyboard = lv_keyboard_create(main_frame);
    lv_obj_set_size(keyboard, lv_pct(100), lv_pct(40));
    lv_obj_align(keyboard, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_add_flag(keyboard, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_event_cb(keyboard, keyboard_pilot_event_cb, LV_EVENT_ALL, NULL);
  }
  
  // Boutons
  ui_button_pair_t buttons = ui_create_save_cancel_buttons(main_frame, txt->save, txt->cancel,
                                                            nullptr, true, true, false,
                                                            btn_save_pilot_cb, btn_cancel_pilot_cb, nullptr,
                                                            &widgets, NULL, NULL);
  
  // Charger les donnees
  load_pilot_data(&widgets);
  
  lv_screen_load(main_screen);
  
#ifdef DEBUG_MODE
  Serial.println("Pilot settings screen initialized");
#endif
}

void ui_settings_pilot_show(void) {
  ui_settings_pilot_init();
}

#endif