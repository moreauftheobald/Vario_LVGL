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

// Backup pour Cancel - allouees en PSRAM
static char *wifi_backup_ssid[4] = { NULL, NULL, NULL, NULL };
static char *wifi_backup_pass[4] = { NULL, NULL, NULL, NULL };

// Sauvegarder params actuels pour Cancel
static void backup_wifi_data(void) {
  for (int i = 0; i < 4; i++) {
    // Liberer ancien backup
    if (wifi_backup_ssid[i]) {
      heap_caps_free(wifi_backup_ssid[i]);
      wifi_backup_ssid[i] = NULL;
    }
    if (wifi_backup_pass[i]) {
      heap_caps_free(wifi_backup_pass[i]);
      wifi_backup_pass[i] = NULL;
    }

    // Copier params actuels
    wifi_backup_ssid[i] = psram_strdup(psram_str_get(params.wifi_ssid[i]));
    wifi_backup_pass[i] = psram_strdup(psram_str_get(params.wifi_password[i]));
  }

#ifdef DEBUG_MODE
  Serial.println("WiFi data backed up");
#endif
}

// Restaurer params depuis backup (pour Cancel)
static void restore_wifi_data(void) {
  for (int i = 0; i < 4; i++) {
    psram_str_set(&params.wifi_ssid[i], wifi_backup_ssid[i]);
    psram_str_set(&params.wifi_password[i], wifi_backup_pass[i]);
  }

#ifdef DEBUG_MODE
  Serial.println("WiFi data restored from backup");
#endif
}

// Liberer backup
static void free_wifi_backup(void) {
  for (int i = 0; i < 4; i++) {
    if (wifi_backup_ssid[i]) {
      heap_caps_free(wifi_backup_ssid[i]);
      wifi_backup_ssid[i] = NULL;
    }
    if (wifi_backup_pass[i]) {
      heap_caps_free(wifi_backup_pass[i]);
      wifi_backup_pass[i] = NULL;
    }
  }
}

// Sauvegarder champs actuels dans params
static void save_current_fields_to_params(void) {
  if (ta_ssid && ta_password) {
    const char *ssid = lv_textarea_get_text(ta_ssid);
    const char *pass = lv_textarea_get_text(ta_password);

    // Supprimer espaces et sauvegarder directement dans params
    String ssid_str = String(ssid);
    ssid_str.trim();
    psram_str_set(&params.wifi_ssid[current_priority], ssid_str.c_str());

    String pass_str = String(pass);
    pass_str.trim();
    psram_str_set(&params.wifi_password[current_priority], pass_str.c_str());

#ifdef DEBUG_MODE
    Serial.printf("Priority %d saved: SSID='%s' (len=%d), Pass len=%d\n",
                  current_priority + 1,
                  psram_str_get(params.wifi_ssid[current_priority]),
                  strlen(psram_str_get(params.wifi_ssid[current_priority])),
                  strlen(psram_str_get(params.wifi_password[current_priority])));
#endif
  }
}

// Charger params dans champs
static void load_current_priority_to_fields(void) {
  if (ta_ssid && ta_password) {
    lv_textarea_set_text(ta_ssid, psram_str_get(params.wifi_ssid[current_priority]));
    lv_textarea_set_text(ta_password, psram_str_get(params.wifi_password[current_priority]));
  }
}

// Callbacks
static void dropdown_wifi_event_cb(lv_event_t *e) {
  lv_event_code_t code = lv_event_get_code(e);
  if (code == LV_EVENT_VALUE_CHANGED) {
    save_current_fields_to_params();
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

  // Sauvegarder champs courants
  save_current_fields_to_params();

  // Ecrire params en NVS
  params_save_wifi();

  // Liberer backup
  free_wifi_backup();

#ifdef DEBUG_MODE
  Serial.println("WiFi settings saved to NVS");
#endif

  ui_settings_show();
}

static void btn_cancel_wifi_cb(lv_event_t *e) {
#ifdef DEBUG_MODE
  Serial.println("Cancel WiFi settings clicked");
#endif

  // Restaurer params depuis backup
  restore_wifi_data();

  // Liberer backup
  free_wifi_backup();

  ui_settings_show();
}

void ui_settings_wifi_init(void) {
  const TextStrings *txt = get_text();

  // Backup params au debut
  backup_wifi_data();
  current_priority = 0;

  lv_obj_t *main_frame = ui_create_black_screen_with_frame(3, ROUND_FRANE_RADUIS_BIG, &current_screen);
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

#ifdef DEBUG_MODE
  Serial.println("WiFi settings screen initialized");
#endif
}

void ui_settings_wifi_show(void) {
  // Sauvegarder ancien écran
  lv_obj_t *old_screen = lv_scr_act();

  if (lvgl_port_lock(-1)) {
    // Créer nouvel écran
    current_screen = lv_obj_create(NULL);

    ui_settings_wifi_init();
    lv_screen_load(current_screen);
    force_full_refresh();
    lvgl_port_unlock();
  }

  // Détruire ancien écran
  if (old_screen != current_screen && old_screen != NULL) {
    lv_obj_del(old_screen);
#ifdef DEBUG_MODE
    Serial.println("[UI] Old screen deleted");
#endif
  }
}

#endif