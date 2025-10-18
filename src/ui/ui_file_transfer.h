#ifndef UI_FILE_TRANSFER_H
#define UI_FILE_TRANSFER_H

#include "lvgl.h"
#include "constants.h"
#include "esp_system.h"
#include "UI_helper.h"
#include "lang.h"
#include "globals.h"

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

  lv_obj_t *main_frame = ui_create_black_screen_with_frame(3, 20, &main_screen);

  // Titre
  ui_create_title(main_frame, txt->file_transfer);

  // Info panel
  lv_obj_t *info_panel = ui_create_info_panel(main_frame, 900, 400);
  lv_obj_align(info_panel, LV_ALIGN_CENTER, 0, 10);
  lv_obj_set_style_pad_row(info_panel, 20, 0);
  lv_obj_set_flex_align(info_panel, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

  // WiFi status icon
  lv_obj_t *wifi_icon = ui_create_label(info_panel, LV_SYMBOL_WIFI,
                                        &lv_font_montserrat_48, lv_color_hex(0x00d4ff));

  // SSID text
  lv_obj_t *ssid_label = ui_create_label(info_panel, "SSID: Vario_AP",
                                         &lv_font_montserrat_24, lv_color_hex(0xaabbcc));
  lv_obj_set_style_text_align(ssid_label, LV_TEXT_ALIGN_CENTER, 0);

  // Password text
  lv_obj_t *pwd_label = ui_create_label(info_panel, "Password: vario1234",
                                        &lv_font_montserrat_24, lv_color_hex(0xaabbcc));
  lv_obj_set_style_text_align(pwd_label, LV_TEXT_ALIGN_CENTER, 0);

  // Separator
  ui_create_separator(info_panel);

  // Instructions
  lv_obj_t *instructions = ui_create_label(info_panel,
                                           "1. Connectez-vous au WiFi\n"
                                           "2. Ouvrez votre navigateur\n"
                                           "3. Allez sur: http://192.168.4.1",
                                           &lv_font_montserrat_20, lv_color_hex(0xaabbcc));
  lv_obj_set_style_text_align(instructions, LV_TEXT_ALIGN_CENTER, 0);
  lv_label_set_long_mode(instructions, LV_LABEL_LONG_WRAP);
  lv_obj_set_width(instructions, lv_pct(90));

  ui_button_pair_t buttons = ui_create_save_cancel_buttons(main_frame, nullptr, txt->exit, nullptr, false, true, false);
  lv_obj_add_event_cb(buttons.cancel, btn_exit_cb, LV_EVENT_CLICKED, NULL);

  lv_screen_load(main_screen);

#ifdef DEBUG_MODE
  Serial.println("File transfer screen initialized");
#endif
}

void ui_file_transfer_show(void) {
  ui_file_transfer_init();

#ifdef DEBUG_MODE
  Serial.println("File transfer screen shown");
#endif
}

#endif