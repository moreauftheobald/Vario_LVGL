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
  ui_load_slider_with_label(slider_brightness, label_brightness_value, params.system_brightness, "%d%%");
  ui_load_dropdown(dropdown_language, (int)params.system_language);

#ifdef DEBUG_MODE
  Serial.printf("System settings loaded from params: brightness=%d%%, language=%d\n",
                params.system_brightness, (int)params.system_language);
#endif
}

static void save_system_settings(void) {
  params.system_brightness = ui_save_slider(slider_brightness);
  params.system_language = (Language)ui_save_dropdown(dropdown_language);

  params_save_system();
  wavesahre_rgb_lcd_set_brightness(params.system_brightness);
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

  lv_obj_t *main_frame = ui_create_black_screen_with_frame(UI_BORDER_MEDIUM, UI_RADIUS_LARGE, &current_screen);
  ui_create_main_frame(main_frame, false, txt->system_settings);

  // === LUMINOSITE ===
  lv_obj_t *label_brightness = ui_create_label(main_left, "Luminosite ecran",
                                               UI_FONT_LARGE, lv_color_hex(UI_COLOR_PRIMARY));

  // Container pour slider + valeur
  lv_obj_t *brightness_row = ui_create_flex_container(main_left, LV_FLEX_FLOW_ROW);
  lv_obj_set_size(brightness_row, lv_pct(100), LV_SIZE_CONTENT);
  lv_obj_set_flex_align(brightness_row, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

  slider_brightness = ui_create_slider_with_label(main_left, lv_pct(90), LV_SIZE_CONTENT,
                                                  1, 100, params.system_brightness, "%d%%", UI_FONT_NORMAL,
                                                  lv_color_hex(UI_COLOR_TEXT_PRIMARY), &label_brightness_value);
  lv_obj_add_event_cb(slider_brightness, slider_brightness_event_cb, LV_EVENT_VALUE_CHANGED, label_brightness_value);

  // Separateur
  ui_create_separator(main_left);

  // === LANGUE ===
  lv_obj_t *label_language = ui_create_label(main_left, "Langue / Language",
                                             UI_FONT_LARGE, lv_color_hex(UI_COLOR_PRIMARY));

  dropdown_language = lv_dropdown_create(main_left);
  lv_dropdown_set_options(dropdown_language, "Francais\nEnglish");
  lv_obj_set_size(dropdown_language, lv_pct(100), 50);
  lv_obj_set_style_bg_color(dropdown_language, lv_color_hex(UI_COLOR_CONTROL_BG), 0);
  lv_obj_set_style_border_color(dropdown_language, lv_color_hex(0x4080a0), 0);
  lv_obj_set_style_border_width(dropdown_language, UI_BORDER_THIN, 0);
  lv_obj_set_style_text_font(dropdown_language, UI_FONT_NORMAL, 0);
  lv_obj_set_style_text_color(dropdown_language, lv_color_white(), 0);

  // Separateur
  ui_create_separator(main_left);

  // Note d'information
  lv_obj_t *note = ui_create_label(main_left,
                                   "Note: Le changement de langue\nprend effet au redemarrage",
                                   &lv_font_montserrat_16, lv_color_hex(UI_COLOR_TEXT_SECONDARY));
  lv_obj_set_style_text_align(note, LV_TEXT_ALIGN_CENTER, 0);

  // Bouton Save
  lv_obj_t *btn_save_pilot = ui_create_button(btn_container, txt->save, LV_SYMBOL_SAVE, lv_color_hex(UI_COLOR_BTN_START),
                                              UI_BTN_PRESTART_W, UI_BTN_PRESTART_H, UI_FONT_SMALL, UI_FONT_NORMAL, btn_save_system_cb,
                                              NULL, (lv_align_t)0, NULL, NULL);

  // Bouton Cancel
  lv_obj_t *btn_cancel_pilot = ui_create_button(btn_container, txt->cancel, LV_SYMBOL_BACKSPACE, lv_color_hex(UI_COLOR_BTN_CANCEL),
                                                UI_BTN_PRESTART_W, UI_BTN_PRESTART_H, UI_FONT_SMALL, UI_FONT_NORMAL, btn_cancel_system_cb,
                                                NULL, (lv_align_t)0, NULL, NULL);


  // Charger les valeurs sauvegardees
  load_system_settings();

#ifdef DEBUG_MODE
  Serial.println("System settings screen initialized");
#endif
}

void ui_settings_system_show(void) {
  ui_switch_screen(ui_settings_system_init);
}

#endif