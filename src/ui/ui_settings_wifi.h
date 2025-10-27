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

// Donnees temporaires pour les 4 priorites - allouees en PSRAM
static char *wifi_data_ssid[4] = { NULL, NULL, NULL, NULL };
static char *wifi_data_pass[4] = { NULL, NULL, NULL, NULL };

static void load_all_wifi_data(void) {
  for (int i = 0; i < 4; i++) {
    // Liberer anciennes allocations
    if (wifi_data_ssid[i]) {
      heap_caps_free(wifi_data_ssid[i]);
      wifi_data_ssid[i] = NULL;
    }
    if (wifi_data_pass[i]) {
      heap_caps_free(wifi_data_pass[i]);
      wifi_data_pass[i] = NULL;
    }

    // Copier depuis params vers donnees temporaires
    wifi_data_ssid[i] = psram_strdup(psram_str_get(params.wifi_ssid[i]));
    wifi_data_pass[i] = psram_strdup(psram_str_get(params.wifi_password[i]));
  }

#ifdef DEBUG_MODE
  Serial.println("All WiFi data loaded from params");
#endif
}

static void save_all_wifi_data(void) {
  for (int i = 0; i < 4; i++) {
    // Sauvegarder depuis donnees temporaires vers params
    psram_str_set(&params.wifi_ssid[i], wifi_data_ssid[i]);
    psram_str_set(&params.wifi_password[i], wifi_data_pass[i]);
  }

  params_save_wifi();

#ifdef DEBUG_MODE
  Serial.println("All WiFi data saved to params");
#endif
}

static void save_current_fields_to_memory(void) {
  if (ta_ssid && ta_password) {
    const char *ssid = lv_textarea_get_text(ta_ssid);
    const char *pass = lv_textarea_get_text(ta_password);

    // Liberer anciennes allocations
    if (wifi_data_ssid[current_priority]) {
      heap_caps_free(wifi_data_ssid[current_priority]);
    }
    if (wifi_data_pass[current_priority]) {
      heap_caps_free(wifi_data_pass[current_priority]);
    }

    // Allouer et copier (en supprimant les espaces)
    String ssid_str = String(ssid);
    ssid_str.trim();
    wifi_data_ssid[current_priority] = psram_strdup(ssid_str.c_str());

    String pass_str = String(pass);
    pass_str.trim();
    wifi_data_pass[current_priority] = psram_strdup(pass_str.c_str());

#ifdef DEBUG_MODE
    Serial.printf("Saved SSID: '%s' (len=%d)\n",
                  wifi_data_ssid[current_priority],
                  strlen(wifi_data_ssid[current_priority]));
    Serial.printf("Saved Pass len: %d\n", strlen(wifi_data_pass[current_priority]));
#endif
  }
}

static void load_current_priority_to_fields(void) {
  if (ta_ssid && ta_password) {
    lv_textarea_set_text(ta_ssid,
                         wifi_data_ssid[current_priority] ? wifi_data_ssid[current_priority] : "");
    lv_textarea_set_text(ta_password,
                         wifi_data_pass[current_priority] ? wifi_data_pass[current_priority] : "");
  }
}

// Callbacks
static void dropdown_wifi_event_cb(lv_event_t *e) {
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
  lv_obj_t *ta = (lv_obj_t *)lv_event_get_target(e);

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

  // Verifier avant sauvegarde
#ifdef DEBUG_MODE
  for (int i = 0; i < 4; i++) {
    Serial.printf("Priority %d: SSID='%s' (len=%d), Pass len=%d\n",
                  i + 1,
                  wifi_data_ssid[i] ? wifi_data_ssid[i] : "",
                  wifi_data_ssid[i] ? strlen(wifi_data_ssid[i]) : 0,
                  wifi_data_pass[i] ? strlen(wifi_data_pass[i]) : 0);
  }
#endif

  save_all_wifi_data();
  ui_settings_show();
}

static void btn_cancel_wifi_cb(lv_event_t *e) {
#ifdef DEBUG_MODE
  Serial.println("Cancel WiFi settings clicked");
#endif

  // Liberer les donnees temporaires
  for (int i = 0; i < 4; i++) {
    if (wifi_data_ssid[i]) {
      heap_caps_free(wifi_data_ssid[i]);
      wifi_data_ssid[i] = NULL;
    }
    if (wifi_data_pass[i]) {
      heap_caps_free(wifi_data_pass[i]);
      wifi_data_pass[i] = NULL;
    }
  }

  ui_settings_show();
}

void ui_settings_wifi_init(void) {
  const TextStrings *txt = get_text();

  load_all_wifi_data();
  current_priority = 0;

  lv_obj_t *main_frame = ui_create_black_screen_with_frame(3, ROUND_FRANE_RADUIS_BIG, &main_screen);
  ui_create_main_frame(main_frame, false, txt->wifi_settings);

  // Ligne Priorite
  lv_obj_t *priority_row = ui_create_form_row(main_left, txt->wifi_priority, 140, lv_color_hex(0xff9500));

  dropdown_priority = lv_dropdown_create(priority_row);
  lv_dropdown_set_options(dropdown_priority, "Priorite 1\nPriorite 2\nPriorite 3\nPriorite 4");
  lv_obj_set_width(dropdown_priority, 760);
  lv_obj_set_style_bg_color(dropdown_priority, lv_color_hex(0x0f1520), 0);
  lv_obj_set_style_text_color(dropdown_priority, lv_color_white(), 0);
  lv_obj_set_style_text_font(dropdown_priority, &lv_font_montserrat_20, 0);
  lv_obj_add_event_cb(dropdown_priority, dropdown_wifi_event_cb, LV_EVENT_ALL, NULL);

  // Ligne SSID
  lv_obj_t *ssid_row = ui_create_form_row(main_left, txt->wifi_ssid, 140, lv_color_hex(0x00d4ff));
  ta_ssid = ui_create_textarea(ssid_row, TXT_AREA_W, TXT_AREA_H, 32, true);
  lv_obj_add_event_cb(ta_ssid, ta_wifi_event_cb, LV_EVENT_FOCUSED, NULL);

  // Ligne Password
  lv_obj_t *password_row = ui_create_form_row(main_left, txt->wifi_password, 140, lv_color_hex(0x00d4ff));
  ta_password = ui_create_textarea(password_row, TXT_AREA_W, TXT_AREA_H, 32, true);
  lv_obj_add_event_cb(ta_password, ta_wifi_event_cb, LV_EVENT_FOCUSED, NULL);

  // Clavier
  if (!keyboard) {
    keyboard = ui_create_keyboard(main_frame);
    lv_obj_add_event_cb(keyboard, keyboard_wifi_event_cb, LV_EVENT_ALL, NULL);
  }

  // Bouton Save
  lv_obj_t *btn_save_wifi = ui_create_button(btn_container, txt->save, LV_SYMBOL_SAVE, lv_color_hex(START_BTN_COLOR),
                                             PRE_BTN_W, PRE_BTN_H, INFO_FONT_S, INFO_FONT_BIG, btn_save_wifi_cb,
                                             NULL, (lv_align_t)0, NULL, NULL);

  // Bouton Cancel
  lv_obj_t *btn_cancel_wifi = ui_create_button(btn_container, txt->cancel, LV_SYMBOL_BACKSPACE, lv_color_hex(CANCE_BTN_COLOR),
                                               PRE_BTN_W, PRE_BTN_H, INFO_FONT_S, INFO_FONT_BIG, btn_cancel_wifi_cb,
                                               NULL, (lv_align_t)0, NULL, NULL);

  // Charger la priorite 0
  load_current_priority_to_fields();

  if (lvgl_port_lock(-1)) {
    lv_screen_load(main_screen);
    lvgl_port_unlock();
  }

#ifdef DEBUG_MODE
  Serial.println("WiFi settings screen initialized");
#endif
}

void ui_settings_wifi_show(void) {
  // Sauvegarder ancien écran
  lv_obj_t *old_screen = lv_scr_act();

  ui_settings_wifi_init();

  // Détruire l'ancien écran SI ce n'est pas le même
  if (old_screen != main_screen && old_screen != NULL) {
    lv_obj_del(old_screen);
#ifdef DEBUG_MODE
    Serial.println("[PRESTART] Old screen deleted");
#endif
  }
}

#endif