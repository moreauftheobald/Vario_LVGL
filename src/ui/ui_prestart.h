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
#include "src/wifi_task.h"
#include "src/metar_task.h"

// Forward declarations
void ui_file_transfer_show(void);
void ui_settings_show(void);

static lv_obj_t *label_bmp_status = NULL;
static lv_obj_t *label_bno_status = NULL;
static lv_obj_t *label_gps_status = NULL;
static lv_obj_t *label_wifi_status = NULL;
static lv_obj_t *label_qnh_status = NULL;
static lv_timer_t *sensor_status_timer = NULL;

// Callback timer pour mise à jour status capteurs
static void sensor_status_update_cb(lv_timer_t *timer) {
  // SÉCURITÉ: Vérifier que tous les labels existent
  if (!label_bmp_status || !label_bno_status || !label_gps_status || 
      !label_wifi_status || !label_qnh_status) {
#ifdef DEBUG_MODE
    Serial.println("[PRESTART] Timer callback: labels destroyed, stopping timer");
#endif
    if (timer) {
      lv_timer_del(timer);
      sensor_status_timer = NULL;
    }
    return;
  }

  // Double vérification que les labels sont valides
  if (!lv_obj_is_valid(label_bmp_status) || !lv_obj_is_valid(label_bno_status) || 
      !lv_obj_is_valid(label_gps_status) || !lv_obj_is_valid(label_wifi_status) ||
      !lv_obj_is_valid(label_qnh_status)) {
#ifdef DEBUG_MODE
    Serial.println("[PRESTART] Timer callback: labels invalid, stopping timer");
#endif
    if (timer) {
      lv_timer_del(timer);
      sensor_status_timer = NULL;
    }
    label_bmp_status = NULL;
    label_bno_status = NULL;
    label_gps_status = NULL;
    label_wifi_status = NULL;
    label_qnh_status = NULL;
    return;
  }

  char info_text[128];

  // Update BMP390
  snprintf(info_text, sizeof(info_text), "%s BMP390: %s",
           LV_SYMBOL_SETTINGS,
           g_sensor_data.bmp390.valid ? "OK" : "ERROR");
  lv_label_set_text(label_bmp_status, info_text);
  lv_obj_set_style_text_color(label_bmp_status,
                              g_sensor_data.bmp390.valid ? lv_color_hex(0x00ff00) : lv_color_hex(0xff0000), 0);

  // Update BNO080
  snprintf(info_text, sizeof(info_text), "%s BNO080: %s",
           LV_SYMBOL_GPS,
           g_sensor_data.bno080.valid ? "OK" : "ERROR");
  lv_label_set_text(label_bno_status, info_text);
  lv_obj_set_style_text_color(label_bno_status,
                              g_sensor_data.bno080.valid ? lv_color_hex(0x00ff00) : lv_color_hex(0xff0000), 0);

  // Update GPS
  if (g_sensor_data.gps.valid && g_sensor_data.gps.fix) {
    snprintf(info_text, sizeof(info_text), "%s GPS: FIX (%d sats)",
             LV_SYMBOL_WIFI,
             g_sensor_data.gps.satellites);
    lv_label_set_text(label_gps_status, info_text);
    lv_obj_set_style_text_color(label_gps_status, lv_color_hex(0x00ff00), 0);
  } else if (g_sensor_data.gps.valid) {
    snprintf(info_text, sizeof(info_text), "%s GPS: NO FIX (%d sats)",
             LV_SYMBOL_WIFI,
             g_sensor_data.gps.satellites);
    lv_label_set_text(label_gps_status, info_text);
    lv_obj_set_style_text_color(label_gps_status, lv_color_hex(0xffaa00), 0);
  } else {
    snprintf(info_text, sizeof(info_text), "%s GPS: ERROR",
             LV_SYMBOL_WIFI);
    lv_label_set_text(label_gps_status, info_text);
    lv_obj_set_style_text_color(label_gps_status, lv_color_hex(0xff0000), 0);
  }

  // Update WiFi
  if (wifi_get_connected_status()) {
    snprintf(info_text, sizeof(info_text), "%s WiFi: %s",
             LV_SYMBOL_WIFI,
             wifi_get_current_ssid());
    lv_label_set_text(label_wifi_status, info_text);
    lv_obj_set_style_text_color(label_wifi_status, lv_color_hex(0x00ff00), 0);
  } else {
    snprintf(info_text, sizeof(info_text), "%s WiFi: Connecting...",
             LV_SYMBOL_WIFI);
    lv_label_set_text(label_wifi_status, info_text);
    lv_obj_set_style_text_color(label_wifi_status, lv_color_hex(0xffaa00), 0);
  }

  // Update QNH depuis METAR
  float qnh = metar_get_qnh();
  metar_data_t metar;
  if (metar_get_data(&metar) && metar.valid) {
    snprintf(info_text, sizeof(info_text), "QNH: %.1f hPa (%s)",
             qnh, metar.station);
    lv_label_set_text(label_qnh_status, info_text);
    lv_obj_set_style_text_color(label_qnh_status, lv_color_hex(0x00ff00), 0);
  } else {
    snprintf(info_text, sizeof(info_text), "QNH: %.1f hPa (Standard)",
             qnh);
    lv_label_set_text(label_qnh_status, info_text);
    lv_obj_set_style_text_color(label_qnh_status, lv_color_hex(0xaabbcc), 0);
  }
}


// Callbacks
static void btn_file_transfer_cb(lv_event_t *e) {
#ifdef DEBUG_MODE
  Serial.println("File transfer button clicked");
#endif

  // Arrêter timer et services
  if (sensor_status_timer != NULL) {
    lv_timer_del(sensor_status_timer);
    sensor_status_timer = NULL;
    label_bmp_status = NULL;
    label_bno_status = NULL;
    label_gps_status = NULL;
    label_wifi_status = NULL;
    label_qnh_status = NULL;
  }

  // Arrêter WiFi et METAR
  metar_stop();
  wifi_task_stop();
  
  ui_file_transfer_show();
}

static void btn_settings_cb(lv_event_t *e) {
#ifdef DEBUG_MODE
  Serial.println("Settings button clicked");
#endif

  // Arrêter timer et services  
  if (sensor_status_timer != NULL) {
    lv_timer_del(sensor_status_timer);
    sensor_status_timer = NULL;
    label_bmp_status = NULL;
    label_bno_status = NULL;
    label_gps_status = NULL;
    label_wifi_status = NULL;
    label_qnh_status = NULL;
  }

  // Arrêter WiFi et METAR
  metar_stop();
  wifi_task_stop();

  ui_settings_show();
}

static void btn_start_cb(lv_event_t *e) {
#ifdef DEBUG_MODE
  Serial.println("Start button clicked");
#endif

  // Arrêter timer et services
  if (sensor_status_timer != NULL) {
    lv_timer_del(sensor_status_timer);
    sensor_status_timer = NULL;
    label_bmp_status = NULL;
    label_bno_status = NULL;
    label_gps_status = NULL;
    label_wifi_status = NULL;
    label_qnh_status = NULL;
  }

  // Arrêter WiFi et METAR
  metar_stop();
  wifi_task_stop();

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

  lv_obj_t *main_frame = ui_create_black_screen_with_frame(3, ROUND_FRANE_RADUIS_BIG, &main_screen);

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
  lv_obj_set_size(btn_container, 400, 475);
  lv_obj_set_style_pad_row(btn_container, 20, 0);
  lv_obj_set_flex_align(btn_container, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);

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
                   lv_color_hex(0xffaa00),
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
  lv_obj_t *info_panel = ui_create_info_panel_bordered(content_container, 550, 475);
  lv_obj_set_style_pad_row(info_panel, 8, 0);

  // Titre du panel
  lv_obj_t *info_title = ui_create_label(info_panel, txt->information,
                                         &lv_font_montserrat_24, lv_color_hex(0x00d4ff));
  lv_obj_set_width(info_title, lv_pct(100));

  // Separateur
  ui_create_separator(info_panel);

  // Version
  char info_text[128];
  snprintf(info_text, sizeof(info_text), "%s Version: %s", LV_SYMBOL_SETTINGS, VARIO_VERSION);
  lv_obj_t *label_version = ui_create_label(info_panel, info_text,
                                            &lv_font_montserrat_18, lv_color_hex(0xaabbcc));
  lv_obj_set_width(label_version, lv_pct(100));

  // === INFORMATIONS SD CARD ===
  // SD Card - Status et capacite
  lv_color_t color_sd = lv_color_hex(0xff0000);
  if (sd_is_ready()) {
    uint64_t total_kb, free_kb;
    sd_get_capacity(&total_kb, &free_kb);
    uint64_t total_gb = total_kb / (1024 * 1024);

    snprintf(info_text, sizeof(info_text), "%s SD: OK %lluGB",
             LV_SYMBOL_SD_CARD, total_gb);
    color_sd = lv_color_hex(0x00ff00);
  } else {
    snprintf(info_text, sizeof(info_text), "%s SD: ERROR",
             LV_SYMBOL_SD_CARD);
  }
  lv_obj_t *label_sd = ui_create_label(info_panel, info_text,
                                       &lv_font_montserrat_18, color_sd);
  lv_obj_set_width(label_sd, lv_pct(100));

  // === INFORMATIONS CAPTEURS ===

  // Status BMP390 (sauvegarde référence)
  snprintf(info_text, sizeof(info_text), "%s BMP390: %s",
           LV_SYMBOL_SETTINGS,
           g_sensor_data.bmp390.valid ? "OK" : "ERROR");
  label_bmp_status = ui_create_label(info_panel, info_text,
                                     &lv_font_montserrat_18,
                                     g_sensor_data.bmp390.valid ? lv_color_hex(0x00ff00) : lv_color_hex(0xff0000));
  lv_obj_set_width(label_bmp_status, lv_pct(100));

  // Status BNO080 (sauvegarde référence)
  snprintf(info_text, sizeof(info_text), "%s BNO080: %s",
           LV_SYMBOL_GPS,
           g_sensor_data.bno080.valid ? "OK" : "ERROR");
  label_bno_status = ui_create_label(info_panel, info_text,
                                     &lv_font_montserrat_18,
                                     g_sensor_data.bno080.valid ? lv_color_hex(0x00ff00) : lv_color_hex(0xff0000));
  lv_obj_set_width(label_bno_status, lv_pct(100));

  // Status GPS (sauvegarde référence)
  if (g_sensor_data.gps.valid && g_sensor_data.gps.fix) {
    snprintf(info_text, sizeof(info_text), "%s GPS: FIX (%d sats)",
             LV_SYMBOL_GPS,
             g_sensor_data.gps.satellites);
    label_gps_status = ui_create_label(info_panel, info_text,
                                       &lv_font_montserrat_18, lv_color_hex(0x00ff00));
  } else if (g_sensor_data.gps.valid) {
    snprintf(info_text, sizeof(info_text), "%s GPS: NO FIX (%d sats)",
             LV_SYMBOL_GPS,
             g_sensor_data.gps.satellites);
    label_gps_status = ui_create_label(info_panel, info_text,
                                       &lv_font_montserrat_18, lv_color_hex(0xffaa00));
  } else {
    snprintf(info_text, sizeof(info_text), "%s GPS: ERROR",
             LV_SYMBOL_GPS);
    label_gps_status = ui_create_label(info_panel, info_text,
                                       &lv_font_montserrat_18, lv_color_hex(0xff0000));
  }
  lv_obj_set_width(label_gps_status, lv_pct(100));

  // Status WiFi
  snprintf(info_text, sizeof(info_text), "%s WiFi: Connecting...",
           LV_SYMBOL_WIFI);
  label_wifi_status = ui_create_label(info_panel, info_text,
                                      &lv_font_montserrat_18, lv_color_hex(0xffaa00));
  lv_obj_set_width(label_wifi_status, lv_pct(100));

  // Status QNH
  snprintf(info_text, sizeof(info_text), "QNH: 1013.25 hPa (Standard)");
  label_qnh_status = ui_create_label(info_panel, info_text,
                                     &lv_font_montserrat_18, lv_color_hex(0xaabbcc));
  lv_obj_set_width(label_qnh_status, lv_pct(100));

  // Créer timer de mise à jour (1Hz)
  sensor_status_timer = lv_timer_create(sensor_status_update_cb, 1000, NULL);

  lv_screen_load(main_screen);

#ifdef DEBUG_MODE
  Serial.println("Prestart screen initialized");
#endif
}

/**
 * @brief Show prestart screen
 */
void ui_prestart_show(void) {
  // Arrêter timer précédent si existe
  if (sensor_status_timer != NULL) {
    lv_timer_del(sensor_status_timer);
    sensor_status_timer = NULL;
#ifdef DEBUG_MODE
    Serial.println("[PRESTART] Deleted existing timer");
#endif
  }
  // Reset labels
  label_bmp_status = NULL;
  label_bno_status = NULL;
  label_gps_status = NULL;
  label_wifi_status = NULL;
  label_qnh_status = NULL;

  ui_prestart_init();

  // Démarrer WiFi et METAR
  wifi_task_start();
  metar_start();
  metar_fetch();

#ifdef DEBUG_MODE
  Serial.println("Prestart screen shown - WiFi and METAR started");
#endif
}

#endif