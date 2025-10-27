#ifndef UI_FILE_TRANSFER_H
#define UI_FILE_TRANSFER_H

#include "lvgl.h"
#include "constants.h"
#include "esp_system.h"
#include "UI_helper.h"
#include "lang.h"
#include "globals.h"
#include "src/wifi_task.h"
#include "src/file_server_task.h"


#ifdef TEST_MODE
#include "src/test_logger_task.h"  // AJOUTE CETTE LIGNE
#endif

static lv_obj_t *label_status = NULL;
static lv_obj_t *label_ssid = NULL;
static lv_obj_t *label_ip = NULL;
static lv_timer_t *status_timer = NULL;

// Timer pour mettre a jour le statut WiFi
static void status_update_timer_cb(lv_timer_t *timer) {
  if (!label_status || !label_ssid || !label_ip) {
    return;
  }

  if (wifi_get_connected_status()) {
    lv_label_set_text(label_status, LV_SYMBOL_WIFI " Connected");
    lv_obj_set_style_text_color(label_status, lv_color_hex(0x34c759), 0);

    // Construire le texte SSID avec buffer
    char ssid_buffer[64];
    snprintf(ssid_buffer, sizeof(ssid_buffer), "SSID: %s", wifi_get_current_ssid());
    lv_label_set_text(label_ssid, ssid_buffer);

    // Construire le texte IP avec buffer
    char ip_buffer[32];
    snprintf(ip_buffer, sizeof(ip_buffer), "IP: %s", wifi_get_current_ip());
    lv_label_set_text(label_ip, ip_buffer);
  } else {
    lv_label_set_text(label_status, LV_SYMBOL_WARNING " Connecting...");
    lv_obj_set_style_text_color(label_status, lv_color_hex(0xff9500), 0);
    lv_label_set_text(label_ssid, "Trying WiFi networks...");
    lv_label_set_text(label_ip, "");
  }
}

// Callback pour le bouton sortie
static void btn_exit_cb(lv_event_t *e) {
#ifdef DEBUG_MODE
  Serial.println("Exit file transfer - stopping services...");
#endif

  // Arreter le timer
  if (status_timer) {
    lv_timer_del(status_timer);
    status_timer = NULL;
  }

  // Arreter les services
  file_server_stop();
  wifi_task_stop();

  vTaskDelay(pdMS_TO_TICKS(500));
  esp_restart();
}

void ui_file_transfer_init(void) {
  const TextStrings *txt = get_text();

#ifdef TEST_MODE
  // CRITIQUE: Arrêter le logger avant de démarrer le serveur
  test_logger_stop();
  vTaskDelay(pdMS_TO_TICKS(500));  // Laisser le temps de fermer proprement

#ifdef DEBUG_MODE
  Serial.println("[FILE_TRANSFER] Test logger stopped");
#endif
#endif

  lv_obj_t *main_frame = ui_create_black_screen_with_frame(3, ROUND_FRANE_RADUIS_BIG, &main_screen);

  // Titre
  lv_obj_t *label_title = ui_create_label(main_frame, txt->file_transfer,
                                          &lv_font_montserrat_32, lv_color_hex(0x00d4ff));
  lv_obj_align(label_title, LV_ALIGN_TOP_MID, 0, 0);

  // Info panel
  lv_obj_t *info_panel = lv_obj_create(main_frame);
  lv_obj_set_size(info_panel, 900, 400);
  lv_obj_align(info_panel, LV_ALIGN_CENTER, 0, 10);
  lv_obj_set_style_bg_color(info_panel, lv_color_hex(0x1a2035), 0);
  lv_obj_set_style_bg_opa(info_panel, LV_OPA_80, 0);
  lv_obj_set_style_border_width(info_panel, 2, 0);
  lv_obj_set_style_border_color(info_panel, lv_color_hex(0x6080a0), 0);
  lv_obj_set_style_radius(info_panel, ROUND_FRANE_RADUIS_BIG, 0);
  lv_obj_set_style_pad_all(info_panel, 20, 0);
  lv_obj_set_flex_flow(info_panel, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_flex_align(info_panel, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER,
                        LV_FLEX_ALIGN_CENTER);
  lv_obj_set_style_pad_row(info_panel, 20, 0);
  lv_obj_clear_flag(info_panel, LV_OBJ_FLAG_SCROLLABLE);

  // WiFi status
  label_status = ui_create_label(info_panel, LV_SYMBOL_WIFI " Starting...",
                                 &lv_font_montserrat_32, lv_color_hex(0xff9500));

  // SSID
  label_ssid = ui_create_label(info_panel, "Initializing WiFi...",
                               &lv_font_montserrat_24, lv_color_hex(0xaabbcc));
  lv_obj_set_style_text_align(label_ssid, LV_TEXT_ALIGN_CENTER, 0);

  // IP
  label_ip = ui_create_label(info_panel, "",
                             &lv_font_montserrat_24, lv_color_hex(0xaabbcc));
  lv_obj_set_style_text_align(label_ip, LV_TEXT_ALIGN_CENTER, 0);

  // Separator
  lv_obj_t *sep = lv_obj_create(info_panel);
  lv_obj_set_size(sep, lv_pct(100), 1);
  lv_obj_set_style_bg_color(sep, lv_color_hex(0x2a3f5f), 0);
  lv_obj_set_style_border_width(sep, 0, 0);

  // Instructions
  lv_obj_t *instructions = ui_create_label(info_panel,
                                           "1. Wait for WiFi connection\n"
                                           "2. Open your web browser\n"
                                           "3. Go to the IP address shown above",
                                           &lv_font_montserrat_20,
                                           lv_color_hex(0xaabbcc));
  lv_obj_set_style_text_align(instructions, LV_TEXT_ALIGN_CENTER, 0);
  lv_label_set_long_mode(instructions, LV_LABEL_LONG_WRAP);
  lv_obj_set_width(instructions, lv_pct(90));

  // Bouton exit
  ui_button_pair_t buttons = ui_create_save_cancel_buttons(
    main_frame, nullptr, txt->exit, nullptr,
    false, true, false,
    nullptr, btn_exit_cb, nullptr,
    NULL, NULL, NULL);
  if (lvgl_port_lock(-1)) {
    lv_screen_load(main_screen);
    lvgl_port_unlock();
  }

  // Demarrer WiFi et serveur de fichiers
  wifi_task_start();

  // Attendre un peu avant de lancer le serveur
  vTaskDelay(pdMS_TO_TICKS(2000));
  file_server_start();

  // Creer le timer pour mettre a jour le statut
  status_timer = lv_timer_create(status_update_timer_cb, 1000, NULL);

#ifdef DEBUG_MODE
  Serial.println("File transfer screen initialized with WiFi server");
#endif
}

void ui_file_transfer_show(void) {
  lv_obj_t *old_screen = lv_scr_act();

  ui_file_transfer_init();

  if (old_screen != main_screen && old_screen != NULL) {
    lv_obj_del(old_screen);
#ifdef DEBUG_MODE
    Serial.println("[PRESTART] Old screen deleted");
#endif
  }
#ifdef DEBUG_MODE
  Serial.println("File transfer screen shown");
#endif
}

#endif