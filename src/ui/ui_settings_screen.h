#ifndef UI_SETTINGS_SCREEN_H
#define UI_SETTINGS_SCREEN_H

#include "lvgl.h"
#include "constants.h"
#include "UI_helper.h"
#include "lang.h"
#include "globals.h"
#include "src/gt911/gt911.h"

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

// Load/Save calibration
static void load_calibration(void) {
  prefs.begin("touch_calib", true);
  calib_offset_x = prefs.getFloat("offset_x", 0.0f);
  calib_offset_y = prefs.getFloat("offset_y", 0.0f);
  calib_scale_x = prefs.getFloat("scale_x", 1.0f);
  calib_scale_y = prefs.getFloat("scale_y", 1.0f);
  prefs.end();

#ifdef DEBUG_MODE
  Serial.printf("Calibration loaded: offset_x=%.3f offset_y=%.3f scale_x=%.3f scale_y=%.3f\n",
                calib_offset_x, calib_offset_y, calib_scale_x, calib_scale_y);
#endif
}

static void save_calibration(void) {
  prefs.begin("touch_calib", false);
  prefs.putFloat("offset_x", calib_offset_x);
  prefs.putFloat("offset_y", calib_offset_y);
  prefs.putFloat("scale_x", calib_scale_x);
  prefs.putFloat("scale_y", calib_scale_y);
  prefs.end();

#ifdef DEBUG_MODE
  Serial.printf("Calibration saved: offset_x=%.3f offset_y=%.3f scale_x=%.3f scale_y=%.3f\n",
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
  
  calib_points[current_point].screen_x = (current_point == 0) ? 50 : (SCREEN_WIDTH - 50);
  calib_points[current_point].screen_y = (current_point == 0) ? 50 : (SCREEN_HEIGHT - 50);
  
  lv_obj_set_pos(target_obj, 
                 calib_points[current_point].screen_x - 50,
                 calib_points[current_point].screen_y - 50);
}

static void btn_test_calib_cb(lv_event_t *e);

static void screen_touch_cb(lv_event_t *e) {
  lv_event_code_t code = lv_event_get_code(e);
  
  if (code == LV_EVENT_PRESSED || code == LV_EVENT_PRESSING) {
    lv_point_t point;
    lv_indev_t *indev = lv_indev_get_act();
    lv_indev_get_point(indev, &point);
    
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
      } else {
        calculate_calibration();
        calibration_done = true;
        lv_obj_add_flag(target_obj, LV_OBJ_FLAG_HIDDEN);
        lv_label_set_text(instruction_label, "Calibration terminee!");
        
        lv_obj_t *btn_test = lv_btn_create(main_screen);
        lv_obj_set_size(btn_test, 120, 40);
        lv_obj_align(btn_test, LV_ALIGN_CENTER, 0, 20);
        lv_obj_t *label_test = lv_label_create(btn_test);
        lv_label_set_text(label_test, "Tester");
        lv_obj_center(label_test);
        lv_obj_add_event_cb(btn_test, btn_test_calib_cb, LV_EVENT_CLICKED, NULL);
      }
    }
  }
}

static void btn_test_calib_cb(lv_event_t *e) {
  test_mode = true;
  
  if (test_point == NULL) {
    test_point = lv_obj_create(main_screen);
    lv_obj_set_size(test_point, 20, 20);
    lv_obj_set_style_radius(test_point, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(test_point, lv_color_hex(0x00FF00), 0);
    lv_obj_set_style_border_width(test_point, 0, 0);
    lv_obj_center(test_point);
  }
  
  lv_label_set_text(instruction_label, "Mode test: touchez l'ecran");
  lv_obj_t *btn = (lv_obj_t*)lv_event_get_target(e);
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
  
  // Nettoyer l'ecran s'il existe
  if (main_screen != NULL) {
    lv_obj_clean(main_screen);
  } else {
    main_screen = lv_obj_create(NULL);
  }
  
  lv_obj_set_style_bg_color(main_screen, lv_color_hex(0x0a0e27), 0);
  lv_obj_add_event_cb(main_screen, screen_touch_cb, LV_EVENT_PRESSED, NULL);
  lv_obj_add_event_cb(main_screen, screen_touch_cb, LV_EVENT_PRESSING, NULL);
  
  // Frame transparent
  lv_obj_t *main_frame = lv_obj_create(main_screen);
  lv_obj_set_size(main_frame, SCREEN_WIDTH, SCREEN_HEIGHT);
  lv_obj_center(main_frame);
  lv_obj_set_style_bg_opa(main_frame, LV_OPA_TRANSP, 0);
  lv_obj_set_style_border_width(main_frame, 0, 0);
  lv_obj_clear_flag(main_frame, LV_OBJ_FLAG_CLICKABLE);
  lv_obj_clear_flag(main_frame, LV_OBJ_FLAG_SCROLLABLE);
  
  // Instructions
  instruction_label = lv_label_create(main_frame);
  lv_label_set_text(instruction_label, "Touchez les cibles qui apparaissent");
  lv_obj_set_style_text_font(instruction_label, &lv_font_montserrat_24, 0);
  lv_obj_set_style_text_color(instruction_label, lv_color_hex(0xFFFFFF), 0);
  lv_obj_set_style_text_align(instruction_label, LV_TEXT_ALIGN_CENTER, 0);
  lv_obj_align(instruction_label, LV_ALIGN_TOP_MID, 0, 20);
  
  // Cible (cercle + point central)
  target_obj = lv_obj_create(main_frame);
  lv_obj_set_size(target_obj, 100, 100);
  lv_obj_set_style_bg_opa(target_obj, LV_OPA_TRANSP, 0);
  lv_obj_set_style_border_width(target_obj, 0, 0);
  lv_obj_clear_flag(target_obj, LV_OBJ_FLAG_CLICKABLE);
  lv_obj_clear_flag(target_obj, LV_OBJ_FLAG_SCROLLABLE);
  
  lv_obj_t *circle = lv_obj_create(target_obj);
  lv_obj_set_size(circle, 100, 100);
  lv_obj_set_style_radius(circle, LV_RADIUS_CIRCLE, 0);
  lv_obj_set_style_bg_opa(circle, LV_OPA_TRANSP, 0);
  lv_obj_set_style_border_color(circle, lv_color_hex(0xFF0000), 0);
  lv_obj_set_style_border_width(circle, 3, 0);
  lv_obj_center(circle);
  lv_obj_clear_flag(circle, LV_OBJ_FLAG_CLICKABLE);
  
  lv_obj_t *center_point = lv_obj_create(target_obj);
  lv_obj_set_size(center_point, 10, 10);
  lv_obj_set_style_radius(center_point, LV_RADIUS_CIRCLE, 0);
  lv_obj_set_style_bg_color(center_point, lv_color_hex(0xFFFF00), 0);
  lv_obj_set_style_border_width(center_point, 0, 0);
  lv_obj_center(center_point);
  lv_obj_clear_flag(center_point, LV_OBJ_FLAG_CLICKABLE);
  
  update_target_position();
  
  // Bouton Enregistrer
  lv_obj_t *btn_save = ui_create_simple_button(main_frame, "Enregistrer", 
                                                 lv_color_hex(0x34c759), 140, 50);
  lv_obj_align(btn_save, LV_ALIGN_BOTTOM_LEFT, 100, -30);
  lv_obj_add_event_cb(btn_save, btn_save_calib_cb, LV_EVENT_CLICKED, NULL);
  
  // Bouton Annuler
  lv_obj_t *btn_cancel = ui_create_simple_button(main_frame, "Annuler", 
                                                   lv_color_hex(0xff3b30), 140, 50);
  lv_obj_align(btn_cancel, LV_ALIGN_BOTTOM_RIGHT, -100, -30);
  lv_obj_add_event_cb(btn_cancel, btn_cancel_calib_cb, LV_EVENT_CLICKED, NULL);
  
  lv_screen_load(main_screen);

#ifdef DEBUG_MODE
  Serial.println("Screen calibration initialized");
#endif
}

void get_touch_calibration(float *offset_x, float *offset_y, float *scale_x, float *scale_y) {
  if (offset_x) *offset_x = calib_offset_x;
  if (offset_y) *offset_y = calib_offset_y;
  if (scale_x) *scale_x = calib_scale_x;
  if (scale_y) *scale_y = calib_scale_y;
}

void ui_settings_screen_show(void) {
  ui_settings_screen_init();

#ifdef DEBUG_MODE
  Serial.println("Screen calibration shown");
#endif
}

#endif