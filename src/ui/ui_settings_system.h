#ifndef UI_SETTINGS_SYSTEM_H
#define UI_SETTINGS_SYSTEM_H

#include "lvgl.h"
#include "constants.h"
#include "UI_helper.h"
#include "lang.h"
#include "globals.h"
#include "src/params/params.h"
#include "src/rgb_lcd_port/rgb_lcd_port.h"

void ui_settings_show(void);

// Variables pour les widgets
static lv_obj_t *slider_brightness = NULL;
static lv_obj_t *label_brightness_value = NULL;
static lv_obj_t *dropdown_language = NULL;

static void load_system_settings(void) {
  if (slider_brightness) {
    lv_slider_set_value(slider_brightness, params.system_brightness, LV_ANIM_OFF);
    lv_label_set_text_fmt(label_brightness_value, "%d%%", params.system_brightness);
  }
  
  if (dropdown_language) {
    lv_dropdown_set_selected(dropdown_language, (int)params.system_language);
  }
  
#ifdef DEBUG_MODE
  Serial.printf("System settings loaded from params: brightness=%d%%, language=%d\n", 
                params.system_brightness, (int)params.system_language);
#endif
}

static void save_system_settings(void) {
  params.system_brightness = (int)lv_slider_get_value(slider_brightness);
  params.system_language = (Language)lv_dropdown_get_selected(dropdown_language);
  
  params_save_system();
  
  // Appliquer la luminosite
  wavesahre_rgb_lcd_set_brightness(params.system_brightness);
  
#ifdef DEBUG_MODE
  Serial.printf("System settings saved: brightness=%d%%, language=%d\n", 
                params.system_brightness, (int)params.system_language);
#endif
}

static void slider_brightness_event_cb(lv_event_t *e) {
  lv_event_code_t code = lv_event_get_code(e);
  if (code == LV_EVENT_VALUE_CHANGED) {
    int value = (int)lv_slider_get_value(slider_brightness);
    lv_label_set_text_fmt(label_brightness_value, "%d%%", value);
    
    // Appliquer en temps reel
    wavesahre_rgb_lcd_set_brightness(value);

#ifdef DEBUG_MODE
    Serial.printf("Brightness changed to: %d%%\n", value);
#endif
  }
}

static void dropdown_language_event_cb(lv_event_t *e) {
  lv_event_code_t code = lv_event_get_code(e);
  if (code == LV_EVENT_VALUE_CHANGED) {
    int lang = lv_dropdown_get_selected(dropdown_language);
    
#ifdef DEBUG_MODE
    Serial.printf("Language changed to: %d\n", lang);
#endif
  }
}

static void btn_save_system_cb(lv_event_t *e) {
#ifdef DEBUG_MODE
  Serial.println("Save system settings clicked");
#endif
  save_system_settings();
  ui_settings_show();
}

static void btn_cancel_system_cb(lv_event_t *e) {
#ifdef DEBUG_MODE
  Serial.println("Cancel system settings clicked");
#endif
  ui_settings_show();
}

void ui_settings_system_init(void) {
  const TextStrings *txt = get_text();

  lv_obj_t *main_frame = ui_create_black_screen_with_frame(3, ROUND_FRANE_RADUIS_BIG, &main_screen);

  // Titre
  lv_obj_t *label_title = ui_create_label(main_frame, txt->system_settings,
                                           &lv_font_montserrat_32, lv_color_hex(0x00d4ff));
  lv_obj_align(label_title, LV_ALIGN_TOP_MID, 0, 0);

  // Container principal
  lv_obj_t *main_container = ui_create_form_column(main_frame, 980);
  lv_obj_set_height(main_container, 320);
  lv_obj_align(main_container, LV_ALIGN_TOP_MID, 0, 50);

  // === LUMINOSITE ===
  lv_obj_t *label_brightness = ui_create_label(main_container, "Luminosite ecran",
                                                &lv_font_montserrat_24, lv_color_hex(0x00d4ff));
  
  // Container pour slider + valeur
  lv_obj_t *brightness_row = ui_create_flex_container(main_container, LV_FLEX_FLOW_ROW);
  lv_obj_set_size(brightness_row, lv_pct(100), LV_SIZE_CONTENT);
  lv_obj_set_flex_align(brightness_row, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
  
  slider_brightness = lv_slider_create(brightness_row);
  lv_slider_set_range(slider_brightness, 10, 100);
  lv_slider_set_value(slider_brightness, 80, LV_ANIM_OFF);
  lv_obj_set_size(slider_brightness, 800, 20);
  lv_obj_set_style_bg_color(slider_brightness, lv_color_hex(0x4080a0), LV_PART_INDICATOR);
  lv_obj_set_style_bg_color(slider_brightness, lv_color_hex(0x2a3f5f), LV_PART_MAIN);
  lv_obj_add_event_cb(slider_brightness, slider_brightness_event_cb, LV_EVENT_VALUE_CHANGED, NULL);
  
  label_brightness_value = ui_create_label(brightness_row, "80%",
                                            &lv_font_montserrat_24, lv_color_hex(0xFFFFFF));
  lv_obj_set_width(label_brightness_value, 80);
  lv_obj_set_style_text_align(label_brightness_value, LV_TEXT_ALIGN_RIGHT, 0);

  // Separateur
  ui_create_separator(main_container);

  // === LANGUE ===
  lv_obj_t *label_language = ui_create_label(main_container, "Langue / Language",
                                              &lv_font_montserrat_24, lv_color_hex(0x00d4ff));
  
  dropdown_language = lv_dropdown_create(main_container);
  lv_dropdown_set_options(dropdown_language, "Francais\nEnglish");
  lv_obj_set_size(dropdown_language, lv_pct(100), 50);
  lv_obj_set_style_bg_color(dropdown_language, lv_color_hex(0x0f1520), 0);
  lv_obj_set_style_border_color(dropdown_language, lv_color_hex(0x4080a0), 0);
  lv_obj_set_style_border_width(dropdown_language, 2, 0);
  lv_obj_set_style_text_font(dropdown_language, &lv_font_montserrat_20, 0);
  lv_obj_set_style_text_color(dropdown_language, lv_color_white(), 0);
  lv_obj_add_event_cb(dropdown_language, dropdown_language_event_cb, LV_EVENT_VALUE_CHANGED, NULL);

  // Separateur
  ui_create_separator(main_container);

  // Note d'information
  lv_obj_t *note = ui_create_label(main_container, 
                                    "Note: Le changement de langue\nprend effet au redemarrage",
                                    &lv_font_montserrat_16, lv_color_hex(0x808080));
  lv_obj_set_style_text_align(note, LV_TEXT_ALIGN_CENTER, 0);

  // Boutons Save/Cancel
  ui_button_pair_t buttons = ui_create_save_cancel_buttons(main_frame, txt->save, txt->cancel, 
                                                            nullptr, true, true, false,
                                                            btn_save_system_cb, btn_cancel_system_cb, nullptr, NULL, NULL, NULL);

  // Charger les valeurs sauvegardees
  load_system_settings();

  lv_screen_load(main_screen);

#ifdef DEBUG_MODE
  Serial.println("System settings screen initialized");
#endif
}

void ui_settings_system_show(void) {
  ui_settings_system_init();

#ifdef DEBUG_MODE
  Serial.println("System settings screen displayed");
#endif
}

#endif