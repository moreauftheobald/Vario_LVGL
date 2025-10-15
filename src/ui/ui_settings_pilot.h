#ifndef UI_SETTINGS_PILOT_H
#define UI_SETTINGS_PILOT_H

#include "lvgl.h"
#include "constants.h"
#include "globals.h"

void ui_settings_show(void);

static lv_obj_t *screen_settings_pilot = NULL;
static lv_obj_t *ta_name = NULL;
static lv_obj_t *ta_firstname = NULL;
static lv_obj_t *ta_wing = NULL;
static lv_obj_t *ta_phone = NULL;
static lv_obj_t *fields_container = NULL;

// SUPPRIMER force_full_refresh() - elle est dans globals.h

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

static lv_obj_t *create_input_field(lv_obj_t *parent, const char *label_text) {
  lv_obj_t *container = lv_obj_create(parent);
  lv_obj_set_size(container, lv_pct(100), LV_SIZE_CONTENT);
  lv_obj_set_style_bg_opa(container, LV_OPA_TRANSP, 0);
  lv_obj_set_style_border_width(container, 0, 0);
  lv_obj_set_style_pad_all(container, 0, 0);
  lv_obj_set_flex_flow(container, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_flex_align(container, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
  lv_obj_set_style_pad_row(container, 5, 0);
  lv_obj_clear_flag(container, LV_OBJ_FLAG_SCROLLABLE);

  lv_obj_t *label = lv_label_create(container);
  lv_label_set_text(label, label_text);
  lv_obj_set_style_text_font(label, &lv_font_montserrat_20, 0);
  lv_obj_set_style_text_color(label, lv_color_hex(0x00d4ff), 0);

  lv_obj_t *ta = lv_textarea_create(container);
  lv_obj_set_size(ta, lv_pct(100), 55);
  lv_textarea_set_one_line(ta, true);
  lv_textarea_set_max_length(ta, 30);
  lv_obj_set_style_bg_color(ta, lv_color_hex(0x1a2035), 0);
  lv_obj_set_style_border_color(ta, lv_color_hex(0x6080a0), 0);
  lv_obj_set_style_border_width(ta, 2, 0);
  lv_obj_set_style_radius(ta, 8, 0);
  lv_obj_set_style_text_color(ta, lv_color_white(), 0);
  lv_obj_set_style_text_font(ta, &lv_font_montserrat_20, 0);
  lv_obj_set_style_pad_all(ta, 8, 0);
  lv_obj_add_event_cb(ta, ta_event_cb, LV_EVENT_CLICKED, NULL);

  return ta;
}

void ui_settings_pilot_init(void) {
  const TextStrings *txt = get_text();

  screen_settings_pilot = lv_obj_create(NULL);
  lv_obj_set_style_bg_color(screen_settings_pilot, lv_color_hex(0x0a0e27), 0);
  lv_obj_set_style_bg_grad_color(screen_settings_pilot, lv_color_hex(0x1a1f3a), 0);
  lv_obj_set_style_bg_grad_dir(screen_settings_pilot, LV_GRAD_DIR_VER, 0);

  lv_obj_t *main_frame = lv_obj_create(screen_settings_pilot);
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
  lv_obj_clear_flag(main_frame, LV_OBJ_FLAG_SCROLLABLE);

  // Titre (plus haut)
  lv_obj_t *label_title = lv_label_create(main_frame);
  lv_label_set_text(label_title, txt->pilot_settings);
  lv_obj_set_style_text_font(label_title, &lv_font_montserrat_32, 0);
  lv_obj_set_style_text_color(label_title, lv_color_hex(0x00d4ff), 0);
  lv_obj_align(label_title, LV_ALIGN_TOP_MID, 0, 0);

  // Zone haute: Champs en 2 colonnes (plus haut et moins haut)
  fields_container = lv_obj_create(main_frame);
  lv_obj_set_size(fields_container, lv_pct(100), 230);
  lv_obj_align(fields_container, LV_ALIGN_TOP_MID, 0, 45);
  lv_obj_set_style_bg_opa(fields_container, LV_OPA_TRANSP, 0);
  lv_obj_set_style_border_width(fields_container, 0, 0);
  lv_obj_set_style_pad_all(fields_container, 10, 0);
  lv_obj_set_flex_flow(fields_container, LV_FLEX_FLOW_ROW);
  lv_obj_set_flex_align(fields_container, LV_FLEX_ALIGN_SPACE_EVENLY, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
  lv_obj_set_style_pad_column(fields_container, 20, 0);
  lv_obj_clear_flag(fields_container, LV_OBJ_FLAG_SCROLLABLE);

  // Colonne gauche
  lv_obj_t *col_left = lv_obj_create(fields_container);
  lv_obj_set_size(col_left, 450, LV_SIZE_CONTENT);
  lv_obj_set_style_bg_color(col_left, lv_color_hex(0x1a2035), 0);
  lv_obj_set_style_bg_opa(col_left, LV_OPA_80, 0);
  lv_obj_set_style_border_width(col_left, 2, 0);
  lv_obj_set_style_border_color(col_left, lv_color_hex(0x6080a0), 0);
  lv_obj_set_style_radius(col_left, 15, 0);
  lv_obj_set_style_pad_all(col_left, 15, 0);
  lv_obj_set_flex_flow(col_left, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_flex_align(col_left, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
  lv_obj_set_style_pad_row(col_left, 15, 0);
  lv_obj_clear_flag(col_left, LV_OBJ_FLAG_SCROLLABLE);

  ta_name = create_input_field(col_left, txt->pilot_name);
  ta_firstname = create_input_field(col_left, txt->pilot_firstname);

  // Colonne droite
  lv_obj_t *col_right = lv_obj_create(fields_container);
  lv_obj_set_size(col_right, 450, LV_SIZE_CONTENT);
  lv_obj_set_style_bg_color(col_right, lv_color_hex(0x1a2035), 0);
  lv_obj_set_style_bg_opa(col_right, LV_OPA_80, 0);
  lv_obj_set_style_border_width(col_right, 2, 0);
  lv_obj_set_style_border_color(col_right, lv_color_hex(0x6080a0), 0);
  lv_obj_set_style_radius(col_right, 15, 0);
  lv_obj_set_style_pad_all(col_right, 15, 0);
  lv_obj_set_flex_flow(col_right, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_flex_align(col_right, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
  lv_obj_set_style_pad_row(col_right, 15, 0);
  lv_obj_clear_flag(col_right, LV_OBJ_FLAG_SCROLLABLE);

  ta_wing = create_input_field(col_right, txt->pilot_wing);
  ta_phone = create_input_field(col_right, txt->pilot_phone);

  // Zone basse: Clavier (plus bas)
  keyboard = lv_keyboard_create(main_frame);
  lv_obj_set_size(keyboard, lv_pct(98), 220);
  lv_obj_align(keyboard, LV_ALIGN_BOTTOM_MID, 0, -70);
  lv_obj_set_style_bg_color(keyboard, lv_color_hex(0x1a2035), 0);
  lv_obj_set_style_bg_opa(keyboard, LV_OPA_90, 0);
  lv_obj_set_style_border_width(keyboard, 2, 0);
  lv_obj_set_style_border_color(keyboard, lv_color_hex(0x6080a0), 0);
  lv_obj_set_style_radius(keyboard, 10, 0);
  lv_keyboard_set_mode(keyboard, LV_KEYBOARD_MODE_TEXT_UPPER);
  lv_obj_add_flag(keyboard, LV_OBJ_FLAG_HIDDEN);
  lv_obj_clear_flag(keyboard, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_add_event_cb(keyboard, keyboard_event_cb, LV_EVENT_ALL, NULL);

  // Boutons Save/Cancel (plus bas)
  lv_obj_t *btn_container = lv_obj_create(main_frame);
  lv_obj_set_size(btn_container, lv_pct(100), LV_SIZE_CONTENT);
  lv_obj_align(btn_container, LV_ALIGN_BOTTOM_MID, 0, -5);
  lv_obj_set_style_bg_opa(btn_container, LV_OPA_TRANSP, 0);
  lv_obj_set_style_border_width(btn_container, 0, 0);
  lv_obj_set_style_pad_all(btn_container, 0, 0);
  lv_obj_set_flex_flow(btn_container, LV_FLEX_FLOW_ROW);
  lv_obj_set_flex_align(btn_container, LV_FLEX_ALIGN_SPACE_EVENLY, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
  lv_obj_clear_flag(btn_container, LV_OBJ_FLAG_SCROLLABLE);

  lv_obj_t *btn_save = lv_button_create(btn_container);
  lv_obj_set_size(btn_save, 220, 50);
  lv_obj_set_style_bg_color(btn_save, lv_color_hex(0x34c759), 0);
  lv_obj_set_style_radius(btn_save, 12, 0);
  lv_obj_set_style_shadow_width(btn_save, 5, 0);
  lv_obj_set_style_shadow_color(btn_save, lv_color_black(), 0);
  lv_obj_set_style_shadow_opa(btn_save, LV_OPA_20, 0);
  lv_obj_add_event_cb(btn_save, btn_save_cb, LV_EVENT_CLICKED, NULL);

  lv_obj_t *label_save = lv_label_create(btn_save);
  lv_label_set_text(label_save, txt->save);
  lv_obj_set_style_text_font(label_save, &lv_font_montserrat_24, 0);
  lv_obj_set_style_text_color(label_save, lv_color_white(), 0);
  lv_obj_center(label_save);

  lv_obj_t *btn_cancel = lv_button_create(btn_container);
  lv_obj_set_size(btn_cancel, 220, 50);
  lv_obj_set_style_bg_color(btn_cancel, lv_color_hex(0xff3b30), 0);
  lv_obj_set_style_radius(btn_cancel, 12, 0);
  lv_obj_set_style_shadow_width(btn_cancel, 5, 0);
  lv_obj_set_style_shadow_color(btn_cancel, lv_color_black(), 0);
  lv_obj_set_style_shadow_opa(btn_cancel, LV_OPA_20, 0);
  lv_obj_add_event_cb(btn_cancel, btn_cancel_cb, LV_EVENT_CLICKED, NULL);

  lv_obj_t *label_cancel = lv_label_create(btn_cancel);
  lv_label_set_text(label_cancel, txt->cancel);
  lv_obj_set_style_text_font(label_cancel, &lv_font_montserrat_24, 0);
  lv_obj_set_style_text_color(label_cancel, lv_color_white(), 0);
  lv_obj_center(label_cancel);

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