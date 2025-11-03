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
    ui_label_set_formatted_text(label_ssid, "SSID: %s", wifi_get_current_ssid());
    ui_label_set_formatted_text(label_ip, "IP: %s", wifi_get_current_ip());
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

  lv_obj_t *main_frame = ui_create_black_screen_with_frame(3, ROUND_FRANE_RADUIS_BIG, &current_screen);
  ui_create_main_frame(main_frame, false, txt->file_transfer);

  lv_obj_align(main_left, LV_ALIGN_CENTER, 0, 10);


  // WiFi status
  label_status = ui_create_label(main_left, LV_SYMBOL_WIFI " Starting...",
                                 &lv_font_montserrat_32, lv_color_hex(0xff9500));
  lv_obj_set_style_text_align(label_status, LV_TEXT_ALIGN_CENTER, 0);
  lv_obj_set_width(label_status, lv_pct(100));

  // SSID
  label_ssid = ui_create_label(main_left, "Initializing WiFi...",
                               &lv_font_montserrat_24, lv_color_hex(0xaabbcc));
  lv_obj_set_style_text_align(label_ssid, LV_TEXT_ALIGN_CENTER, 0);
  lv_obj_set_width(label_ssid, lv_pct(100));

  // IP
  label_ip = ui_create_label(main_left, "",
                             &lv_font_montserrat_24, lv_color_hex(0xaabbcc));
  lv_obj_set_style_text_align(label_ip, LV_TEXT_ALIGN_CENTER, 0);
  lv_obj_set_width(label_ip, lv_pct(100));

  // Separator
  lv_obj_t *sep = ui_create_h_separator(main_left, lv_color_hex(0x2a3f5f));

  // Instructions
  lv_obj_t *instructions = ui_create_label(main_left,
                                           "1. Wait for WiFi connection\n"
                                           "2. Open your web browser\n"
                                           "3. Go to the IP address shown above",
                                           &lv_font_montserrat_20,
                                           lv_color_hex(0xaabbcc));
  lv_obj_set_style_text_align(instructions, LV_TEXT_ALIGN_CENTER, 0);
  lv_label_set_long_mode(instructions, LV_LABEL_LONG_WRAP);
  lv_obj_set_width(instructions, lv_pct(100));

  // Bouton exit
  lv_obj_t *btn_cancel_pilot = ui_create_button(btn_container, txt->exit, LV_SYMBOL_BACKSPACE, lv_color_hex(CANCE_BTN_COLOR),
                                                PRE_BTN_W, PRE_BTN_H, INFO_FONT_S, INFO_FONT_BIG, btn_exit_cb,
                                                NULL, (lv_align_t)0, NULL, NULL);

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
  ui_switch_screen(ui_file_transfer_init);
}

#endif