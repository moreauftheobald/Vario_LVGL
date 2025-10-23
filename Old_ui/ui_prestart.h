#ifndef UI_PRESTART_H
#define UI_PRESTART_H

#include "lvgl.h"
#include "constants.h"
#include "UI_helper.h"
#include "lang.h"
#include "globals.h"
#include "src/params/params.h"
#include "src/ui/ui_main_screens.h"
#include "src/sd_card.h"
#include "src/ui/ui_file_transfer_loader.h"

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
  #ifdef DEBUG_MODE
  Serial.println("DEBUG: Debut ui_prestart_init");
  #endif
  
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
  lv_obj_set_size(btn_container, 400, LV_SIZE_CONTENT);
  lv_obj_set_style_pad_row(btn_container, 20, 0);

  ui_create_button(btn_container,
                   txt->file_transfer,
                   LV_SYMBOL_USB,
                   lv_color_hex(0x007aff),
                   400,
                   120,
                   &lv_font_montserrat_24,
                   &lv_font_montserrat_32,
                   btn_file_transfer_cb,
                   NULL,
                   (lv_align_t)0,
                   0,
                   0);

  ui_create_button(btn_container,
                   txt->settings,
                   LV_SYMBOL_SETTINGS,
                   lv_color_hex(0xff9500),
                   400,
                   120,
                   &lv_font_montserrat_24,
                   &lv_font_montserrat_32,
                   btn_settings_cb,
                   NULL,
                   (lv_align_t)0,
                   0,
                   0);

  ui_create_button(btn_container,
                   txt->start,
                   LV_SYMBOL_PLAY,
                   lv_color_hex(0x34c759),
                   400,
                   120,
                   &lv_font_montserrat_24,
                   &lv_font_montserrat_32,
                   btn_start_cb,
                   NULL,
                   (lv_align_t)0,
                   0,
                   0);

  // Colonne droite: Panel d'informations
  lv_obj_t *info_panel = ui_create_info_panel_bordered(content_container, 550, 450);
  lv_obj_set_style_pad_row(info_panel, 12, 0);

  // Titre du panel
  lv_obj_t *info_title = ui_create_label(info_panel, txt->information,
                                         &lv_font_montserrat_24, lv_color_hex(0x00d4ff));
  lv_obj_set_width(info_title, lv_pct(100));

  // Separateur
  ui_create_separator(info_panel);

  // Version
  char version_text[64];
  snprintf(version_text, sizeof(version_text), "%s %s: %s", LV_SYMBOL_SETTINGS, txt->version, VARIO_VERSION);
  lv_obj_t *label_version = ui_create_label(info_panel, version_text,
                                            &lv_font_montserrat_20, lv_color_hex(0xaabbcc));
  lv_obj_set_width(label_version, lv_pct(100));

  // === INFORMATIONS SD CARD ===
  char info_text[128];

  // SD Card - Status et capacite
  lv_color_t color_sd = lv_color_hex(0xff3b30);
  if (sd_is_ready()) {
    uint64_t total_kb, free_kb;
    sd_get_capacity(&total_kb, &free_kb);
    uint64_t total_gb = total_kb / (1024 * 1024);

    snprintf(info_text, sizeof(info_text), "%s %s: OK %lluGB",
             LV_SYMBOL_SD_CARD, txt->sd_card, total_gb);
    color_sd = lv_color_hex(0x34c759);
  } else {
    snprintf(info_text, sizeof(info_text), "%s %s: ERR",
             LV_SYMBOL_SD_CARD, txt->sd_card);
    color_sd = lv_color_hex(0xff3b30);
  }
  lv_obj_t *label_sd = ui_create_label(info_panel, info_text,
                                         &lv_font_montserrat_20, color_sd);  // Rouge
  lv_obj_set_width(label_sd, lv_pct(100));

  // Espace libre
    if (sd_is_ready()) {
    uint64_t total_kb, free_kb;
    sd_get_capacity(&total_kb, &free_kb);

    if (free_kb > 1024 * 1024) {
      uint64_t free_gb = free_kb / (1024 * 1024);
      snprintf(info_text, sizeof(info_text), "%s %s: %llu GB",
               LV_SYMBOL_DIRECTORY, txt->free_space, free_gb);
    } else {
      uint64_t free_mb = free_kb / 1024;
      snprintf(info_text, sizeof(info_text), "%s %s: %llu MB",
               LV_SYMBOL_DIRECTORY, txt->free_space, free_mb);
    }
  } else {
    snprintf(info_text, sizeof(info_text), "%s %s: --",
             LV_SYMBOL_DIRECTORY, txt->free_space);
  }
  lv_obj_t *label_space = ui_create_label(info_panel, info_text,
                                          &lv_font_montserrat_20, lv_color_hex(0xaabbcc));
  lv_obj_set_width(label_space, lv_pct(100));

  // Cartes OSM
  if (sd_is_ready()) {
    int osm_count = sd_count_osm_tiles();
    snprintf(info_text, sizeof(info_text), "%s %s: %d",
             LV_SYMBOL_IMAGE, txt->maps, osm_count);
  } else {
    snprintf(info_text, sizeof(info_text), "%s %s: --",
             LV_SYMBOL_IMAGE, txt->maps);
  }
  lv_obj_t *label_maps = ui_create_label(info_panel, info_text,
                                         &lv_font_montserrat_20, lv_color_hex(0xaabbcc));
  lv_obj_set_width(label_maps, lv_pct(100));

  // Vols IGC
  if (sd_is_ready()) {
    int igc_count = sd_count_igc_files();
    snprintf(info_text, sizeof(info_text), "%s %s: %d",
             LV_SYMBOL_LIST, txt->flights, igc_count);
  } else {
    snprintf(info_text, sizeof(info_text), "%s %s: --",
             LV_SYMBOL_LIST, txt->flights);
  }
  lv_obj_t *label_flights = ui_create_label(info_panel, info_text,
                                            &lv_font_montserrat_20, lv_color_hex(0xaabbcc));
  lv_obj_set_width(label_flights, lv_pct(100));

  // Pilote avec nom, prenom et telephone
  String pilot_display = String(LV_SYMBOL_HOME) + " " + txt->pilot + ": ";

  if (params.pilot_firstname != "" || params.pilot_name != "") {
    pilot_display += params.pilot_firstname;
    if (params.pilot_firstname != "" && params.pilot_name != "") {
      pilot_display += " ";
    }
    pilot_display += params.pilot_name;
  } else {
    pilot_display += "--";
  }

  lv_obj_t *label_pilot = ui_create_label(info_panel, pilot_display.c_str(),
                                          &lv_font_montserrat_20, lv_color_hex(0xaabbcc));
  lv_obj_set_width(label_pilot, lv_pct(100));

  // Telephone (si renseigne)
  if (params.pilot_phone != "") {
    String phone_display = String(LV_SYMBOL_CALL) + " " + txt->phone + ": " + params.pilot_phone;
    lv_obj_t *label_phone = ui_create_label(info_panel, phone_display.c_str(),
                                            &lv_font_montserrat_20, lv_color_hex(0xaabbcc));
    lv_obj_set_width(label_phone, lv_pct(100));
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