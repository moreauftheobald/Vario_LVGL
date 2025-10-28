#ifndef UI_SETTINGS_SCREEN_H
#define UI_SETTINGS_SCREEN_H

#include "lvgl.h"
#include "constants.h"
#include "UI_helper.h"
#include "lang.h"
#include "globals.h"
#include "src/gt911/gt911.h"
#include "src/params/params.h"

void ui_settings_show(void);

// Variables statiques necessaires pour l'etat de calibration
static lv_obj_t *instruction_label = NULL;
static lv_obj_t *target_obj = NULL;
static lv_obj_t *test_point = NULL;
static bool test_mode = false;

// Points de calibration
typedef struct {
  int16_t screen_x;
  int16_t screen_y;
  int16_t raw_x;
  int16_t raw_y;
} calib_point_t;

static calib_point_t calib_points[2];
static uint8_t current_point = 0;
static bool calibration_done = false;

// Parametres de calibration lineaire
static float calib_offset_x = 0.0f;
static float calib_offset_y = 0.0f;
static float calib_scale_x = 1.0f;
static float calib_scale_y = 1.0f;

static void load_calibration(void) {
  calib_offset_x = params.touch_offset_x;
  calib_offset_y = params.touch_offset_y;
  calib_scale_x = params.touch_scale_x;
  calib_scale_y = params.touch_scale_y;

#ifdef DEBUG_MODE
  Serial.printf("Calibration loaded from params: offset_x=%.3f offset_y=%.3f scale_x=%.3f scale_y=%.3f\n",
                calib_offset_x, calib_offset_y, calib_scale_x, calib_scale_y);
#endif
}

static void save_calibration(void) {
  params.touch_offset_x = calib_offset_x;
  params.touch_offset_y = calib_offset_y;
  params.touch_scale_x = calib_scale_x;
  params.touch_scale_y = calib_scale_y;

  params_save_calibration();

#ifdef DEBUG_MODE
  Serial.printf("Calibration saved to params: offset_x=%.3f offset_y=%.3f scale_x=%.3f scale_y=%.3f\n",
                calib_offset_x, calib_offset_y, calib_scale_x, calib_scale_y);
#endif
}

static void calculate_calibration(void) {
#ifdef DEBUG_MODE
  Serial.println("Calibration lineaire simple");
  Serial.printf("Point 0: ecran(%d,%d) brut(%d,%d)\n",
                calib_points[0].screen_x, calib_points[0].screen_y,
                calib_points[0].raw_x, calib_points[0].raw_y);
  Serial.printf("Point 1: ecran(%d,%d) brut(%d,%d)\n",
                calib_points[1].screen_x, calib_points[1].screen_y,
                calib_points[1].raw_x, calib_points[1].raw_y);
#endif

  int16_t delta_raw_x = calib_points[1].raw_x - calib_points[0].raw_x;
  int16_t delta_raw_y = calib_points[1].raw_y - calib_points[0].raw_y;
  int16_t delta_screen_x = calib_points[1].screen_x - calib_points[0].screen_x;
  int16_t delta_screen_y = calib_points[1].screen_y - calib_points[0].screen_y;

  if (delta_raw_x == 0) delta_raw_x = 1;
  if (delta_raw_y == 0) delta_raw_y = 1;

  calib_scale_x = (float)delta_screen_x / (float)delta_raw_x;
  calib_scale_y = (float)delta_screen_y / (float)delta_raw_y;

  calib_offset_x = calib_points[0].screen_x - (calib_points[0].raw_x * calib_scale_x);
  calib_offset_y = calib_points[0].screen_y - (calib_points[0].raw_y * calib_scale_y);

#ifdef DEBUG_MODE
  Serial.printf("Calibration calculee: offset_x=%.3f offset_y=%.3f scale_x=%.3f scale_y=%.3f\n",
                calib_offset_x, calib_offset_y, calib_scale_x, calib_scale_y);
#endif
}

static void apply_calibration(int16_t raw_x, int16_t raw_y, int16_t *calib_x, int16_t *calib_y) {
  *calib_x = (int16_t)(raw_x * calib_scale_x + calib_offset_x);
  *calib_y = (int16_t)(raw_y * calib_scale_y + calib_offset_y);
}

static void update_target_position(void) {
  if (!target_obj) return;

  calib_points[current_point].screen_x = (current_point == 0) ? 50 : (LCD_H_RES - 50);
  calib_points[current_point].screen_y = (current_point == 0) ? 50 : (LCD_V_RES - 50);

  lv_obj_set_pos(target_obj,
                 calib_points[current_point].screen_x - 50,
                 calib_points[current_point].screen_y - 50);
}

static void btn_test_calib_cb(lv_event_t *e);

static void screen_touch_cb(lv_event_t *e) {
  lv_event_code_t code = lv_event_get_code(e);

  if (code == LV_EVENT_PRESSED) {
    lv_point_t point;
    lv_indev_t *indev = lv_indev_get_act();
    if (!indev) return;

    lv_indev_get_point(indev, &point);

#ifdef DEBUG_MODE
    Serial.printf("Touch detected at: (%d, %d)\n", point.x, point.y);
#endif

    // Mode test
    if (test_mode && test_point != NULL) {
      int16_t calib_x, calib_y;
      apply_calibration(point.x, point.y, &calib_x, &calib_y);
      lv_obj_set_pos(test_point, calib_x - 10, calib_y - 10);

#ifdef DEBUG_MODE
      Serial.printf("Test: brut(%d,%d) -> calibre(%d,%d)\n",
                    point.x, point.y, calib_x, calib_y);
#endif
      return;
    }

    // Mode calibration
    if (calibration_done || current_point >= 2) return;

    int16_t target_x = calib_points[current_point].screen_x;
    int16_t target_y = calib_points[current_point].screen_y;
    int16_t distance = abs(point.x - target_x) + abs(point.y - target_y);

#ifdef DEBUG_MODE
    Serial.printf("Distance from target: %d (threshold: 150)\n", distance);
#endif

    if (distance < 150) {
      calib_points[current_point].raw_x = point.x;
      calib_points[current_point].raw_y = point.y;

#ifdef DEBUG_MODE
      Serial.printf("Point %d captured: ecran(%d,%d) brut(%d,%d)\n",
                    current_point,
                    calib_points[current_point].screen_x,
                    calib_points[current_point].screen_y,
                    calib_points[current_point].raw_x,
                    calib_points[current_point].raw_y);
#endif

      current_point++;

      if (current_point < 2) {
        update_target_position();
        char text[64];
        snprintf(text, sizeof(text), "Point %d/2 - Touchez la cible", current_point + 1);
        lv_label_set_text(instruction_label, text);
      } else {
        calculate_calibration();
        calibration_done = true;
        lv_obj_add_flag(target_obj, LV_OBJ_FLAG_HIDDEN);
        lv_label_set_text(instruction_label, "Calibration terminee!");

        lv_obj_t *btn_test = lv_btn_create(current_screen);
        lv_obj_set_size(btn_test, 120, 40);
        lv_obj_align(btn_test, LV_ALIGN_CENTER, 0, 20);
        lv_obj_t *label_test = lv_label_create(btn_test);
        lv_label_set_text(label_test, "Tester");
        lv_obj_center(label_test);
        lv_obj_add_event_cb(btn_test, btn_test_calib_cb, LV_EVENT_CLICKED, NULL);
      }
    } else {
#ifdef DEBUG_MODE
      Serial.println("Touch too far from target, ignored");
#endif
    }
  }
}

static void btn_test_calib_cb(lv_event_t *e) {
  test_mode = true;

  if (test_point == NULL) {
    test_point = lv_obj_create(current_screen);
    lv_obj_set_size(test_point, 20, 20);
    lv_obj_set_style_radius(test_point, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(test_point, lv_color_hex(0x00FF00), 0);
    lv_obj_set_style_border_width(test_point, 0, 0);
    lv_obj_set_style_pad_all(test_point, 0, 0);
    lv_obj_set_pos(test_point, 0, 0);  // Position initiale en haut à gauche
    lv_obj_clear_flag(test_point, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_clear_flag(test_point, LV_OBJ_FLAG_SCROLLABLE);
  }

  lv_label_set_text(instruction_label, "Mode test: touchez l'ecran");
  lv_obj_t *btn = (lv_obj_t *)lv_event_get_target(e);
  lv_obj_add_flag(btn, LV_OBJ_FLAG_HIDDEN);
}

static void btn_save_calib_cb(lv_event_t *e) {
  if (calibration_done) {
    save_calibration();
#ifdef DEBUG_MODE
    Serial.println("Calibration sauvegardee");
#endif
  }
  test_point = NULL;
  test_mode = false;
  ui_settings_show();
}

static void btn_cancel_calib_cb(lv_event_t *e) {
#ifdef DEBUG_MODE
  Serial.println("Calibration annulee");
#endif
  test_point = NULL;
  test_mode = false;
  ui_settings_show();
}

void ui_settings_screen_init(void) {
  const TextStrings *txt = get_text();

  current_point = 0;
  calibration_done = false;
  test_mode = false;
  test_point = NULL;

  load_calibration();

  current_screen = ui_create_screen();
  lv_obj_clear_flag(current_screen, LV_OBJ_FLAG_SCROLLABLE);  // IMPORTANT: désactiver scroll
  lv_obj_add_event_cb(current_screen, screen_touch_cb, LV_EVENT_PRESSED, NULL);

  // Instructions (directement sur l'écran principal)
  instruction_label = lv_label_create(current_screen);
  lv_label_set_text(instruction_label, "Point 1/2 - Touchez la cible");
  lv_obj_set_style_text_font(instruction_label, &lv_font_montserrat_24, 0);
  lv_obj_set_style_text_color(instruction_label, lv_color_hex(0xFFFFFF), 0);
  lv_obj_set_style_text_align(instruction_label, LV_TEXT_ALIGN_CENTER, 0);
  lv_obj_align(instruction_label, LV_ALIGN_TOP_MID, 0, 20);
  lv_obj_clear_flag(instruction_label, LV_OBJ_FLAG_CLICKABLE);

  // Cible (cercle + point central)
  target_obj = lv_obj_create(current_screen);
  lv_obj_set_size(target_obj, 100, 100);
  lv_obj_set_style_bg_opa(target_obj, LV_OPA_TRANSP, 0);
  lv_obj_set_style_border_width(target_obj, 0, 0);
  lv_obj_set_style_pad_all(target_obj, 0, 0);
  lv_obj_clear_flag(target_obj, LV_OBJ_FLAG_CLICKABLE);
  lv_obj_clear_flag(target_obj, LV_OBJ_FLAG_SCROLLABLE);

  lv_obj_t *circle = lv_obj_create(target_obj);
  lv_obj_set_size(circle, 100, 100);
  lv_obj_set_style_radius(circle, LV_RADIUS_CIRCLE, 0);
  lv_obj_set_style_bg_opa(circle, LV_OPA_TRANSP, 0);
  lv_obj_set_style_border_color(circle, lv_color_hex(0xFF0000), 0);
  lv_obj_set_style_border_width(circle, 3, 0);
  lv_obj_set_style_pad_all(circle, 0, 0);
  lv_obj_center(circle);
  lv_obj_clear_flag(circle, LV_OBJ_FLAG_CLICKABLE);
  lv_obj_clear_flag(circle, LV_OBJ_FLAG_SCROLLABLE);

  lv_obj_t *center_point = lv_obj_create(target_obj);
  lv_obj_set_size(center_point, 10, 10);
  lv_obj_set_style_radius(center_point, LV_RADIUS_CIRCLE, 0);
  lv_obj_set_style_bg_color(center_point, lv_color_hex(0xFFFF00), 0);
  lv_obj_set_style_border_width(center_point, 0, 0);
  lv_obj_set_style_pad_all(center_point, 0, 0);
  lv_obj_center(center_point);
  lv_obj_clear_flag(center_point, LV_OBJ_FLAG_CLICKABLE);
  lv_obj_clear_flag(center_point, LV_OBJ_FLAG_SCROLLABLE);

  update_target_position();

  ui_button_pair_t buttons = ui_create_save_cancel_buttons(current_screen, txt->save, txt->cancel, nullptr, true, true, false, btn_save_calib_cb, btn_cancel_calib_cb, nullptr, NULL, NULL, NULL);

#ifdef DEBUG_MODE
  Serial.println("Screen calibration initialized");
  Serial.printf("Target position: (%d, %d)\n",
                calib_points[0].screen_x, calib_points[0].screen_y);
#endif
}

void get_touch_calibration(float *offset_x, float *offset_y, float *scale_x, float *scale_y) {
  if (offset_x) *offset_x = calib_offset_x;
  if (offset_y) *offset_y = calib_offset_y;
  if (scale_x) *scale_x = calib_scale_x;
  if (scale_y) *scale_y = calib_scale_y;
}

void ui_settings_screen_show(void) {
  // Sauvegarder ancien écran
  lv_obj_t *old_screen = lv_scr_act();

  if (lvgl_port_lock(-1)) {
    // Créer nouvel écran
    current_screen = lv_obj_create(NULL);
    ui_settings_screen_init();
    lv_screen_load(current_screen);
    force_full_refresh();
    lvgl_port_unlock();
  }

  // Détruire ancien écran
  if (old_screen != current_screen && old_screen != NULL) {
    lv_obj_del(old_screen);
#ifdef DEBUG_MODE
    Serial.println("[UI] Old screen deleted");
#endif
  }
}

#endif