#ifndef UI_FILE_TRANSFER_H
#define UI_FILE_TRANSFER_H

#include "lvgl.h"
#include "constants.h"
#include "esp_system.h"

static lv_obj_t *screen_file_transfer = NULL;

// Callback pour le bouton sortie
static void btn_exit_cb(lv_event_t *e) {
#ifdef DEBUG_MODE
  Serial.println("Exit button clicked - Rebooting...");
#endif
  vTaskDelay(pdMS_TO_TICKS(500));
  esp_restart();
}

void ui_file_transfer_init(void) {
  const TextStrings *txt = get_text();

  // Create screen avec gradient background
  screen_file_transfer = lv_obj_create(NULL);
  lv_obj_set_style_bg_color(screen_file_transfer, lv_color_hex(0x0a0e27), 0);
  lv_obj_set_style_bg_grad_color(screen_file_transfer, lv_color_hex(0x1a1f3a), 0);
  lv_obj_set_style_bg_grad_dir(screen_file_transfer, LV_GRAD_DIR_VER, 0);

  // Main container
  lv_obj_t *main_frame = lv_obj_create(screen_file_transfer);
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

  // Supprimer le scrolling qui peut causer des decalages
  lv_obj_clear_flag(main_frame, LV_OBJ_FLAG_SCROLLABLE);

  // Title
  lv_obj_t *label_title = lv_label_create(main_frame);
  lv_label_set_text(label_title, txt->file_transfer);
  lv_obj_set_style_text_font(label_title, &lv_font_montserrat_48, 0);
  lv_obj_set_style_text_color(label_title, lv_color_hex(0x00d4ff), 0);
  lv_obj_align(label_title, LV_ALIGN_TOP_MID, 0, 5);

  // Info panel - remonter de 10px (20 -> 10)
  lv_obj_t *info_panel = lv_obj_create(main_frame);
  lv_obj_set_size(info_panel, 900, 400);
  lv_obj_align(info_panel, LV_ALIGN_CENTER, 0, 10);  // Change de 20 a 10
  lv_obj_set_style_bg_color(info_panel, lv_color_hex(0x1a2035), 0);
  lv_obj_set_style_bg_opa(info_panel, LV_OPA_80, 0);
  lv_obj_set_style_border_width(info_panel, 0, 0);
  lv_obj_set_style_radius(info_panel, 15, 0);
  lv_obj_set_style_pad_all(info_panel, 30, 0);
  lv_obj_set_flex_flow(info_panel, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_flex_align(info_panel, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
  lv_obj_set_style_pad_row(info_panel, 20, 0);

  // Desactiver le scroll sur l'info panel
  lv_obj_clear_flag(info_panel, LV_OBJ_FLAG_SCROLLABLE);

  // WiFi status icon
  lv_obj_t *wifi_icon = lv_label_create(info_panel);
  lv_label_set_text(wifi_icon, LV_SYMBOL_WIFI);
  lv_obj_set_style_text_font(wifi_icon, &lv_font_montserrat_48, 0);
  lv_obj_set_style_text_color(wifi_icon, lv_color_hex(0xff9500), 0);

  // Status label
  lv_obj_t *status_label = lv_label_create(info_panel);
  lv_label_set_text(status_label, "Demarrage du WiFi...");
  lv_obj_set_style_text_font(status_label, &lv_font_montserrat_24, 0);
  lv_obj_set_style_text_color(status_label, lv_color_hex(0xaabbcc), 0);
  lv_obj_set_style_text_align(status_label, LV_TEXT_ALIGN_CENTER, 0);

  // SSID
  lv_obj_t *ssid_label = lv_label_create(info_panel);
  lv_label_set_text(ssid_label, "SSID: Bip-Bip-Hourra");
  lv_obj_set_style_text_font(ssid_label, &lv_font_montserrat_20, 0);
  lv_obj_set_style_text_color(ssid_label, lv_color_hex(0x00d4ff), 0);

  // IP Address
  lv_obj_t *ip_label = lv_label_create(info_panel);
  lv_label_set_text(ip_label, "IP: 192.168.4.1");
  lv_obj_set_style_text_font(ip_label, &lv_font_montserrat_20, 0);
  lv_obj_set_style_text_color(ip_label, lv_color_hex(0x00d4ff), 0);

  // Separator
  lv_obj_t *separator = lv_obj_create(info_panel);
  lv_obj_set_size(separator, lv_pct(80), 2);
  lv_obj_set_style_bg_color(separator, lv_color_hex(0x2a3f5f), 0);
  lv_obj_set_style_border_width(separator, 0, 0);

  // Instructions
  lv_obj_t *instructions = lv_label_create(info_panel);
  lv_label_set_text(instructions,
                    "1. Connectez-vous au WiFi\n"
                    "2. Ouvrez votre navigateur\n"
                    "3. Allez sur: http://192.168.4.1");
  lv_obj_set_style_text_font(instructions, &lv_font_montserrat_20, 0);
  lv_obj_set_style_text_color(instructions, lv_color_hex(0xaabbcc), 0);
  lv_obj_set_style_text_align(instructions, LV_TEXT_ALIGN_CENTER, 0);
  lv_label_set_long_mode(instructions, LV_LABEL_LONG_WRAP);
  lv_obj_set_width(instructions, lv_pct(90));

  // Exit button
  lv_obj_t *btn_exit = lv_button_create(main_frame);
  lv_obj_set_size(btn_exit, 300, 80);
  lv_obj_align(btn_exit, LV_ALIGN_BOTTOM_MID, 0, -20);
  lv_obj_set_style_bg_color(btn_exit, lv_color_hex(0xff3b30), 0);
  lv_obj_set_style_radius(btn_exit, 15, 0);
  lv_obj_set_style_shadow_width(btn_exit, 5, 0);
  lv_obj_set_style_shadow_color(btn_exit, lv_color_black(), 0);
  lv_obj_set_style_shadow_opa(btn_exit, LV_OPA_20, 0);
  lv_obj_set_style_bg_color(btn_exit, lv_color_darken(lv_color_hex(0xff3b30), 20), LV_STATE_PRESSED);
  lv_obj_add_event_cb(btn_exit, btn_exit_cb, LV_EVENT_CLICKED, NULL);

  // Exit button icon
  lv_obj_t *exit_icon = lv_label_create(btn_exit);
  lv_label_set_text(exit_icon, LV_SYMBOL_POWER);
  lv_obj_set_style_text_font(exit_icon, &lv_font_montserrat_32, 0);
  lv_obj_set_style_text_color(exit_icon, lv_color_white(), 0);
  lv_obj_align(exit_icon, LV_ALIGN_LEFT_MID, 30, 0);

  // Exit button label
  lv_obj_t *exit_label = lv_label_create(btn_exit);
  lv_label_set_text(exit_label, txt->exit);
  lv_obj_set_style_text_font(exit_label, &lv_font_montserrat_24, 0);
  lv_obj_set_style_text_color(exit_label, lv_color_white(), 0);
  lv_obj_align(exit_label, LV_ALIGN_CENTER, 15, 0);

#ifdef DEBUG_MODE
  Serial.println("File transfer screen initialized");
#endif
}

void ui_file_transfer_show(void) {
  if (screen_file_transfer == NULL) {
    ui_file_transfer_init();
  }
  lv_screen_load(screen_file_transfer);

#ifdef DEBUG_MODE
  Serial.println("File transfer screen shown");
#endif
}

#endif