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
#include "src/ui/ui_file_transfer.h"
#include "src/wifi_task.h"
#include "src/metar_task.h"

void ui_file_transfer_show(void);
void ui_settings_show(void);

static lv_obj_t *label_bmp_status = NULL;
static lv_obj_t *label_bno_status = NULL;
static lv_obj_t *label_gps_status = NULL;
static lv_obj_t *label_wifi_status = NULL;
static lv_obj_t *label_qnh_status = NULL;
static lv_obj_t *label_kalman_status = NULL;
static lv_obj_t *label_sd_status = NULL;
static lv_obj_t *btn_start = NULL;
static lv_timer_t *sensor_status_timer = NULL;

// Verification conditions de demarrage
static bool check_start_conditions() {
  bool sd_ok = sd_is_ready();

#ifdef FLIGHT_TEST_MODE
  return sd_ok;
#else
  bool bmp_ok = g_sensor_data.bmp390.valid;
  bool bno_ok = g_sensor_data.bno080.valid;
  bool gps_ok = g_sensor_data.gps.valid && g_sensor_data.gps.fix;

  kalman_data_t kdata;
  kalman_get_data(&kdata);
  bool kalman_ok = kdata.valid;

  return (sd_ok && bmp_ok && bno_ok && gps_ok && kalman_ok);
#endif
}

// ============================================================================
// FONCTION OPTIMISEE: Mise a jour des labels de status capteurs
// Factorise le code duplique entre init et timer callback
// ============================================================================
static void update_sensor_status_labels(void) {
  // Buffer optimise de 64 bytes (suffit pour tous les messages)
  char buf[64];

  // SD Card
  if (sd_is_ready()) {
    uint64_t total_kb, free_kb;
    sd_get_capacity(&total_kb, &free_kb);
    uint64_t total_gb = total_kb / (1024 * 1024);
    uint64_t free_gb = free_kb / (1024 * 1024);

    snprintf(buf, sizeof(buf), "%s SD: %lluGB (%lluGB libre)",
             LV_SYMBOL_SD_CARD, total_gb, free_gb);
    lv_label_set_text(label_sd_status, buf);
    lv_obj_set_style_text_color(label_sd_status, lv_color_hex(OK_COLOR), 0);
  } else {
    snprintf(buf, sizeof(buf), "%s SD: ERROR", LV_SYMBOL_SD_CARD);
    lv_label_set_text(label_sd_status, buf);
    lv_obj_set_style_text_color(label_sd_status, lv_color_hex(KO_COLOR), 0);
  }

  // BMP390
  snprintf(buf, sizeof(buf), "%s BMP390: %s",
           LV_SYMBOL_SETTINGS,
           g_sensor_data.bmp390.valid ? "OK" : "ERROR");
  lv_label_set_text(label_bmp_status, buf);
  lv_obj_set_style_text_color(label_bmp_status,
                              g_sensor_data.bmp390.valid ? lv_color_hex(OK_COLOR) : lv_color_hex(KO_COLOR), 0);

  // BNO080
  snprintf(buf, sizeof(buf), "%s BNO080: %s",
           LV_SYMBOL_GPS,
           g_sensor_data.bno080.valid ? "OK" : "ERROR");
  lv_label_set_text(label_bno_status, buf);
  lv_obj_set_style_text_color(label_bno_status,
                              g_sensor_data.bno080.valid ? lv_color_hex(OK_COLOR) : lv_color_hex(KO_COLOR), 0);

  // GPS
  if (g_sensor_data.gps.valid && g_sensor_data.gps.fix) {
    snprintf(buf, sizeof(buf), "%s GPS: FIX (%d sats)",
             LV_SYMBOL_GPS, g_sensor_data.gps.satellites);
    lv_label_set_text(label_gps_status, buf);
    lv_obj_set_style_text_color(label_gps_status, lv_color_hex(OK_COLOR), 0);
  } else if (g_sensor_data.gps.valid) {
    snprintf(buf, sizeof(buf), "%s GPS: NO FIX (%d sats)",
             LV_SYMBOL_GPS, g_sensor_data.gps.satellites);
    lv_label_set_text(label_gps_status, buf);
    lv_obj_set_style_text_color(label_gps_status, lv_color_hex(WG_COLOR), 0);
  } else {
    snprintf(buf, sizeof(buf), "%s GPS: ERROR", LV_SYMBOL_GPS);
    lv_label_set_text(label_gps_status, buf);
    lv_obj_set_style_text_color(label_gps_status, lv_color_hex(KO_COLOR), 0);
  }

  // WiFi
  if (wifi_get_connected_status()) {
    snprintf(buf, sizeof(buf), "%s WiFi: %s",
             LV_SYMBOL_WIFI, wifi_get_current_ssid());
    lv_label_set_text(label_wifi_status, buf);
    lv_obj_set_style_text_color(label_wifi_status, lv_color_hex(OK_COLOR), 0);

    // Lancer recuperation METAR si WiFi OK
    static bool metar_fetched = false;

#ifdef FLIGHT_TEST_MODE
    if (!metar_fetched) {
      metar_fetch();
      metar_fetched = true;
#ifdef DEBUG_MODE
      Serial.println("[PRESTART] Flight Test - WiFi OK -> Fetch METAR");
#endif
    }
#else
    if (!metar_fetched && g_sensor_data.gps.valid && g_sensor_data.gps.fix) {
      metar_fetch();
      metar_fetched = true;
#ifdef DEBUG_MODE
      Serial.println("[PRESTART] GPS Fix + WiFi OK -> Fetch METAR");
#endif
    }
#endif
  } else {
    snprintf(buf, sizeof(buf), "%s WiFi: Connexion...", LV_SYMBOL_WIFI);
    lv_label_set_text(label_wifi_status, buf);
    lv_obj_set_style_text_color(label_wifi_status, lv_color_hex(WG_COLOR), 0);
  }

  // QNH METAR
  float qnh = metar_get_qnh();
  metar_data_t metar;
  if (metar_get_data(&metar) && metar.valid) {
    snprintf(buf, sizeof(buf), "QNH: %.1f hPa (%s)", qnh, metar.station);
    lv_label_set_text(label_qnh_status, buf);
    lv_obj_set_style_text_color(label_qnh_status, lv_color_hex(OK_COLOR), 0);
  } else {
    snprintf(buf, sizeof(buf), "QNH: %.1f hPa (Standard)", qnh);
    lv_label_set_text(label_qnh_status, buf);
    lv_obj_set_style_text_color(label_qnh_status, lv_color_hex(WG_COLOR), 0);
  }

  // Kalman
  kalman_data_t kdata;
  kalman_get_data(&kdata);
  snprintf(buf, sizeof(buf), "%s Kalman: %s",
           LV_SYMBOL_SETTINGS,
           kdata.valid ? "OK" : "INIT...");
  lv_label_set_text(label_kalman_status, buf);
  lv_obj_set_style_text_color(label_kalman_status,
                              kdata.valid ? lv_color_hex(OK_COLOR) : lv_color_hex(KO_COLOR), 0);

  // Activer/desactiver bouton Start
  if (btn_start) {
    bool can_start = check_start_conditions();

    if (can_start) {
      lv_obj_clear_state(btn_start, LV_STATE_DISABLED);
      lv_obj_set_style_bg_opa(btn_start, LV_OPA_COVER, 0);
    } else {
      lv_obj_add_state(btn_start, LV_STATE_DISABLED);
      lv_obj_set_style_bg_opa(btn_start, LV_OPA_50, LV_STATE_DISABLED);
    }
  }
}

// ============================================================================
// Callback timer SIMPLIFIE - appelle juste la fonction factorisee
// ============================================================================
static void sensor_status_update_cb(lv_timer_t *timer) {
  // Verification securite objets valides
  if (!label_bmp_status || !label_bno_status || !label_gps_status || !label_wifi_status || !label_qnh_status || !label_kalman_status || !label_sd_status) {
    if (timer) {
      lv_timer_del(timer);
      sensor_status_timer = NULL;
    }
    return;
  }

  if (!lv_obj_is_valid(label_bmp_status) || !lv_obj_is_valid(label_bno_status) || !lv_obj_is_valid(label_gps_status) || !lv_obj_is_valid(label_wifi_status) || !lv_obj_is_valid(label_qnh_status) || !lv_obj_is_valid(label_kalman_status) || !lv_obj_is_valid(label_sd_status)) {
    if (timer) {
      lv_timer_del(timer);
      sensor_status_timer = NULL;
    }
    return;
  }

  // OPTIMISE: Appel unique de la fonction factorisee
  update_sensor_status_labels();
}

// Callbacks boutons
static void btn_file_transfer_cb(lv_event_t *e) {
  if (sensor_status_timer != NULL) {
    lv_timer_del(sensor_status_timer);
    sensor_status_timer = NULL;
  }

  metar_stop();
  wifi_task_stop();
  ui_file_transfer_show();
}

static void btn_settings_cb(lv_event_t *e) {
  if (sensor_status_timer != NULL) {
    lv_timer_del(sensor_status_timer);
    sensor_status_timer = NULL;
  }

  metar_stop();
  wifi_task_stop();
  ui_settings_show();
}

static void btn_start_cb(lv_event_t *e) {
  if (sensor_status_timer != NULL) {
    lv_timer_del(sensor_status_timer);
    sensor_status_timer = NULL;
  }

  metar_stop();
  wifi_task_stop();
  ui_main_screens_show();
}

// ============================================================================
// Initialisation ecran prestart OPTIMISEE
// ============================================================================
void ui_prestart_init(void) {
#ifdef FLIGHT_TEST_MODE
  start_tile_cache_task(params.map_zoom, TEST_LAT, TEST_LON);
#endif

  const TextStrings *txt = get_text();

  lv_obj_t *main_frame = ui_create_black_screen_with_frame(3, ROUND_FRANE_RADUIS_BIG, &current_screen);
  ui_create_main_frame(main_frame, true, VARIO_NAME);

  // Left Col Content
  lv_obj_t *captors_title = ui_create_label(main_left, "Info Systeme",
                                            INFO_FONT_BIG, lv_color_hex(TITLE_COLOR));
  ui_create_separator(main_left);

  // Buffer optimise de 64 bytes
  char info_text[64];
  snprintf(info_text, sizeof(info_text), "Version Systeme: %s", VARIO_VERSION);
  lv_obj_t *label_version = ui_create_label(main_left, info_text,
                                            INFO_FONT_S, lv_color_hex(TITLE_COLOR));

  // Creation des labels de statut (vides, seront remplis par update_sensor_status_labels)
  label_sd_status = ui_create_label(main_left, "", INFO_FONT_S, lv_color_hex(OK_COLOR));
  label_bmp_status = ui_create_label(main_left, "", INFO_FONT_S, lv_color_hex(OK_COLOR));
  label_bno_status = ui_create_label(main_left, "", INFO_FONT_S, lv_color_hex(OK_COLOR));
  label_gps_status = ui_create_label(main_left, "", INFO_FONT_S, lv_color_hex(OK_COLOR));
  label_wifi_status = ui_create_label(main_left, "", INFO_FONT_S, lv_color_hex(WG_COLOR));
  label_qnh_status = ui_create_label(main_left, "", INFO_FONT_S, lv_color_hex(WG_COLOR));
  label_kalman_status = ui_create_label(main_left, "", INFO_FONT_S, lv_color_hex(WG_COLOR));

  // Avertissements modes speciaux
#if defined(DEBUG_MODE) || defined(TEST_MODE) || defined(FLIGHT_TEST_MODE)
  lv_obj_t *warning_panel = lv_obj_create(main_left);
  lv_obj_set_size(warning_panel, lv_pct(100), LV_SIZE_CONTENT);
  ui_set_flex_layout(warning_panel, LV_FLEX_FLOW_COLUMN,
                     LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
  lv_obj_set_style_pad_row(warning_panel, 5, 0);
  ui_set_panel_style(warning_panel, lv_color_hex(WG_COLOR), LV_OPA_COVER,
                     2, lv_color_hex(KO_COLOR), ROUND_FRANE_RADUIS_SMALL, 10);

  // Buffer optimise de 48 bytes
  char warning_text[48];

#ifdef DEBUG_MODE
  snprintf(warning_text, sizeof(warning_text), "%s DEBUG MODE ACTIVE", LV_SYMBOL_WARNING);
  lv_obj_t *label_debug = ui_create_label(warning_panel, warning_text,
                                          INFO_FONT_S, lv_color_hex(0x000000));
#endif

#ifdef TEST_MODE
  snprintf(warning_text, sizeof(warning_text), "%s TEST MODE ACTIVE", LV_SYMBOL_WARNING);
  lv_obj_t *label_test = ui_create_label(warning_panel, warning_text,
                                         INFO_FONT_S, lv_color_hex(0x000000));
#endif

#ifdef FLIGHT_TEST_MODE
  snprintf(warning_text, sizeof(warning_text), "%s FLIGHT TEST MODE ACTIVE", LV_SYMBOL_WARNING);
  lv_obj_t *label_flight = ui_create_label(warning_panel, warning_text,
                                           INFO_FONT_S, lv_color_hex(0x000000));
#endif
#endif

  // Right Col Content - Panneau pilote
  lv_obj_t *pilot_title = ui_create_label(main_right, txt->pilot,
                                          INFO_FONT_BIG, lv_color_hex(TITLE_COLOR));
  ui_create_separator(main_right);

  // Nom
  lv_obj_t *name_row = ui_create_inline_container(main_right);
  lv_obj_t *label_name_title = ui_create_label(name_row, txt->pilot_name,
                                               INFO_FONT_S, lv_color_hex(INFO_LABEL_COLOR));
  lv_obj_set_width(label_name_title, PRE_LINE_HEADER_W);
  lv_obj_t *label_name_sep = ui_create_label(name_row, ":",
                                             INFO_FONT_S, lv_color_hex(INFO_LABEL_COLOR));
  const char *name_str = psram_str_get(params.pilot_name);
  lv_obj_t *label_name_value = ui_create_label(name_row,
                                               (name_str && strlen(name_str) > 0) ? name_str : "---",
                                               INFO_FONT_S, lv_color_hex(INFO_DATAS_COLOR));

  // Prenom
  lv_obj_t *firstname_row = ui_create_inline_container(main_right);
  lv_obj_t *label_firstname_title = ui_create_label(firstname_row, txt->pilot_firstname,
                                                    INFO_FONT_S, lv_color_hex(INFO_LABEL_COLOR));
  lv_obj_set_width(label_firstname_title, PRE_LINE_HEADER_W);
  lv_obj_t *label_firstname_sep = ui_create_label(firstname_row, ":",
                                                  INFO_FONT_S, lv_color_hex(INFO_LABEL_COLOR));
  const char *firstname_str = psram_str_get(params.pilot_firstname);
  lv_obj_t *label_firstname_value = ui_create_label(firstname_row,
                                                    (firstname_str && strlen(firstname_str) > 0) ? firstname_str : "---",
                                                    INFO_FONT_S, lv_color_hex(INFO_DATAS_COLOR));

  // Telephone
  lv_obj_t *phone_row = ui_create_inline_container(main_right);
  lv_obj_t *label_phone_title = ui_create_label(phone_row, txt->pilot_phone,
                                                INFO_FONT_S, lv_color_hex(INFO_LABEL_COLOR));
  lv_obj_set_width(label_phone_title, PRE_LINE_HEADER_W);
  lv_obj_t *label_phone_sep = ui_create_label(phone_row, ":",
                                              INFO_FONT_S, lv_color_hex(INFO_LABEL_COLOR));
  const char *phone_str = psram_str_get(params.pilot_phone);
  lv_obj_t *label_phone_value = ui_create_label(phone_row,
                                                (phone_str && strlen(phone_str) > 0) ? phone_str : "---",
                                                INFO_FONT_S, lv_color_hex(INFO_DATAS_COLOR));

  // Voile
  lv_obj_t *wing_row = ui_create_inline_container(main_right);
  lv_obj_t *label_wing_title = ui_create_label(wing_row, txt->pilot_wing,
                                               INFO_FONT_S, lv_color_hex(INFO_LABEL_COLOR));
  lv_obj_set_width(label_wing_title, PRE_LINE_HEADER_W);
  lv_obj_t *label_wing_sep = ui_create_label(wing_row, ":",
                                             INFO_FONT_S, lv_color_hex(INFO_LABEL_COLOR));
  const char *wing_str = psram_str_get(params.pilot_wing);
  lv_obj_t *label_wing_value = ui_create_label(wing_row,
                                               (wing_str && strlen(wing_str) > 0) ? wing_str : "---",
                                               INFO_FONT_S, lv_color_hex(INFO_DATAS_COLOR));

  // Boutons
  btn_start = ui_create_button(btn_container, txt->start, LV_SYMBOL_PLAY,
                               lv_color_hex(START_BTN_COLOR),
                               PRE_BTN_W, PRE_BTN_H, INFO_FONT_BIG, INFO_FONT_BIG,
                               btn_start_cb, NULL, (lv_align_t)0, NULL, NULL);

  lv_obj_t *btn_settings = ui_create_button(btn_container, txt->settings, LV_SYMBOL_SETTINGS,
                                            lv_color_hex(SETUP_BTN_COLOR),
                                            PRE_BTN_W, PRE_BTN_H, INFO_FONT_BIG, INFO_FONT_BIG,
                                            btn_settings_cb, NULL, (lv_align_t)0, NULL, NULL);

  lv_obj_t *btn_file_transfer = ui_create_button(btn_container, txt->file_transfer, LV_SYMBOL_USB,
                                                 lv_color_hex(FILES_BTN_COLOR),
                                                 PRE_BTN_W, PRE_BTN_H, INFO_FONT_BIG, INFO_FONT_BIG,
                                                 btn_file_transfer_cb, NULL, (lv_align_t)0, NULL, NULL);

  // OPTIMISE: Mise a jour initiale des labels via fonction factorisee
  update_sensor_status_labels();

  // Demarrage WiFi
  wifi_task_start();

  // Timer pour mise a jour periodique
  sensor_status_timer = lv_timer_create(sensor_status_update_cb, 1000, NULL);

#ifdef DEBUG_MODE
  Serial.println("[PRESTART] Screen initialized");
#endif
}

void ui_prestart_show(void) {
  ui_switch_screen(ui_prestart_init);
}

#endif