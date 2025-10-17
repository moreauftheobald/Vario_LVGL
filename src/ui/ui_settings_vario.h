#ifndef UI_SETTINGS_VARIO_H
#define UI_SETTINGS_VARIO_H

#include "lvgl.h"
#include "constants.h"
#include "UI_helper.h"
#include "lang.h"
#include <Preferences.h>

extern Preferences prefs;
extern lv_obj_t *main_screen;
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
    if (relative_x >= 0 && relative_x < chart_width && 
        relative_y >= 0 && relative_y < chart_height) {
      
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
  
  prefs.begin("vario", false);
  int integration = (int)lv_slider_get_value(slider_integration);
  
  prefs.putInt("integration", integration);
  
  // Sauvegarde des frequences
  for (int i = 0; i < 16; i++) {
    char key[16];
    snprintf(key, sizeof(key), "freq_%d", i);
    prefs.putUShort(key, audio_frequencies[i]);
  }
  
  prefs.end();
  
#ifdef DEBUG_MODE
  Serial.printf("Vario settings saved - Integration: %d s\n", integration);
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
  prefs.begin("vario", true);
  int integration = prefs.getInt("integration", 5);
  
  // Valeurs par defaut
  uint16_t default_freqs[16] = {
    640, 664, 691, 727, 759, 789, 842, 920, 
    998, 1060, 1097, 1121, 1144, 1161, 1170, 1175
  };
  
  // Chargement des frequences
  for (int i = 0; i < 16; i++) {
    char key[16];
    snprintf(key, sizeof(key), "freq_%d", i);
    audio_frequencies[i] = prefs.getUShort(key, default_freqs[i]);
    
    if (series_audio) {
      lv_chart_set_value_by_id(chart_audio, series_audio, i, audio_frequencies[i]);
    }
  }
  
  if (slider_integration) {
    lv_slider_set_value(slider_integration, integration, LV_ANIM_OFF);
    if (label_integration_value) {
      lv_label_set_text_fmt(label_integration_value, "%d s", integration);
    }
  }
  
  if (chart_audio) {
    lv_chart_refresh(chart_audio);
  }
  
  prefs.end();
  
#ifdef DEBUG_MODE
  Serial.println("Vario settings loaded");
  Serial.printf("Integration: %d s\n", integration);
#endif
}

void ui_settings_vario_init(void) {
  const TextStrings *txt = get_text();
  
  // Nettoyer l'ecran s'il existe
  if (main_screen != NULL) {
    lv_obj_clean(main_screen);
  } else {
    main_screen = lv_obj_create(NULL);
  }
  
  lv_obj_set_style_bg_color(main_screen, lv_color_hex(0x0a0e27), 0);
  lv_obj_set_style_bg_grad_color(main_screen, lv_color_hex(0x1a1f3a), 0);
  lv_obj_set_style_bg_grad_dir(main_screen, LV_GRAD_DIR_VER, 0);
  
  lv_obj_t *main_frame = ui_create_main_frame(main_screen);
  
  ui_create_title(main_frame, txt->vario_settings);
  
  // Container principal
  lv_obj_t *main_container = ui_create_flex_container(main_frame, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_size(main_container, 940, 460);
  lv_obj_align(main_container, LV_ALIGN_CENTER, 0, 15);
  lv_obj_set_flex_align(main_container, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
  lv_obj_set_style_pad_row(main_container, 20, 0);
  
  // 1. Periode d'integration du vario
  lv_obj_t *integration_row = ui_create_flex_container(main_container, LV_FLEX_FLOW_ROW);
  lv_obj_set_width(integration_row, lv_pct(100));
  lv_obj_set_flex_align(integration_row, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
  lv_obj_set_style_pad_column(integration_row, 20, 0);
  
  lv_obj_t *label_integration = ui_create_label(integration_row, "Periode d'integration",
                                                 &lv_font_montserrat_20, lv_color_hex(0x00d4ff));
  lv_obj_set_width(label_integration, 280);
  
  slider_integration = lv_slider_create(integration_row);
  lv_obj_set_size(slider_integration, 500, 20);
  lv_slider_set_range(slider_integration, 1, 20);
  lv_slider_set_value(slider_integration, 5, LV_ANIM_OFF);
  lv_obj_set_style_bg_color(slider_integration, lv_color_hex(0x2a3f5f), LV_PART_MAIN);
  lv_obj_set_style_bg_color(slider_integration, lv_color_hex(0x00d4ff), LV_PART_INDICATOR);
  lv_obj_set_style_bg_color(slider_integration, lv_color_hex(0x00d4ff), LV_PART_KNOB);
  lv_obj_set_style_pad_all(slider_integration, 5, LV_PART_KNOB);
  lv_obj_add_event_cb(slider_integration, slider_integration_event_cb, LV_EVENT_VALUE_CHANGED, NULL);
  
  label_integration_value = ui_create_label(integration_row, "5 s",
                                            &lv_font_montserrat_20, lv_color_white());
  lv_obj_set_width(label_integration_value, 60);
  
  // 2. Profil sonore avec graphique
  lv_obj_t *label_audio = ui_create_label(main_container, "Profil sonore (Vario m/s -> Frequence Hz)",
                                          &lv_font_montserrat_20, lv_color_hex(0x00d4ff));
  
  // Container pour le graphique avec labels (augmente de 40px)
  chart_container = lv_obj_create(main_container);
  lv_obj_set_size(chart_container, lv_pct(100), 320);
  lv_obj_set_style_bg_color(chart_container, lv_color_hex(0x1a2035), 0);
  lv_obj_set_style_bg_opa(chart_container, LV_OPA_80, 0);
  lv_obj_set_style_border_width(chart_container, 2, 0);
  lv_obj_set_style_border_color(chart_container, lv_color_hex(0x6080a0), 0);
  lv_obj_set_style_radius(chart_container, 10, 0);
  lv_obj_set_style_pad_all(chart_container, 15, 0);
  lv_obj_clear_flag(chart_container, LV_OBJ_FLAG_SCROLLABLE);
  
  // Labels axe Y (frequences) - 600, 800, 1000, 1200, 1400
  for (int i = 0; i <= 4; i++) {
    int freq = 1400 - (i * 200);
    lv_obj_t *label_y = lv_label_create(chart_container);
    lv_label_set_text_fmt(label_y, "%d", freq);
    lv_obj_set_style_text_font(label_y, &lv_font_montserrat_12, 0);
    lv_obj_set_style_text_color(label_y, lv_color_hex(0x808080), 0);
    lv_obj_set_pos(label_y, 5, 20 + (i * 60));
  }
  
  // Labels axe X (vitesse vario) - -5 a +10 (centres sous chaque point)
  for (int i = 0; i <= 15; i++) {
    int vario = i - 5;
    lv_obj_t *label_x = lv_label_create(chart_container);
    lv_label_set_text_fmt(label_x, "%d", vario);
    lv_obj_set_style_text_font(label_x, &lv_font_montserrat_12, 0);
    lv_obj_set_style_text_color(label_x, lv_color_hex(0x808080), 0);
    // Calcul position centree: debut_graph + (i * largeur_graph / 15) - largeur_label/2
    lv_obj_set_pos(label_x, 45 + 6 + (i * 870 / 15) - 6, 275);
  }
  
  // Creation du graphique (augmente de 40px)
  chart_audio = lv_chart_create(chart_container);
  lv_obj_set_size(chart_audio, 870, 250);
  lv_obj_set_pos(chart_audio, 45, 15);
  lv_chart_set_type(chart_audio, LV_CHART_TYPE_LINE);
  lv_chart_set_point_count(chart_audio, 16);
  lv_chart_set_range(chart_audio, LV_CHART_AXIS_PRIMARY_Y, 600, 1400);
  
  // Suppression des lignes de division verticales (garder seulement horizontales)
  lv_chart_set_div_line_count(chart_audio, 5, 0);
  
  // Style du graphique
  lv_obj_set_style_bg_color(chart_audio, lv_color_hex(0x0f1729), 0);
  lv_obj_set_style_border_width(chart_audio, 1, 0);
  lv_obj_set_style_border_color(chart_audio, lv_color_hex(0x4080a0), 0);
  lv_obj_set_style_line_color(chart_audio, lv_color_hex(0x404040), LV_PART_ITEMS);
  
  // Taille des points
  lv_obj_set_style_size(chart_audio, 15, 15, LV_PART_INDICATOR);
  
  // Serie de donnees en ROUGE
  series_audio = lv_chart_add_series(chart_audio, lv_color_hex(0xff0000), LV_CHART_AXIS_PRIMARY_Y);
  lv_obj_set_style_line_width(chart_audio, 3, LV_PART_ITEMS);
  
  // Couleur rouge pour les points aussi
  lv_obj_set_style_bg_color(chart_audio, lv_color_hex(0xff0000), LV_PART_INDICATOR);
  
  // Initialisation des points
  for (int i = 0; i < 16; i++) {
    lv_chart_set_value_by_id(chart_audio, series_audio, i, audio_frequencies[i]);
  }
  
  // Activation du clic sur le graphique
  lv_obj_add_flag(chart_audio, LV_OBJ_FLAG_CLICKABLE);
  lv_obj_add_event_cb(chart_audio, chart_audio_event_cb, LV_EVENT_PRESSED, NULL);
  lv_obj_add_event_cb(chart_audio, chart_audio_event_cb, LV_EVENT_PRESSING, NULL);
  
  // Boutons Enregistrer, Reinitialiser et Annuler (decales de 20px vers le bas)
  lv_obj_t *buttons_container = ui_create_flex_container(main_frame, LV_FLEX_FLOW_ROW);
  lv_obj_set_width(buttons_container, 700);
  lv_obj_align(buttons_container, LV_ALIGN_BOTTOM_MID, 0, -10);
  lv_obj_set_flex_align(buttons_container, LV_FLEX_ALIGN_SPACE_EVENLY, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
  lv_obj_set_style_pad_column(buttons_container, 15, 0);
  
  lv_obj_t *btn_reset = ui_create_simple_button(buttons_container, "Reset",
                                                 lv_color_hex(0xff9500), 220, 50);
  lv_obj_add_event_cb(btn_reset, btn_reset_audio_cb, LV_EVENT_CLICKED, NULL);
  
  lv_obj_t *btn_save = ui_create_simple_button(buttons_container, txt->save,
                                                lv_color_hex(0x34c759), 220, 50);
  lv_obj_add_event_cb(btn_save, btn_save_vario_cb, LV_EVENT_CLICKED, NULL);
  
  lv_obj_t *btn_cancel = ui_create_simple_button(buttons_container, txt->cancel,
                                                  lv_color_hex(0xff3b30), 220, 50);
  lv_obj_add_event_cb(btn_cancel, btn_cancel_vario_cb, LV_EVENT_CLICKED, NULL);
  
  load_vario_settings();
  
  lv_screen_load(main_screen);

#ifdef DEBUG_MODE
  Serial.println("Vario settings screen initialized");
#endif
}

void ui_settings_vario_show(void) {
  ui_settings_vario_init();
  
#ifdef DEBUG_MODE
  Serial.println("Vario settings screen displayed");
#endif
}

#endif