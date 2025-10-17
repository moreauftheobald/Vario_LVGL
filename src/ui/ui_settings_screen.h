#ifndef UI_SETTINGS_SCREEN_H
#define UI_SETTINGS_SCREEN_H

#include "lvgl.h"
#include "constants.h"
#include "globals.h"
#include "src/gt911/gt911.h"

// Declaration anticipee des fonctions
static void screen_touch_cb(lv_event_t *e);
static void btn_save_calib_cb(lv_event_t *e);
static void btn_cancel_calib_cb(lv_event_t *e);
static void btn_test_calib_cb(lv_event_t *e);
static void update_target_position(void);
static void calculate_calibration(void);
static void apply_calibration(int16_t raw_x, int16_t raw_y, int16_t *calib_x, int16_t *calib_y);
void ui_settings_screen_init(void);
void ui_settings_screen_show(void);
void get_touch_calibration(float *offset_x, float *offset_y, float *scale_x, float *scale_y);

static lv_obj_t *screen_calibration = NULL;
static lv_obj_t *instruction_label = NULL;
static lv_obj_t *target_obj = NULL;
static lv_obj_t *test_point = NULL;
static bool test_mode = false;

// Points de calibration (on utilise seulement 2 points: haut-gauche et bas-droit)
typedef struct {
  int16_t screen_x;
  int16_t screen_y;
  int16_t raw_x;
  int16_t raw_y;
} calib_point_t;

static calib_point_t calib_points[2];
static uint8_t current_point = 0;
static bool calibration_done = false;

// Parametres de calibration lineaire simple
static float calib_offset_x = 0.0f;
static float calib_offset_y = 0.0f;
static float calib_scale_x = 1.0f;
static float calib_scale_y = 1.0f;

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

  // Calcul des deltas
  int16_t delta_raw_x = calib_points[1].raw_x - calib_points[0].raw_x;
  int16_t delta_raw_y = calib_points[1].raw_y - calib_points[0].raw_y;
  int16_t delta_screen_x = calib_points[1].screen_x - calib_points[0].screen_x;
  int16_t delta_screen_y = calib_points[1].screen_y - calib_points[0].screen_y;

  // Protection division par zero
  if (delta_raw_x == 0) delta_raw_x = 1;
  if (delta_raw_y == 0) delta_raw_y = 1;

  // Calcul scale
  calib_scale_x = (float)delta_screen_x / (float)delta_raw_x;
  calib_scale_y = (float)delta_screen_y / (float)delta_raw_y;

  // Calcul offset
  calib_offset_x = calib_points[0].screen_x - (calib_points[0].raw_x * calib_scale_x);
  calib_offset_y = calib_points[0].screen_y - (calib_points[0].raw_y * calib_scale_y);

#ifdef DEBUG_MODE
  Serial.printf("Calibration calculee: offset_x=%.3f offset_y=%.3f scale_x=%.3f scale_y=%.3f\n",
                calib_offset_x, calib_offset_y, calib_scale_x, calib_scale_y);
  
  // Verification
  for (int i = 0; i < 2; i++) {
    int16_t calc_x = (int16_t)(calib_points[i].raw_x * calib_scale_x + calib_offset_x);
    int16_t calc_y = (int16_t)(calib_points[i].raw_y * calib_scale_y + calib_offset_y);
    Serial.printf("Point %d: attendu(%d,%d) calcule(%d,%d) erreur(%d,%d)\n", 
                  i, calib_points[i].screen_x, calib_points[i].screen_y,
                  calc_x, calc_y,
                  calc_x - calib_points[i].screen_x, calc_y - calib_points[i].screen_y);
  }
#endif
}

// Fonction pour appliquer la calibration aux coordonnees brutes
static void apply_calibration(int16_t raw_x, int16_t raw_y, int16_t *calib_x, int16_t *calib_y) {
  *calib_x = (int16_t)(raw_x * calib_scale_x + calib_offset_x);
  *calib_y = (int16_t)(raw_y * calib_scale_y + calib_offset_y);
  
  // Limites ecran
  if (*calib_x < 0) *calib_x = 0;
  if (*calib_x >= SCREEN_WIDTH) *calib_x = SCREEN_WIDTH - 1;
  if (*calib_y < 0) *calib_y = 0;
  if (*calib_y >= SCREEN_HEIGHT) *calib_y = SCREEN_HEIGHT - 1;
}

static void update_target_position(void) {
  int16_t x = 30, y = 30;
  
  switch(current_point) {
    case 0:
      x = 30; 
      y = 30;
      calib_points[0].screen_x = x;
      calib_points[0].screen_y = y;
      lv_label_set_text(instruction_label, "Touchez la cible en haut a gauche");
      break;
    case 1:
      x = SCREEN_WIDTH - 30; 
      y = SCREEN_HEIGHT - 30;
      calib_points[1].screen_x = x;
      calib_points[1].screen_y = y;
      lv_label_set_text(instruction_label, "Touchez la cible en bas a droite");
      break;
    default:
      return;
  }
  
  lv_obj_set_pos(target_obj, x - 40, y - 40);
}

static void btn_test_calib_cb(lv_event_t *e) {
  test_mode = true;
  
  if (test_point == NULL) {
    test_point = lv_obj_create(screen_calibration);
    lv_obj_set_size(test_point, 20, 20);
    lv_obj_set_style_radius(test_point, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(test_point, lv_color_hex(0x00FF00), 0);
    lv_obj_set_style_border_color(test_point, lv_color_hex(0xFF0000), 0);
    lv_obj_set_style_border_width(test_point, 2, 0);
    lv_obj_clear_flag(test_point, LV_OBJ_FLAG_CLICKABLE);
  }
  
  lv_obj_clear_flag(test_point, LV_OBJ_FLAG_HIDDEN);
  lv_label_set_text(instruction_label, "Mode test: touchez l'ecran");
  
  lv_obj_t *btn = (lv_obj_t*)lv_event_get_target(e);
  lv_obj_add_flag(btn, LV_OBJ_FLAG_HIDDEN);
}

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
    
#ifdef DEBUG_MODE
    Serial.printf("Touch at (%d,%d), target at (%d,%d), distance=%d\n", 
                  point.x, point.y, target_x, target_y, distance);
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
      } else {
        calculate_calibration();
        calibration_done = true;
        lv_obj_add_flag(target_obj, LV_OBJ_FLAG_HIDDEN);
        lv_label_set_text(instruction_label, "Calibration terminee!");
        
        lv_obj_t *btn_test = lv_btn_create(screen_calibration);
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

static void btn_save_calib_cb(lv_event_t *e) {
  if (calibration_done) {
    save_calibration();
#ifdef DEBUG_MODE
    Serial.println("Calibration sauvegardee");
#endif
  }
  screen_calibration = NULL;
  test_point = NULL;
  test_mode = false;
  ui_settings_show();
}

static void btn_cancel_calib_cb(lv_event_t *e) {
#ifdef DEBUG_MODE
  Serial.println("Calibration annulee");
#endif
  screen_calibration = NULL;
  test_point = NULL;
  test_mode = false;
  ui_settings_show();
}

void ui_settings_screen_init(void) {
  current_point = 0;
  calibration_done = false;
  test_mode = false;
  test_point = NULL;
  
  load_calibration();
  
  screen_calibration = lv_obj_create(NULL);
  lv_obj_set_style_bg_color(screen_calibration, lv_color_hex(0x0a0e27), 0);
  lv_obj_add_event_cb(screen_calibration, screen_touch_cb, LV_EVENT_PRESSED, NULL);
  lv_obj_add_event_cb(screen_calibration, screen_touch_cb, LV_EVENT_PRESSING, NULL);
  
  lv_obj_t *main_frame = lv_obj_create(screen_calibration);
  lv_obj_set_size(main_frame, SCREEN_WIDTH, SCREEN_HEIGHT);
  lv_obj_center(main_frame);
  lv_obj_set_style_bg_opa(main_frame, LV_OPA_TRANSP, 0);
  lv_obj_set_style_border_width(main_frame, 0, 0);
  lv_obj_clear_flag(main_frame, LV_OBJ_FLAG_CLICKABLE);
  
  instruction_label = lv_label_create(main_frame);
  lv_label_set_text(instruction_label, "Touchez la cible en haut a gauche");
  lv_obj_set_style_text_align(instruction_label, LV_TEXT_ALIGN_CENTER, 0);
  lv_obj_set_style_text_color(instruction_label, lv_color_hex(0xFFFFFF), 0);
  lv_obj_set_style_text_font(instruction_label, &lv_font_montserrat_24, 0);
  lv_obj_align(instruction_label, LV_ALIGN_CENTER, 0, -150);
  
  target_obj = lv_obj_create(screen_calibration);
  lv_obj_set_size(target_obj, 80, 80);
  lv_obj_set_style_bg_color(target_obj, lv_color_hex(0x202040), 0);
  lv_obj_set_style_bg_opa(target_obj, LV_OPA_50, 0);
  lv_obj_set_style_border_width(target_obj, 0, 0);
  lv_obj_clear_flag(target_obj, LV_OBJ_FLAG_CLICKABLE);
  
  lv_obj_t *cross_h = lv_obj_create(target_obj);
  lv_obj_set_size(cross_h, 80, 3);
  lv_obj_set_style_bg_color(cross_h, lv_color_hex(0xFF0000), 0);
  lv_obj_set_style_border_width(cross_h, 0, 0);
  lv_obj_center(cross_h);
  lv_obj_clear_flag(cross_h, LV_OBJ_FLAG_CLICKABLE);
  
  lv_obj_t *cross_v = lv_obj_create(target_obj);
  lv_obj_set_size(cross_v, 3, 80);
  lv_obj_set_style_bg_color(cross_v, lv_color_hex(0xFF0000), 0);
  lv_obj_set_style_border_width(cross_v, 0, 0);
  lv_obj_center(cross_v);
  lv_obj_clear_flag(cross_v, LV_OBJ_FLAG_CLICKABLE);
  
  lv_obj_t *circle = lv_obj_create(target_obj);
  lv_obj_set_size(circle, 50, 50);
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
  
  lv_obj_t *btn_save = lv_btn_create(main_frame);
  lv_obj_set_size(btn_save, 140, 50);
  lv_obj_align(btn_save, LV_ALIGN_BOTTOM_LEFT, 100, -30);
  lv_obj_t *label_save = lv_label_create(btn_save);
  lv_label_set_text(label_save, "Enregistrer");
  lv_obj_center(label_save);
  lv_obj_add_event_cb(btn_save, btn_save_calib_cb, LV_EVENT_CLICKED, NULL);
  
  lv_obj_t *btn_cancel = lv_btn_create(main_frame);
  lv_obj_set_size(btn_cancel, 140, 50);
  lv_obj_align(btn_cancel, LV_ALIGN_BOTTOM_RIGHT, -100, -30);
  lv_obj_t *label_cancel = lv_label_create(btn_cancel);
  lv_label_set_text(label_cancel, "Annuler");
  lv_obj_center(label_cancel);
  lv_obj_add_event_cb(btn_cancel, btn_cancel_calib_cb, LV_EVENT_CLICKED, NULL);
  
  lv_scr_load(screen_calibration);
}

void get_touch_calibration(float *offset_x, float *offset_y, float *scale_x, float *scale_y) {
  if (offset_x) *offset_x = calib_offset_x;
  if (offset_y) *offset_y = calib_offset_y;
  if (scale_x) *scale_x = calib_scale_x;
  if (scale_y) *scale_y = calib_scale_y;
}

void ui_settings_screen_show(void) {
  if (screen_calibration == NULL) {
    ui_settings_screen_init();
  }
  lv_scr_load(screen_calibration);
}

#endif