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
  // Verifier SD Card (obligatoire pour tous les modes)
  bool sd_ok = sd_is_ready();

#ifdef FLIGHT_TEST_MODE
  // Mode Test Flight: SD obligatoire, reste optionnel
  return sd_ok;
#else
  // Mode Normal/Test: verifier tous les capteurs + SD
  bool bmp_ok = g_sensor_data.bmp390.valid;
  bool bno_ok = g_sensor_data.bno080.valid;
  bool gps_ok = g_sensor_data.gps.valid && g_sensor_data.gps.fix;

  kalman_data_t kdata;
  kalman_get_data(&kdata);
  bool kalman_ok = kdata.valid;

  return (sd_ok && bmp_ok && bno_ok && gps_ok && kalman_ok);
#endif
}

// Callback timer pour mise a jour status
static void sensor_status_update_cb(lv_timer_t *timer) {
  if (!label_bmp_status || !label_bno_status || !label_gps_status || !label_wifi_status || !label_qnh_status || !label_kalman_status || !label_sd_status) {
#ifdef DEBUG_MODE
    Serial.println("[PRESTART] Labels invalides, arret timer");
#endif
    if (timer) {
      lv_timer_del(timer);
      sensor_status_timer = NULL;
    }
    return;
  }

  if (!lv_obj_is_valid(label_bmp_status) || !lv_obj_is_valid(label_bno_status) || !lv_obj_is_valid(label_gps_status) || !lv_obj_is_valid(label_wifi_status) || !lv_obj_is_valid(label_qnh_status) || !lv_obj_is_valid(label_kalman_status) || !lv_obj_is_valid(label_sd_status)) {
#ifdef DEBUG_MODE
    Serial.println("[PRESTART] Labels detruits, arret timer");
#endif
    if (timer) {
      lv_timer_del(timer);
      sensor_status_timer = NULL;
    }
    return;
  }

  char info_text[128];

  // SD Card
  if (sd_is_ready()) {
    uint64_t total_kb, free_kb;
    sd_get_capacity(&total_kb, &free_kb);
    uint64_t total_gb = total_kb / (1024 * 1024);
    uint64_t free_gb = free_kb / (1024 * 1024);

    snprintf(info_text, sizeof(info_text), "%s SD: %lluGB (%lluGB libre)",
             LV_SYMBOL_SD_CARD, total_gb, free_gb);
    lv_label_set_text(label_sd_status, info_text);
    lv_obj_set_style_text_color(label_sd_status, lv_color_hex(OK_COLOR), 0);
  } else {
    snprintf(info_text, sizeof(info_text), "%s SD: ERROR",
             LV_SYMBOL_SD_CARD);
    lv_label_set_text(label_sd_status, info_text);
    lv_obj_set_style_text_color(label_sd_status, lv_color_hex(KO_COLOR), 0);
  }

  // BMP390
  snprintf(info_text, sizeof(info_text), "%s BMP390: %s", LV_SYMBOL_SETTINGS, g_sensor_data.bmp390.valid ? "OK" : "ERROR");
  lv_label_set_text(label_bmp_status, info_text);
  lv_obj_set_style_text_color(label_bmp_status, g_sensor_data.bmp390.valid ? lv_color_hex(OK_COLOR) : lv_color_hex(KO_COLOR), 0);

  // BNO080
  snprintf(info_text, sizeof(info_text), "%s BNO080: %s",
           LV_SYMBOL_GPS,
           g_sensor_data.bno080.valid ? "OK" : "ERROR");
  lv_label_set_text(label_bno_status, info_text);
  lv_obj_set_style_text_color(label_bno_status,
                              g_sensor_data.bno080.valid ? lv_color_hex(OK_COLOR) : lv_color_hex(KO_COLOR), 0);

  // GPS
  if (g_sensor_data.gps.valid && g_sensor_data.gps.fix) {
    snprintf(info_text, sizeof(info_text), "%s GPS: FIX (%d sats)",
             LV_SYMBOL_GPS,
             g_sensor_data.gps.satellites);
    lv_label_set_text(label_gps_status, info_text);
    lv_obj_set_style_text_color(label_gps_status, lv_color_hex(OK_COLOR), 0);
  } else if (g_sensor_data.gps.valid) {
    snprintf(info_text, sizeof(info_text), "%s GPS: NO FIX (%d sats)",
             LV_SYMBOL_GPS,
             g_sensor_data.gps.satellites);
    lv_label_set_text(label_gps_status, info_text);
    lv_obj_set_style_text_color(label_gps_status, lv_color_hex(WG_COLOR), 0);
  } else {
    snprintf(info_text, sizeof(info_text), "%s GPS: ERROR",
             LV_SYMBOL_GPS);
    lv_label_set_text(label_gps_status, info_text);
    lv_obj_set_style_text_color(label_gps_status, lv_color_hex(KO_COLOR), 0);
  }

  // WiFi
  if (wifi_get_connected_status()) {
    snprintf(info_text, sizeof(info_text), "%s WiFi: %s",
             LV_SYMBOL_WIFI,
             wifi_get_current_ssid());
    lv_label_set_text(label_wifi_status, info_text);
    lv_obj_set_style_text_color(label_wifi_status, lv_color_hex(OK_COLOR), 0);

    // Lancer recuperation METAR si WiFi OK
    static bool metar_fetched = false;

#ifdef FLIGHT_TEST_MODE
    // Mode Flight Test: fetch immediat des que WiFi connecte (pas besoin GPS)
    if (!metar_fetched) {
      metar_fetch();
      metar_fetched = true;
#ifdef DEBUG_MODE
      Serial.println("[PRESTART] Flight Test Mode - WiFi OK -> Fetch METAR (position test)");
#endif
    }
#else
    // Mode Normal/Test: fetch seulement si GPS Fix
    if (!metar_fetched && g_sensor_data.gps.valid && g_sensor_data.gps.fix) {
      metar_fetch();
      metar_fetched = true;
#ifdef DEBUG_MODE
      Serial.println("[PRESTART] GPS Fix + WiFi OK -> Fetch METAR");
#endif
    }
#endif
  } else {
    snprintf(info_text, sizeof(info_text), "%s WiFi: Connexion...",
             LV_SYMBOL_WIFI);
    lv_label_set_text(label_wifi_status, info_text);
    lv_obj_set_style_text_color(label_wifi_status, lv_color_hex(WG_COLOR), 0);
  }

  // QNH METAR
  float qnh = metar_get_qnh();
  metar_data_t metar;
  if (metar_get_data(&metar) && metar.valid) {
    snprintf(info_text, sizeof(info_text), "QNH: %.1f hPa (%s)",
             qnh, metar.station);
    lv_label_set_text(label_qnh_status, info_text);
    lv_obj_set_style_text_color(label_qnh_status, lv_color_hex(OK_COLOR), 0);
  } else {
    snprintf(info_text, sizeof(info_text), "QNH: %.1f hPa (Standard)",
             qnh);
    lv_label_set_text(label_qnh_status, info_text);
    lv_obj_set_style_text_color(label_qnh_status, lv_color_hex(WG_COLOR), 0);
  }

  // Kalman
  kalman_data_t kdata;
  kalman_get_data(&kdata);
  snprintf(info_text, sizeof(info_text), "%s Kalman: %s",
           LV_SYMBOL_SETTINGS,
           kdata.valid ? "OK" : "INIT...");
  lv_label_set_text(label_kalman_status, info_text);
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

// Callbacks boutons
static void btn_file_transfer_cb(lv_event_t *e) {
#ifdef DEBUG_MODE
  Serial.println("[PRESTART] File transfer clicked");
#endif

  if (sensor_status_timer != NULL) {
    lv_timer_del(sensor_status_timer);
    sensor_status_timer = NULL;
  }

  metar_stop();
  wifi_task_stop();
  ui_file_transfer_show();
}

static void btn_settings_cb(lv_event_t *e) {
#ifdef DEBUG_MODE
  Serial.println("[PRESTART] Settings clicked");
#endif

  if (sensor_status_timer != NULL) {
    lv_timer_del(sensor_status_timer);
    sensor_status_timer = NULL;
  }

  metar_stop();
  wifi_task_stop();
  ui_settings_show();
}

static void btn_start_cb(lv_event_t *e) {
#ifdef DEBUG_MODE
  Serial.println("[PRESTART] Start clicked");
#endif

  if (sensor_status_timer != NULL) {
    lv_timer_del(sensor_status_timer);
    sensor_status_timer = NULL;
  }

  metar_stop();
  wifi_task_stop();
  ui_main_screens_show();
}

// Initialisation ecran prestart
void ui_prestart_init(void) {
#ifdef DEBUG_MODE
  Serial.println("[PRESTART] Init");
#endif

  const TextStrings *txt = get_text();

  lv_obj_t *main_frame = ui_create_black_screen_with_frame(3, ROUND_FRANE_RADUIS_BIG, &main_screen);

  ui_create_main_frame(main_frame, true, VARIO_NAME);

  //Left Col Content
  lv_obj_t *captors_title = ui_create_label(main_left, "Info Systeme", INFO_FONT_BIG, lv_color_hex(TITLE_COLOR));

  // Separateur
  ui_create_separator(main_left);

  char info_text[128];
  snprintf(info_text, sizeof(info_text), "Version Systeme: %s", VARIO_VERSION);

  lv_obj_t *label_version = ui_create_label(main_left, info_text, INFO_FONT_S, lv_color_hex(TITLE_COLOR));

  // SD Card
  if (sd_is_ready()) {
    uint64_t total_kb, free_kb;
    sd_get_capacity(&total_kb, &free_kb);
    uint64_t total_gb = total_kb / (1024 * 1024);
    uint64_t free_gb = free_kb / (1024 * 1024);

    snprintf(info_text, sizeof(info_text), "%s SD: %lluGB (%lluGB libre)", LV_SYMBOL_SD_CARD, total_gb, free_gb);
    label_sd_status = ui_create_label(main_left, info_text, INFO_FONT_S, lv_color_hex(OK_COLOR));
  } else {
    snprintf(info_text, sizeof(info_text), "%s SD: ERROR", LV_SYMBOL_SD_CARD);
    label_sd_status = ui_create_label(main_left, info_text, INFO_FONT_S, lv_color_hex(KO_COLOR));
  }

  // BMP390
  snprintf(info_text, sizeof(info_text), "%s BMP390: %s", LV_SYMBOL_SETTINGS, g_sensor_data.bmp390.valid ? "OK" : "ERROR");
  label_bmp_status = ui_create_label(main_left, info_text, INFO_FONT_S, g_sensor_data.bmp390.valid ? lv_color_hex(OK_COLOR) : lv_color_hex(KO_COLOR));

  // BNO080
  snprintf(info_text, sizeof(info_text), "%s BNO080: %s", LV_SYMBOL_GPS, g_sensor_data.bno080.valid ? "OK" : "ERROR");
  label_bno_status = ui_create_label(main_left, info_text, INFO_FONT_S, g_sensor_data.bno080.valid ? lv_color_hex(OK_COLOR) : lv_color_hex(KO_COLOR));

  // GPS
  if (g_sensor_data.gps.valid && g_sensor_data.gps.fix) {
    snprintf(info_text, sizeof(info_text), "%s GPS: FIX (%d sats)", LV_SYMBOL_GPS, g_sensor_data.gps.satellites);
    label_gps_status = ui_create_label(main_left, info_text, INFO_FONT_S, lv_color_hex(OK_COLOR));
  } else if (g_sensor_data.gps.valid) {
    snprintf(info_text, sizeof(info_text), "%s GPS: NO FIX (%d sats)", LV_SYMBOL_GPS, g_sensor_data.gps.satellites);
    label_gps_status = ui_create_label(main_left, info_text, INFO_FONT_S, lv_color_hex(WG_COLOR));
  } else {
    snprintf(info_text, sizeof(info_text), "%s GPS: ERROR", LV_SYMBOL_GPS);
    label_gps_status = ui_create_label(main_left, info_text, INFO_FONT_S, lv_color_hex(KO_COLOR));
  }


  // WiFi
  snprintf(info_text, sizeof(info_text), "%s WiFi: Connexion...", LV_SYMBOL_WIFI);
  label_wifi_status = ui_create_label(main_left, info_text, INFO_FONT_S, lv_color_hex(WG_COLOR));

  // QNH
  snprintf(info_text, sizeof(info_text), "QNH: 1013.25 hPa (Standard)");
  label_qnh_status = ui_create_label(main_left, info_text, INFO_FONT_S, lv_color_hex(WG_COLOR));

  // Kalman
  snprintf(info_text, sizeof(info_text), "%s Kalman: INIT...", LV_SYMBOL_SETTINGS);
  label_kalman_status = ui_create_label(main_left, info_text, INFO_FONT_S, lv_color_hex(WG_COLOR));


  // Avertissements modes speciaux
#if defined(DEBUG_MODE) || defined(TEST_MODE) || defined(FLIGHT_TEST_MODE)
  lv_obj_t *warning_panel = lv_obj_create(main_left);
  lv_obj_set_size(warning_panel, lv_pct(100), LV_SIZE_CONTENT);
  lv_obj_set_flex_flow(warning_panel, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_flex_align(warning_panel, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
  lv_obj_set_style_pad_row(warning_panel, 5, 0);
  lv_obj_set_style_pad_all(warning_panel, 10, 0);
  lv_obj_set_style_bg_color(warning_panel, lv_color_hex(WG_COLOR), 0);
  lv_obj_set_style_border_width(warning_panel, 2, 0);
  lv_obj_set_style_border_color(warning_panel, lv_color_hex(KO_COLOR), 0);
  lv_obj_set_style_radius(warning_panel, ROUND_FRANE_RADUIS_SMALL, 0);

  char warning_text[128];
#ifdef DEBUG_MODE
  snprintf(warning_text, sizeof(warning_text), "%s DEBUG MODE ACTIVE", LV_SYMBOL_WARNING);
  lv_obj_t *label_debug = ui_create_label(warning_panel, warning_text, INFO_FONT_S, lv_color_hex(0x000000));
#endif

#ifdef TEST_MODE
  snprintf(warning_text, sizeof(warning_text), "%s TEST MODE ACTIVE", LV_SYMBOL_WARNING);
  lv_obj_t *label_test = ui_create_label(warning_panel, warning_text, INFO_FONT_S, lv_color_hex(0x000000));
#endif

#ifdef FLIGHT_TEST_MODE
  snprintf(warning_text, sizeof(warning_text), "%s FLIGHT TEST MODE ACTIVE", LV_SYMBOL_WARNING);
  lv_obj_t *label_flight = ui_create_label(warning_panel, warning_text, INFO_FONT_S, lv_color_hex(0x000000));
#endif
#endif

  //Right Col Content
  // Titre panneau pilote
  lv_obj_t *pilot_title = ui_create_label(main_right, txt->pilot, INFO_FONT_BIG, lv_color_hex(TITLE_COLOR));

  // Separateur
  ui_create_separator(main_right);

  // Nom (libelle et valeur sur meme ligne)
  lv_obj_t *name_row = ui_create_inline_container(main_right);
  lv_obj_t *label_name_title = ui_create_label(name_row, txt->pilot_name, INFO_FONT_S, lv_color_hex(INFO_LABEL_COLOR));
  lv_obj_set_width(label_name_title, PRE_LINE_HEADER_W);
  lv_obj_t *label_name_sep = ui_create_label(name_row, ":", INFO_FONT_S, lv_color_hex(INFO_LABEL_COLOR));
  const char *name_str = psram_str_get(params.pilot_name);
  lv_obj_t *label_name_value = ui_create_label(name_row, (name_str && strlen(name_str) > 0) ? name_str : "---", INFO_FONT_S, lv_color_hex(INFO_DATAS_COLOR));

  // Prenom (libelle et valeur sur meme ligne)
  lv_obj_t *firstname_row = ui_create_inline_container(main_right);
  lv_obj_t *label_firstname_title = ui_create_label(firstname_row, txt->pilot_firstname, INFO_FONT_S, lv_color_hex(INFO_LABEL_COLOR));
  lv_obj_set_width(label_firstname_title, PRE_LINE_HEADER_W);
  lv_obj_t *label_firstname_sep = ui_create_label(firstname_row, ":", INFO_FONT_S, lv_color_hex(INFO_LABEL_COLOR));
  const char *firstname_str = psram_str_get(params.pilot_firstname);
  lv_obj_t *label_firstname_value = ui_create_label(firstname_row, (firstname_str && strlen(firstname_str) > 0) ? firstname_str : "---", INFO_FONT_S, lv_color_hex(INFO_DATAS_COLOR));

  // Telephone (libelle et valeur sur meme ligne)
  lv_obj_t *phone_row = ui_create_inline_container(main_right);
  lv_obj_t *label_phone_title = ui_create_label(phone_row, txt->pilot_phone, INFO_FONT_S, lv_color_hex(INFO_LABEL_COLOR));
  lv_obj_set_width(label_phone_title, PRE_LINE_HEADER_W);
  lv_obj_t *label_phone_sep = ui_create_label(phone_row, ":", INFO_FONT_S, lv_color_hex(INFO_LABEL_COLOR));
  const char *phone_str = psram_str_get(params.pilot_phone);
  lv_obj_t *label_phone_value = ui_create_label(phone_row, (phone_str && strlen(phone_str) > 0) ? phone_str : "---", INFO_FONT_S, lv_color_hex(INFO_DATAS_COLOR));

  // Modele d'aile (libelle et valeur sur meme ligne)
  lv_obj_t *wing_row = ui_create_inline_container(main_right);
  lv_obj_t *label_wing_title = ui_create_label(wing_row, txt->pilot_wing, INFO_FONT_S, lv_color_hex(INFO_LABEL_COLOR));
  lv_obj_set_width(label_wing_title, PRE_LINE_HEADER_W);
  lv_obj_t *label_wing_sep = ui_create_label(wing_row, ":", INFO_FONT_S, lv_color_hex(INFO_LABEL_COLOR));
  const char *wing_str = psram_str_get(params.pilot_wing);
  lv_obj_t *label_wing_value = ui_create_label(wing_row, (wing_str && strlen(wing_str) > 0) ? wing_str : "---", INFO_FONT_S, lv_color_hex(INFO_DATAS_COLOR));

  // Separateur avant I.C.E.
  lv_obj_t *ice_separator = lv_obj_create(main_right);
  lv_obj_set_size(ice_separator, lv_pct(100), 2);
  lv_obj_set_style_bg_color(ice_separator, lv_color_hex(0x000000), 0);
  lv_obj_set_style_bg_opa(ice_separator, LV_OPA_COVER, 0);
  lv_obj_set_style_border_width(ice_separator, 0, 0);
  lv_obj_set_style_pad_all(ice_separator, 0, 0);

  // Section I.C.E. (In Case of Emergency)
  lv_obj_t *ice_title = ui_create_label(main_right, "I.C.E.", INFO_FONT_BIG, lv_color_hex(INFO_LED_L_COLOR));
  lv_obj_set_width(ice_title, lv_pct(100));

  ui_create_separator(main_right);

  // Nom ICE (meme format que les autres)
  lv_obj_t *ice_name_row = ui_create_inline_container(main_right);
  lv_obj_t *label_ice_name_title = ui_create_label(ice_name_row, txt->pilot_name, INFO_FONT_S, lv_color_hex(INFO_LABEL_COLOR));
  lv_obj_set_width(label_ice_name_title, PRE_LINE_HEADER_W);
  lv_obj_t *label_ice_name_sep = ui_create_label(ice_name_row, ":", INFO_FONT_S, lv_color_hex(INFO_LABEL_COLOR));
  lv_obj_t *label_ice_name_value = ui_create_label(ice_name_row, "---", INFO_FONT_S, lv_color_hex(INFO_DATAS_COLOR));

  // Telephone ICE
  lv_obj_t *ice_phone_row = ui_create_inline_container(main_right);
  lv_obj_t *label_ice_phone_title = ui_create_label(ice_phone_row, txt->pilot_phone, INFO_FONT_S, lv_color_hex(INFO_LABEL_COLOR));
  lv_obj_set_width(label_ice_phone_title, PRE_LINE_HEADER_W);
  lv_obj_t *label_ice_phone_sep = ui_create_label(ice_phone_row, ":", INFO_FONT_S, lv_color_hex(INFO_LABEL_COLOR));
  lv_obj_t *label_ice_phone_value = ui_create_label(ice_phone_row, "---", INFO_FONT_S, lv_color_hex(INFO_DATAS_COLOR));

  //Right Col Content
  // Bouton Transfer
  lv_obj_t *btn_transfer = ui_create_button(btn_container, txt->file_transfer, LV_SYMBOL_UPLOAD, lv_color_hex(FILES_BTN_COLOR),
                                            PRE_BTN_W, PRE_BTN_H, INFO_FONT_S, INFO_FONT_BIG, btn_file_transfer_cb,
                                            NULL, (lv_align_t)0, NULL, NULL);

  // Bouton Settings
  lv_obj_t *btn_settings_obj = ui_create_button(btn_container, txt->settings, LV_SYMBOL_SETTINGS, lv_color_hex(SETUP_BTN_COLOR),
                                                PRE_BTN_W, PRE_BTN_H, INFO_FONT_S, INFO_FONT_BIG, btn_settings_cb,
                                                NULL, (lv_align_t)0, NULL, NULL);

  // Bouton Start (desactive au debut)
  btn_start = ui_create_button(btn_container, txt->start, LV_SYMBOL_PLAY, lv_color_hex(START_BTN_COLOR),
                               PRE_BTN_W, PRE_BTN_H, INFO_FONT_S, INFO_FONT_BIG, btn_start_cb,
                               NULL, (lv_align_t)0, NULL, NULL);

#ifdef FLIGHT_TEST_MODE
  // Mode Test Flight: Start immediatement disponible
  lv_obj_clear_state(btn_start, LV_STATE_DISABLED);
#else
  // Mode Normal/Test: bouton desactive jusqu'a conditions OK
  lv_obj_add_state(btn_start, LV_STATE_DISABLED);
  lv_obj_set_style_bg_opa(btn_start, LV_OPA_50, LV_STATE_DISABLED);
#endif

  // Timer de mise a jour (1Hz)
  sensor_status_timer = lv_timer_create(sensor_status_update_cb, 1000, NULL);
  if (lvgl_port_lock(-1)) {
    lv_screen_load(main_screen);
    lvgl_port_unlock();
  }

#ifdef DEBUG_MODE
  Serial.println("[PRESTART] Screen loaded");
#endif
}

// Affichage ecran prestart
void ui_prestart_show(void) {

  if (sensor_status_timer != NULL) {
    lv_timer_del(sensor_status_timer);
    sensor_status_timer = NULL;
  }

  label_bmp_status = NULL;
  label_bno_status = NULL;
  label_gps_status = NULL;
  label_wifi_status = NULL;
  label_qnh_status = NULL;
  label_kalman_status = NULL;
  label_sd_status = NULL;
  btn_start = NULL;

  // Sauvegarder ancien écran
  lv_obj_t *old_screen = lv_scr_act();

  ui_prestart_init();

   Détruire l'ancien écran SI ce n'est pas le même
  if (old_screen != main_screen && old_screen != NULL) {
    lv_obj_del(old_screen);
#ifdef DEBUG_MODE
    Serial.println("[PRESTART] Old screen deleted");
#endif
  }

  // Demarrer WiFi et METAR
  wifi_task_start();
  metar_start();

#ifdef DEBUG_MODE
  Serial.println("[PRESTART] WiFi/METAR started");
#endif
}

#endif