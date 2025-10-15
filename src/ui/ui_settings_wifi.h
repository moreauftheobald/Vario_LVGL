#ifndef UI_SETTINGS_WIFI_H
#define UI_SETTINGS_WIFI_H

#include "lvgl.h"
#include "constants.h"
#include "globals.h"

void ui_settings_show(void);

static lv_obj_t *screen_settings_wifi = NULL;
static lv_obj_t *dropdown_priority = NULL;
static lv_obj_t *ta_ssid = NULL;
static lv_obj_t *ta_password = NULL;
static int current_priority = 0;

// Donnees temporaires pour les 4 priorites
static String wifi_data_ssid[4] = {"", "", "", ""};
static String wifi_data_pass[4] = {"", "", "", ""};

static void load_all_wifi_data(void) {
    prefs.begin("wifi", true);
    
    for (int i = 0; i < 4; i++) {
        String ssid_key = "ssid" + String(i);
        String pass_key = "pass" + String(i);
        
        wifi_data_ssid[i] = prefs.getString(ssid_key.c_str(), "");
        wifi_data_pass[i] = prefs.getString(pass_key.c_str(), "");
    }
    
    prefs.end();
    
#ifdef DEBUG_MODE
    Serial.println("All WiFi data loaded");
#endif
}

static void save_all_wifi_data(void) {
    prefs.begin("wifi", false);
    
    for (int i = 0; i < 4; i++) {
        String ssid_key = "ssid" + String(i);
        String pass_key = "pass" + String(i);
        
        prefs.putString(ssid_key.c_str(), wifi_data_ssid[i].c_str());
        prefs.putString(pass_key.c_str(), wifi_data_pass[i].c_str());
    }
    
    prefs.end();
    
#ifdef DEBUG_MODE
    Serial.println("All WiFi data saved");
#endif
}

static void save_current_fields_to_memory(void) {
    if (ta_ssid && ta_password) {
        wifi_data_ssid[current_priority] = String(lv_textarea_get_text(ta_ssid));
        wifi_data_pass[current_priority] = String(lv_textarea_get_text(ta_password));
    }
}

static void load_current_priority_to_fields(void) {
    if (ta_ssid && ta_password) {
        lv_textarea_set_text(ta_ssid, wifi_data_ssid[current_priority].c_str());
        lv_textarea_set_text(ta_password, wifi_data_pass[current_priority].c_str());
    }
}

static void dropdown_event_cb(lv_event_t *e) {
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_VALUE_CHANGED) {
        // Sauvegarder les champs actuels
        save_current_fields_to_memory();
        
        // Changer la priorite
        current_priority = lv_dropdown_get_selected(dropdown_priority);
        
        // Charger les nouvelles donnees
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
    // Sauvegarder les champs actuels en memoire
    save_current_fields_to_memory();
    
    // Sauvegarder tout en Preferences
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
    
    // Charger toutes les donnees
    load_all_wifi_data();
    current_priority = 0;
    
    screen_settings_wifi = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(screen_settings_wifi, lv_color_hex(0x0a0e27), 0);
    lv_obj_set_style_bg_grad_color(screen_settings_wifi, lv_color_hex(0x1a1f3a), 0);
    lv_obj_set_style_bg_grad_dir(screen_settings_wifi, LV_GRAD_DIR_VER, 0);
    
    lv_obj_t *main_frame = lv_obj_create(screen_settings_wifi);
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
    
    // Titre
    lv_obj_t *label_title = lv_label_create(main_frame);
    lv_label_set_text(label_title, txt->wifi_settings);
    lv_obj_set_style_text_font(label_title, &lv_font_montserrat_32, 0);
    lv_obj_set_style_text_color(label_title, lv_color_hex(0x00d4ff), 0);
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
    lv_obj_t *priority_row = lv_obj_create(fields_container);
    lv_obj_set_size(priority_row, lv_pct(100), LV_SIZE_CONTENT);
    lv_obj_set_style_bg_opa(priority_row, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(priority_row, 0, 0);
    lv_obj_set_style_pad_all(priority_row, 0, 0);
    lv_obj_set_flex_flow(priority_row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(priority_row, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_column(priority_row, 20, 0);
    lv_obj_clear_flag(priority_row, LV_OBJ_FLAG_SCROLLABLE);
    
    lv_obj_t *priority_label = lv_label_create(priority_row);
    lv_label_set_text(priority_label, txt->wifi_priority);
    lv_obj_set_style_text_font(priority_label, &lv_font_montserrat_20, 0);
    lv_obj_set_style_text_color(priority_label, lv_color_hex(0xff9500), 0);
    lv_obj_set_width(priority_label, 140);
    
    dropdown_priority = lv_dropdown_create(priority_row);
    lv_dropdown_set_options(dropdown_priority, "Priorite 1\nPriorite 2\nPriorite 3\nPriorite 4");
    lv_obj_set_width(dropdown_priority, 760);
    lv_obj_set_style_bg_color(dropdown_priority, lv_color_hex(0x0f1520), 0);
    lv_obj_set_style_text_color(dropdown_priority, lv_color_white(), 0);
    lv_obj_set_style_text_font(dropdown_priority, &lv_font_montserrat_20, 0);
    lv_obj_add_event_cb(dropdown_priority, dropdown_event_cb, LV_EVENT_ALL, NULL);
    
    // Ligne SSID
    lv_obj_t *ssid_row = lv_obj_create(fields_container);
    lv_obj_set_size(ssid_row, lv_pct(100), LV_SIZE_CONTENT);
    lv_obj_set_style_bg_opa(ssid_row, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(ssid_row, 0, 0);
    lv_obj_set_style_pad_all(ssid_row, 0, 0);
    lv_obj_set_flex_flow(ssid_row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(ssid_row, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_column(ssid_row, 20, 0);
    lv_obj_clear_flag(ssid_row, LV_OBJ_FLAG_SCROLLABLE);
    
    lv_obj_t *ssid_label = lv_label_create(ssid_row);
    lv_label_set_text(ssid_label, txt->wifi_ssid);
    lv_obj_set_style_text_font(ssid_label, &lv_font_montserrat_20, 0);
    lv_obj_set_style_text_color(ssid_label, lv_color_hex(0x00d4ff), 0);
    lv_obj_set_width(ssid_label, 140);
    
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
    lv_obj_t *pass_row = lv_obj_create(fields_container);
    lv_obj_set_size(pass_row, lv_pct(100), LV_SIZE_CONTENT);
    lv_obj_set_style_bg_opa(pass_row, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(pass_row, 0, 0);
    lv_obj_set_style_pad_all(pass_row, 0, 0);
    lv_obj_set_flex_flow(pass_row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(pass_row, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_column(pass_row, 20, 0);
    lv_obj_clear_flag(pass_row, LV_OBJ_FLAG_SCROLLABLE);
    
    lv_obj_t *pass_label = lv_label_create(pass_row);
    lv_label_set_text(pass_label, txt->wifi_password);
    lv_obj_set_style_text_font(pass_label, &lv_font_montserrat_20, 0);
    lv_obj_set_style_text_color(pass_label, lv_color_hex(0x00d4ff), 0);
    lv_obj_set_width(pass_label, 140);
    
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
    
    // Charger priorite 1 par defaut
    load_current_priority_to_fields();
    
    // Clavier
    keyboard = lv_keyboard_create(main_frame);
    lv_obj_set_size(keyboard, 980, 240);
    lv_obj_align(keyboard, LV_ALIGN_BOTTOM_MID, 0, -70);
    lv_obj_set_style_bg_color(keyboard, lv_color_hex(0x1a2035), 0);
    lv_obj_set_style_bg_opa(keyboard, LV_OPA_90, 0);
    lv_obj_set_style_border_width(keyboard, 2, 0);
    lv_obj_set_style_border_color(keyboard, lv_color_hex(0x6080a0), 0);
    lv_obj_set_style_radius(keyboard, 10, 0);
    lv_keyboard_set_mode(keyboard, LV_KEYBOARD_MODE_TEXT_LOWER);
    lv_obj_add_flag(keyboard, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(keyboard, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_event_cb(keyboard, keyboard_wifi_event_cb, LV_EVENT_ALL, NULL);
    
    // Boutons Save/Cancel
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
    lv_obj_add_event_cb(btn_save, btn_save_wifi_cb, LV_EVENT_CLICKED, NULL);
    
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
    lv_obj_add_event_cb(btn_cancel, btn_cancel_wifi_cb, LV_EVENT_CLICKED, NULL);
    
    lv_obj_t *label_cancel = lv_label_create(btn_cancel);
    lv_label_set_text(label_cancel, txt->cancel);
    lv_obj_set_style_text_font(label_cancel, &lv_font_montserrat_24, 0);
    lv_obj_set_style_text_color(label_cancel, lv_color_white(), 0);
    lv_obj_center(label_cancel);
    
#ifdef DEBUG_MODE
    Serial.println("WiFi settings screen initialized");
#endif
}

void ui_settings_wifi_show(void) {
    if (screen_settings_wifi == NULL) {
        ui_settings_wifi_init();
    }
    lv_screen_load(screen_settings_wifi);
    
#ifdef DEBUG_MODE
    Serial.println("WiFi settings screen shown");
#endif
}

#endif