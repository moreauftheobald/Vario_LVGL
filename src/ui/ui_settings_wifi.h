#ifndef UI_SETTINGS_WIFI_H
#define UI_SETTINGS_WIFI_H

#include "lvgl.h"
#include "constants.h"
#include "UI_helper.h"
#include "lang.h"
#include "globals.h"
#include "src/params/params.h"

void ui_settings_show(void);

static lv_obj_t *dropdown_priority = NULL;
static lv_obj_t *ta_ssid = NULL;
static lv_obj_t *ta_password = NULL;
static int current_priority = 0;

// Donnees temporaires pour les 4 priorites
static String wifi_data_ssid[4] = {"", "", "", ""};
static String wifi_data_pass[4] = {"", "", "", ""};

static void load_all_wifi_data(void) {
  for (int i = 0; i < 4; i++) {
    wifi_data_ssid[i] = params.wifi_ssid[i];
    wifi_data_pass[i] = params.wifi_password[i];
  }
  
#ifdef DEBUG_MODE
  Serial.println("All WiFi data loaded from params");
#endif
}

static void save_all_wifi_data(void) {
  for (int i = 0; i < 4; i++) {
    params.wifi_ssid[i] = wifi_data_ssid[i];
    params.wifi_password[i] = wifi_data_pass[i];
  }
  
  params_save_wifi();
  
#ifdef DEBUG_MODE
  Serial.println("All WiFi data saved to params");
#endif
}

static void save_current_fields_to_memory(void) {
  if (ta_ssid && ta_password) {
    // Récupérer les textes
    String ssid = String(lv_textarea_get_text(ta_ssid));
    String pass = String(lv_textarea_get_text(ta_password));
    
    // TRIM : Supprimer les espaces et caractères invisibles
    ssid.trim();
    pass.trim();
    
    wifi_data_ssid[current_priority] = ssid;
    wifi_data_pass[current_priority] = pass;
    
#ifdef DEBUG_MODE
    Serial.printf("Saved SSID: '%s' (len=%d)\n", ssid.c_str(), ssid.length());
    Serial.printf("Saved Pass: '***' (len=%d)\n", pass.length());
#endif
  }
}

static void load_current_priority_to_fields(void) {
  if (ta_ssid && ta_password) {
    lv_textarea_set_text(ta_ssid, wifi_data_ssid[current_priority].c_str());
    lv_textarea_set_text(ta_password, wifi_data_pass[current_priority].c_str());
  }
}

// Callbacks
static void dropdown_event_cb(lv_event_t *e) {
  lv_event_code_t code = lv_event_get_code(e);
  if (code == LV_EVENT_VALUE_CHANGED) {
    save_current_fields_to_memory();
    current_priority = lv_dropdown_get_selected(dropdown_priority);
    load_current_priority_to_fields();
    
#ifdef DEBUG_MODE
    Serial.printf("Priority changed to: %d\n", current_priority + 1);
#endif
  }
}

static void ta_wifi_event_cb(lv_event_t *e) {
  lv_obj_t *ta = (lv_obj_t*)lv_event_get_target(e);
  
  if (ta_active != ta) {
    ta_active = ta;
    lv_keyboard_set_textarea(keyboard, ta);
    lv_obj_clear_flag(keyboard, LV_OBJ_FLAG_HIDDEN);
    lv_obj_move_foreground(keyboard);
    force_full_refresh();
  }
}

static void keyboard_wifi_event_cb(lv_event_t *e) {
  lv_event_code_t code = lv_event_get_code(e);
  if (code == LV_EVENT_READY || code == LV_EVENT_CANCEL) {
    lv_obj_add_flag(keyboard, LV_OBJ_FLAG_HIDDEN);
    ta_active = NULL;
    force_full_refresh();
  }
}

static void btn_save_wifi_cb(lv_event_t *e) {
#ifdef DEBUG_MODE
  Serial.println("Save WiFi data clicked");
#endif
  
  save_current_fields_to_memory();
  
  // Vérifier avant sauvegarde
#ifdef DEBUG_MODE
  for (int i = 0; i < 4; i++) {
    Serial.printf("Priority %d: SSID='%s' (len=%d), Pass len=%d\n", 
                  i+1, 
                  wifi_data_ssid[i].c_str(), 
                  wifi_data_ssid[i].length(),
                  wifi_data_pass[i].length());
  }
#endif
  
  save_all_wifi_data();
  ui_settings_show();
}

static void btn_cancel_wifi_cb(lv_event_t *e) {
#ifdef DEBUG_MODE
  Serial.println("Cancel WiFi settings clicked");
#endif
  ui_settings_show();
}

void ui_settings_wifi_init(void) {
  const TextStrings *txt = get_text();
  
  load_all_wifi_data();
  current_priority = 0;
  
  lv_obj_t *main_frame = ui_create_black_screen_with_frame(3, 20, &main_screen);
  
  // Titre
  lv_obj_t *label_title = ui_create_label(main_frame, txt->wifi_settings,
                                           &lv_font_montserrat_32, lv_color_hex(0x00d4ff));
  lv_obj_align(label_title, LV_ALIGN_TOP_MID, 0, 0);
  
  // Container pour les champs
  lv_obj_t *fields_container = lv_obj_create(main_frame);
  lv_obj_set_size(fields_container, 980, 180);
  lv_obj_align(fields_container, LV_ALIGN_TOP_MID, 0, 50);
  lv_obj_set_style_bg_color(fields_container, lv_color_hex(0x1a2035), 0);
  lv_obj_set_style_bg_opa(fields_container, LV_OPA_80, 0);
  lv_obj_set_style_border_width(fields_container, 2, 0);
  lv_obj_set_style_border_color(fields_container, lv_color_hex(0x6080a0), 0);
  lv_obj_set_style_radius(fields_container, 15, 0);
  lv_obj_set_style_pad_all(fields_container, 20, 0);
  lv_obj_set_flex_flow(fields_container, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_flex_align(fields_container, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
  lv_obj_set_style_pad_row(fields_container, 10, 0);
  lv_obj_clear_flag(fields_container, LV_OBJ_FLAG_SCROLLABLE);
  
  // Ligne Priorite
  lv_obj_t *priority_row = ui_create_form_row(fields_container, txt->wifi_priority, 
                                                140, lv_color_hex(0xff9500));
  
  dropdown_priority = lv_dropdown_create(priority_row);
  lv_dropdown_set_options(dropdown_priority, "Priorite 1\nPriorite 2\nPriorite 3\nPriorite 4");
  lv_obj_set_width(dropdown_priority, 760);
  lv_obj_set_style_bg_color(dropdown_priority, lv_color_hex(0x0f1520), 0);
  lv_obj_set_style_text_color(dropdown_priority, lv_color_white(), 0);
  lv_obj_set_style_text_font(dropdown_priority, &lv_font_montserrat_20, 0);
  lv_obj_add_event_cb(dropdown_priority, dropdown_event_cb, LV_EVENT_ALL, NULL);
  
  // Ligne SSID
  lv_obj_t *ssid_row = ui_create_form_row(fields_container, txt->wifi_ssid, 
                                           140, lv_color_hex(0x00d4ff));
  
  ta_ssid = lv_textarea_create(ssid_row);
  lv_obj_set_size(ta_ssid, 760, 50);
  lv_textarea_set_one_line(ta_ssid, true);
  lv_textarea_set_max_length(ta_ssid, 32);
  lv_obj_set_style_bg_color(ta_ssid, lv_color_hex(0x0f1520), 0);
  lv_obj_set_style_border_color(ta_ssid, lv_color_hex(0x4080a0), 0);
  lv_obj_set_style_border_width(ta_ssid, 2, 0);
  lv_obj_set_style_radius(ta_ssid, 8, 0);
  lv_obj_set_style_text_color(ta_ssid, lv_color_white(), 0);
  lv_obj_set_style_text_font(ta_ssid, &lv_font_montserrat_20, 0);
  lv_obj_set_style_pad_all(ta_ssid, 8, 0);
  lv_obj_add_event_cb(ta_ssid, ta_wifi_event_cb, LV_EVENT_CLICKED, NULL);
  
  // Ligne Password
  lv_obj_t *pass_row = ui_create_form_row(fields_container, txt->wifi_password, 
                                           140, lv_color_hex(0x00d4ff));
  
  ta_password = lv_textarea_create(pass_row);
  lv_obj_set_size(ta_password, 760, 50);
  lv_textarea_set_one_line(ta_password, true);
  lv_textarea_set_max_length(ta_password, 64);
  lv_obj_set_style_bg_color(ta_password, lv_color_hex(0x0f1520), 0);
  lv_obj_set_style_border_color(ta_password, lv_color_hex(0x4080a0), 0);
  lv_obj_set_style_border_width(ta_password, 2, 0);
  lv_obj_set_style_radius(ta_password, 8, 0);
  lv_obj_set_style_text_color(ta_password, lv_color_white(), 0);
  lv_obj_set_style_text_font(ta_password, &lv_font_montserrat_20, 0);
  lv_obj_set_style_pad_all(ta_password, 8, 0);
  lv_obj_add_event_cb(ta_password, ta_wifi_event_cb, LV_EVENT_CLICKED, NULL);
  
  load_current_priority_to_fields();
  
  // Clavier
  keyboard = ui_create_keyboard(main_frame, LV_KEYBOARD_MODE_TEXT_LOWER);
  lv_obj_set_size(keyboard, 980, 240);
  lv_obj_add_event_cb(keyboard, keyboard_wifi_event_cb, LV_EVENT_ALL, NULL);
  
  // Boutons Save/Cancel
  lv_obj_t *btn_container = ui_create_flex_container(main_frame, LV_FLEX_FLOW_ROW);
  lv_obj_set_size(btn_container, lv_pct(100), LV_SIZE_CONTENT);
  lv_obj_align(btn_container, LV_ALIGN_BOTTOM_MID, 0, -5);
  lv_obj_set_flex_align(btn_container, LV_FLEX_ALIGN_SPACE_EVENLY, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
  
  ui_button_pair_t buttons = ui_create_save_cancel_buttons(btn_container, txt->save, txt->cancel, nullptr, true, true, false, btn_save_wifi_cb, btn_cancel_wifi_cb, nullptr, NULL, NULL, NULL);

  lv_screen_load(main_screen);

#ifdef DEBUG_MODE
  Serial.println("WiFi settings screen initialized");
#endif
}

void ui_settings_wifi_show(void) {
  ui_settings_wifi_init();
  
#ifdef DEBUG_MODE
  Serial.println("WiFi settings screen shown");
#endif
}

#endif