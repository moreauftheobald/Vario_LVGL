#ifndef UI_SETTINGS_VARIO_H
#define UI_SETTINGS_VARIO_H

#include "lvgl.h"
#include "constants.h"
#include "UI_helper.h"
#include "lang.h"
#include "src/params/params.h"

extern void force_full_refresh(void);

void ui_settings_show(void);

static lv_obj_t *slider_integration = NULL;
static lv_obj_t *label_integration_value = NULL;
static lv_obj_t *chart_audio = NULL;
static lv_chart_series_t *series_audio = NULL;
static lv_obj_t *chart_container = NULL;

// Valeurs des frequences pour chaque vitesse vario (-5 a +10 m/s)
static uint16_t audio_frequencies[16] = {
  640, 664, 691, 727, 759, 789, 842, 920,
  998, 1060, 1097, 1121, 1144, 1161, 1170, 1175
};

// Callbacks
static void slider_integration_event_cb(lv_event_t *e) {
  lv_event_code_t code = lv_event_get_code(e);
  if (code == LV_EVENT_VALUE_CHANGED) {
    int value = (int)lv_slider_get_value(slider_integration);
    lv_label_set_text_fmt(label_integration_value, "%d s", value);

#ifdef DEBUG_MODE
    Serial.printf("Integration period changed to: %d s\n", value);
#endif
  }
}

static void save_vario_settings(void) {
  params.vario_integration_period = (int)lv_slider_get_value(slider_integration);

  for (int i = 0; i < 16; i++) {
    params.vario_audio_frequencies[i] = audio_frequencies[i];
  }

  params_save_vario();

#ifdef DEBUG_MODE
  Serial.println("Vario settings saved to params");
#endif
}

static void chart_audio_event_cb(lv_event_t *e) {
  lv_event_code_t code = lv_event_get_code(e);

  if (code == LV_EVENT_PRESSED || code == LV_EVENT_PRESSING) {
    lv_point_t point;
    lv_indev_t *indev = lv_indev_get_act();
    lv_indev_get_point(indev, &point);

    // Conversion coordonnees ecran vers coordonnees graphique
    lv_area_t chart_area;
    lv_obj_get_coords(chart_audio, &chart_area);

    int16_t relative_x = point.x - chart_area.x1;
    int16_t relative_y = point.y - chart_area.y1;

    int16_t chart_width = lv_area_get_width(&chart_area);
    int16_t chart_height = lv_area_get_height(&chart_area);

    // Determination du point le plus proche
    if (relative_x >= 0 && relative_x < chart_width && relative_y >= 0 && relative_y < chart_height) {

      // Calcul de l'index (0 a 15 pour -5 a +10)
      int point_index = (relative_x * 16) / chart_width;
      if (point_index < 0) point_index = 0;
      if (point_index > 15) point_index = 15;

      // Calcul de la frequence (600 a 1400 Hz)
      int16_t freq = 1400 - ((relative_y * 800) / chart_height);
      if (freq < 600) freq = 600;
      if (freq > 1400) freq = 1400;

      audio_frequencies[point_index] = freq;

      // Mise a jour du graphique
      lv_chart_set_value_by_id(chart_audio, series_audio, point_index, freq);
      lv_chart_refresh(chart_audio);

#ifdef DEBUG_MODE
      Serial.printf("Point %d: freq = %d Hz (vario = %d m/s)\n",
                    point_index, freq, point_index - 5);
#endif
    }
  }
}

static void btn_save_vario_cb(lv_event_t *e) {
#ifdef DEBUG_MODE
  Serial.println("Saving vario settings");
#endif

  save_vario_settings();

#ifdef DEBUG_MODE
  Serial.printf("Vario settings saved - Integration: %d s\n", params.vario_integration_period);
#endif

  ui_settings_show();
}

static void btn_cancel_vario_cb(lv_event_t *e) {
#ifdef DEBUG_MODE
  Serial.println("Cancelling vario settings");
#endif
  ui_settings_show();
}

static void btn_reset_audio_cb(lv_event_t *e) {
#ifdef DEBUG_MODE
  Serial.println("Resetting audio profile");
#endif

  // Reinitialisation valeurs par defaut
  uint16_t default_freqs[16] = {
    640, 664, 691, 727, 759, 789, 842, 920,
    998, 1060, 1097, 1121, 1144, 1161, 1170, 1175
  };

  for (int i = 0; i < 16; i++) {
    audio_frequencies[i] = default_freqs[i];
    lv_chart_set_value_by_id(chart_audio, series_audio, i, default_freqs[i]);
  }
  lv_chart_refresh(chart_audio);
}

static void load_vario_settings(void) {
  ui_load_slider_with_label(slider_integration, label_integration_value, params.vario_integration_period, "%d s");

  // Charger les frequences audio
  for (int i = 0; i < 16; i++) {
    audio_frequencies[i] = params.vario_audio_frequencies[i];
    if (series_audio) {
      lv_chart_set_value_by_id(chart_audio, series_audio, i, audio_frequencies[i]);
    }
  }

  //lv_chart_refresh(chart_audio);

#ifdef DEBUG_MODE
  Serial.println("Vario settings loaded from params");
#endif
}

void ui_settings_vario_init(void) {
  const TextStrings *txt = get_text();

  lv_obj_t *main_frame = ui_create_black_screen_with_frame(UI_BORDER_MEDIUM, UI_RADIUS_LARGE, &current_screen);
  ui_create_main_frame(main_frame, false, txt->vario_settings);

  // 1. Periode d'integration du vario
  lv_obj_t *integration_row = ui_create_form_row(main_left, "Periode d'integration", UI_HEADER_LINE_W, lv_color_hex(UI_COLOR_PRIMARY));
  slider_integration = ui_create_slider_with_label(integration_row, UI_SLIDER_VARIO_W, UI_SLIDER_VARIO_H,
                                                   INT_MIN_PER, INT_MAX_PER, 5, "%d s", UI_FONT_NORMAL,
                                                   lv_color_hex(UI_COLOR_PRIMARY), &label_integration_value);
  lv_obj_set_width(label_integration_value, 60);
  lv_obj_add_event_cb(slider_integration, slider_integration_event_cb, LV_EVENT_VALUE_CHANGED, NULL);

  // 2. Profil sonore avec graphique
  lv_obj_t *label_audio = ui_create_label(main_left, "Profil sonore (Vario m/s -> Frequence Hz)", UI_FONT_NORMAL, lv_color_hex(UI_COLOR_PRIMARY));

  // Container pour le graphique avec labels (augmente de 40px)
  chart_container = lv_obj_create(main_left);
  lv_obj_set_size(chart_container, lv_pct(100), UI_AUDIO_CHART_H);
  ui_set_panel_style(chart_container, lv_color_hex(UI_COLOR_AUDIO_CHART_BG), LV_OPA_80,
                     UI_BORDER_THIN, lv_color_hex(UI_COLOR_PRIMARY), UI_RADIUS_SMALL, 15);
  lv_obj_clear_flag(chart_container, LV_OBJ_FLAG_SCROLLABLE);

  // Labels axe Y (frequences) - 600, 800, 1000, 1200, 1400
  for (int i = 0; i <= 4; i++) {
    int freq = 1400 - (i * 200);
    lv_obj_t *label_y = lv_label_create(chart_container);
    lv_label_set_text_fmt(label_y, "%d", freq);
    lv_obj_set_style_text_font(label_y, &lv_font_montserrat_12, 0);
    lv_obj_set_style_text_color(label_y, lv_color_hex(UI_COLOR_TEXT_SECONDARY), 0);
    lv_obj_set_pos(label_y, 5, 20 + (i * 60));
  }

  // Labels axe X (vitesse vario) - -5 a +10 (centres sous chaque point)
  for (int i = 0; i <= 15; i++) {
    int vario = i - 5;
    lv_obj_t *label_x = lv_label_create(chart_container);
    lv_label_set_text_fmt(label_x, "%d", vario);
    lv_obj_set_style_text_font(label_x, &lv_font_montserrat_12, 0);
    lv_obj_set_style_text_color(label_x, lv_color_hex(UI_COLOR_TEXT_SECONDARY), 0);
    lv_obj_set_pos(label_x, 45 + 6 + (i * 870 / 15) - 6, 275);
  }

  // Creation du graphique (augmente de 40px)
  chart_audio = lv_chart_create(chart_container);
  lv_obj_set_size(chart_audio, 870, 250);
  lv_obj_set_pos(chart_audio, 45, 15);
  lv_chart_set_type(chart_audio, LV_CHART_TYPE_LINE);
  lv_chart_set_point_count(chart_audio, 16);
  lv_chart_set_range(chart_audio, LV_CHART_AXIS_PRIMARY_Y, MIN_FREQ, MAX_FREQ);

  // Suppression des lignes de division verticales (garder seulement horizontales)
  lv_chart_set_div_line_count(chart_audio, 5, 0);

  // Style du graphique
  lv_obj_set_style_bg_color(chart_audio, lv_color_hex(UI_COLOR_CHART_BG), 0);
  lv_obj_set_style_border_width(chart_audio, 1, 0);
  lv_obj_set_style_border_color(chart_audio, lv_color_hex(UI_COLOR_BORDER_INPUT), 0);
  lv_obj_set_style_line_color(chart_audio, lv_color_hex(UI_COLOR_CHART_LINE), LV_PART_ITEMS);

  // Taille des points
  lv_obj_set_style_size(chart_audio, 15, 15, LV_PART_INDICATOR);

  // Serie de donnees en ROUGE
  series_audio = lv_chart_add_series(chart_audio, lv_color_hex(UI_COLOR_ERROR), LV_CHART_AXIS_PRIMARY_Y);
  lv_obj_set_style_line_width(chart_audio, UI_BORDER_MEDIUM, LV_PART_ITEMS);

  // Couleur rouge pour les points aussi
  lv_obj_set_style_bg_color(chart_audio, lv_color_hex(UI_COLOR_ERROR), LV_PART_INDICATOR);

  // Initialisation des points
  for (int i = 0; i < 16; i++) {
    lv_chart_set_value_by_id(chart_audio, series_audio, i, audio_frequencies[i]);
  }

  // Activation du clic sur le graphique
  lv_obj_add_flag(chart_audio, LV_OBJ_FLAG_CLICKABLE);
  lv_obj_add_event_cb(chart_audio, chart_audio_event_cb, LV_EVENT_PRESSED, NULL);
  lv_obj_add_event_cb(chart_audio, chart_audio_event_cb, LV_EVENT_PRESSING, NULL);

  // Bouton Save
  lv_obj_t *btn_save_vario = ui_create_button(btn_container, txt->save, LV_SYMBOL_SAVE, lv_color_hex(UI_COLOR_BTN_START),
                                              UI_BTN_PRESTART_W, UI_BTN_PRESTART_H, UI_FONT_SMALL, UI_FONT_NORMAL, btn_save_vario_cb,
                                              NULL, (lv_align_t)0, NULL, NULL);

  // Bouton Cancel
  lv_obj_t *btn_cancel_vario = ui_create_button(btn_container, txt->cancel, LV_SYMBOL_BACKSPACE, lv_color_hex(UI_COLOR_BTN_CANCEL),
                                                UI_BTN_PRESTART_W, UI_BTN_PRESTART_H, UI_FONT_SMALL, UI_FONT_NORMAL, btn_cancel_vario_cb,
                                                NULL, (lv_align_t)0, NULL, NULL);

  // Bouton Reset
  lv_obj_t *btn_reset_vario = ui_create_button(btn_container, txt->reset, LV_SYMBOL_BACKSPACE, lv_color_hex(UI_COLOR_BTN_RESET),
                                               UI_BTN_PRESTART_W, UI_BTN_PRESTART_H, UI_FONT_SMALL, UI_FONT_NORMAL, btn_reset_audio_cb,
                                               NULL, (lv_align_t)0, NULL, NULL);

  load_vario_settings();

#ifdef DEBUG_MODE
  Serial.println("Vario settings screen initialized");
#endif
}

void ui_settings_vario_show(void) {
  ui_switch_screen(ui_settings_vario_init);
}

#endif