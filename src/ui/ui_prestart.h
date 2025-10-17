#ifndef UI_PRESTART_H
#define UI_PRESTART_H

#include "lvgl.h"
#include "constants.h"
#include "UI_helper.h"
#include "lang.h"
#include "globals.h"
#include "src/ui/ui_main_screens.h"

// Forward declarations
void ui_file_transfer_show(void);
void ui_settings_show(void);

// Callbacks
static void btn_file_transfer_cb(lv_event_t *e) {
#ifdef DEBUG_MODE
  Serial.println("File transfer button clicked");
#endif
  ui_file_transfer_show();
}

static void btn_settings_cb(lv_event_t *e) {
#ifdef DEBUG_MODE
  Serial.println("Settings button clicked");
#endif
  ui_settings_show();
}

static void btn_start_cb(lv_event_t *e) {
#ifdef DEBUG_MODE
  Serial.println("Start button clicked");
#endif
  ui_main_screens_show();
}

/**
 * @brief Initialize prestart screen
 */
void ui_prestart_init(void) {
  const TextStrings *txt = get_text();

  lv_obj_t *main_frame = ui_create_black_screen_with_frame(3, 20, &main_screen);

  // Titre
  lv_obj_t *label_title = lv_label_create(main_frame);
  lv_label_set_text(label_title, VARIO_NAME);
  lv_obj_set_style_text_font(label_title, &lv_font_montserrat_48, 0);
  lv_obj_set_style_text_color(label_title, lv_color_hex(0x00d4ff), 0);
  lv_obj_set_style_text_align(label_title, LV_TEXT_ALIGN_CENTER, 0);
  lv_obj_align(label_title, LV_ALIGN_TOP_MID, 0, 5);
  lv_obj_set_style_bg_opa(label_title, LV_OPA_TRANSP, 0);
  lv_obj_set_style_pad_all(label_title, 0, 0);

  // Conteneur principal 2 colonnes
  lv_obj_t *content_container = ui_create_flex_container(main_frame, LV_FLEX_FLOW_ROW);
  lv_obj_set_size(content_container, lv_pct(100), LV_SIZE_CONTENT);
  lv_obj_align(content_container, LV_ALIGN_CENTER, 0, 40);
  lv_obj_set_flex_align(content_container, LV_FLEX_ALIGN_SPACE_EVENLY, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
  lv_obj_set_style_pad_column(content_container, 30, 0);

  // Colonne gauche: Boutons
  lv_obj_t *btn_container = ui_create_flex_container(content_container, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_flex_align(btn_container, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
  lv_obj_set_style_pad_row(btn_container, 30, 0);

  // Bouton 1: File Transfer (Orange)
  lv_obj_t *btn_file_transfer = ui_create_modern_button(btn_container, txt->file_transfer,
                                                        LV_SYMBOL_USB, lv_color_hex(0xff9500));
  lv_obj_add_event_cb(btn_file_transfer, btn_file_transfer_cb, LV_EVENT_CLICKED, NULL);

  // Bouton 2: Settings (Bleu)
  lv_obj_t *btn_settings = ui_create_modern_button(btn_container, txt->settings,
                                                   LV_SYMBOL_SETTINGS, lv_color_hex(0x007aff));
  lv_obj_add_event_cb(btn_settings, btn_settings_cb, LV_EVENT_CLICKED, NULL);

  // Bouton 3: Start (Vert)
  lv_obj_t *btn_start = ui_create_modern_button(btn_container, txt->start,
                                                LV_SYMBOL_PLAY, lv_color_hex(0x34c759));
  lv_obj_add_event_cb(btn_start, btn_start_cb, LV_EVENT_CLICKED, NULL);

  // Colonne droite: Panel d'informations
  lv_obj_t *info_panel = ui_create_info_panel_bordered(content_container, 550, 450);
  lv_obj_set_style_pad_row(info_panel, 12, 0);

  // Titre du panel
  lv_obj_t *info_title = ui_create_label(info_panel, "Informations",
                                         &lv_font_montserrat_24, lv_color_hex(0x00d4ff));
  lv_obj_set_width(info_title, lv_pct(100));

  // Separateur
  ui_create_separator(info_panel);

  // Version
  lv_obj_t *label_version = ui_create_label(info_panel,
                                            LV_SYMBOL_SETTINGS " Version: " VARIO_VERSION,
                                            &lv_font_montserrat_20, lv_color_hex(0xaabbcc));
  lv_obj_set_width(label_version, lv_pct(100));

  // Informations placeholder
  const char *info_items[] = {
    LV_SYMBOL_SD_CARD " Carte SD: --",
    LV_SYMBOL_DIRECTORY " Espace libre: --",
    LV_SYMBOL_IMAGE " Cartes: --",
    LV_SYMBOL_LIST " Vols: --",
    LV_SYMBOL_HOME " Pilote: --"
  };

  for (int i = 0; i < 5; i++) {
    lv_obj_t *info_item = ui_create_label(info_panel, info_items[i],
                                          &lv_font_montserrat_20, lv_color_hex(0xaabbcc));
    lv_obj_set_width(info_item, lv_pct(100));
  }

  lv_screen_load(main_screen);

#ifdef DEBUG_MODE
  Serial.println("Prestart screen initialized");
#endif
}

/**
 * @brief Show prestart screen
 */
void ui_prestart_show(void) {
  ui_prestart_init();

#ifdef DEBUG_MODE
  Serial.println("Prestart screen shown");
#endif
}

#endif