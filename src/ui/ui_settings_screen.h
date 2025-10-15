#ifndef UI_SETTINGS_SCREEN_H
#define UI_SETTINGS_SCREEN_H

#include "lvgl.h"
#include "constants.h"
#include "globals.h"
#include "src/gt911/gt911.h"

void ui_settings_show(void);

static lv_obj_t *screen_calibration = NULL;
static lv_obj_t *instruction_label = NULL;
static lv_obj_t *target_obj = NULL;

// Points de calibration
typedef struct {
  int16_t screen_x;
  int16_t screen_y;
  int16_t raw_x;
  int16_t raw_y;
} calib_point_t;

static calib_point_t calib_points[4];
static uint8_t current_point = 0;
static bool calibration_done = false;

// Parametres de calibration
static int32_t offset_x = 0;
static int32_t offset_y = 0;
static int32_t scale_x = 1000;
static int32_t scale_y = 1000;

static void load_calibration(void) {
  prefs.begin("screen", true);
  offset_x = prefs.getInt("offset_x", 0);
  offset_y = prefs.getInt("offset_y", 0);
  scale_x = prefs.getInt("scale_x", 1000);
  scale_y = prefs.getInt("scale_y", 1000);
  prefs.end();

#ifdef DEBUG_MODE
  Serial.printf("Calibration loaded: offset(%d,%d) scale(%d,%d)\n",
                offset_x, offset_y, scale_x, scale_y);
#endif
}

static void save_calibration(void) {
  prefs.begin("screen", false);
  prefs.putInt("offset_x", offset_x);
  prefs.putInt("offset_y", offset_y);
  prefs.putInt("scale_x", scale_x);
  prefs.putInt("scale_y", scale_y);
  prefs.end();
  current_point = 0;

#ifdef DEBUG_MODE
  Serial.printf("Calibration saved: offset(%d,%d) scale(%d,%d)\n",
                offset_x, offset_y, scale_x, scale_y);
#endif
}

static void calculate_calibration(void) {
  // Calcul simple: moyenne des ecarts
  int32_t sum_dx = 0;
  int32_t sum_dy = 0;
  int32_t sum_sx = 0;
  int32_t sum_sy = 0;

  for (int i = 0; i < 4; i++) {
    sum_dx += (calib_points[i].screen_x - calib_points[i].raw_x);
    sum_dy += (calib_points[i].screen_y - calib_points[i].raw_y);

    if (calib_points[i].raw_x != 0) {
      sum_sx += (calib_points[i].screen_x * 1000) / calib_points[i].raw_x;
    }
    if (calib_points[i].raw_y != 0) {
      sum_sy += (calib_points[i].screen_y * 1000) / calib_points[i].raw_y;
    }
  }

  offset_x = sum_dx / 4;
  offset_y = sum_dy / 4;
  scale_x = sum_sx / 4;
  scale_y = sum_sy / 4;

#ifdef DEBUG_MODE
  Serial.println("Calibration calculated:");
  for (int i = 0; i < 4; i++) {
    Serial.printf("Point %d: screen(%d,%d) raw(%d,%d)\n", i,
                  calib_points[i].screen_x, calib_points[i].screen_y,
                  calib_points[i].raw_x, calib_points[i].raw_y);
  }
  Serial.printf("Result: offset(%d,%d) scale(%d,%d)\n",
                offset_x, offset_y, scale_x, scale_y);
#endif
}

static void update_target_position(void) {
  const int margin = 50;
  int16_t x, y;

  switch (current_point) {
    case 0:  // Haut gauche
      x = margin;
      y = margin;
      calib_points[0].screen_x = margin;
      calib_points[0].screen_y = margin;
      lv_label_set_text(instruction_label, "Touchez la cible en haut a gauche");
      break;
    case 1:  // Haut droite
      x = SCREEN_WIDTH - margin;
      y = margin;
      calib_points[1].screen_x = SCREEN_WIDTH - margin;
      calib_points[1].screen_y = margin;
      lv_label_set_text(instruction_label, "Touchez la cible en haut a droite");
      break;
    case 2:  // Bas droite
      x = SCREEN_WIDTH - margin;
      y = SCREEN_HEIGHT - margin;
      calib_points[2].screen_x = SCREEN_WIDTH - margin;
      calib_points[2].screen_y = SCREEN_HEIGHT - margin;
      lv_label_set_text(instruction_label, "Touchez la cible en bas a droite");
      break;
    case 3:  // Bas gauche
      x = margin;
      y = SCREEN_HEIGHT - margin;
      calib_points[3].screen_x = margin;
      calib_points[3].screen_y = SCREEN_HEIGHT - margin;
      lv_label_set_text(instruction_label, "Touchez la cible en bas a gauche");
      break;
    default:
      return;
  }

  lv_obj_set_pos(target_obj, x - 30, y - 30);
}

static void screen_touch_cb(lv_event_t *e) {
  if (calibration_done || current_point >= 4) return;
  lv_point_t point;
  lv_indev_t *indev = lv_indev_get_act();
  lv_indev_get_point(indev, &point);

  calib_points[current_point].raw_x = point.x;
  calib_points[current_point].raw_y = point.y;

#ifdef DEBUG_MODE
  Serial.printf("Point %d captured: raw(%d,%d)\n", current_point, point.x, point.y);
#endif

  current_point++;

  if (current_point < 4) {
    update_target_position();
  } else {
    // Calibration terminee
    calculate_calibration();
    calibration_done = true;
    lv_obj_add_flag(target_obj, LV_OBJ_FLAG_HIDDEN);
    lv_label_set_text(instruction_label,
                      "Calibration terminee!\nCliquez sur Enregistrer");
  }
}

static void btn_save_calib_cb(lv_event_t *e) {
  if (calibration_done) {
    save_calibration();
    screen_calibration = NULL;
#ifdef DEBUG_MODE
    Serial.println("Screen calibration saved");
#endif
  }
  ui_settings_show();
}

static void btn_cancel_calib_cb(lv_event_t *e) {
#ifdef DEBUG_MODE
  Serial.println("Screen calibration cancelled");
#endif
  screen_calibration = NULL;
  ui_settings_show();
}

void ui_settings_screen_init(void) {
  const TextStrings *txt = get_text();

  current_point = 0;
  calibration_done = false;

  screen_calibration = lv_obj_create(NULL);
  lv_obj_set_style_bg_color(screen_calibration, lv_color_hex(0x0a0e27), 0);
  lv_obj_add_event_cb(screen_calibration, screen_touch_cb, LV_EVENT_CLICKED, NULL);

  // Frame principal
  lv_obj_t *main_frame = lv_obj_create(screen_calibration);
  lv_obj_set_size(main_frame, SCREEN_WIDTH, SCREEN_HEIGHT);
  lv_obj_center(main_frame);
  lv_obj_set_style_bg_opa(main_frame, LV_OPA_TRANSP, 0);
  lv_obj_set_style_border_width(main_frame, 0, 0);
  lv_obj_set_style_pad_all(main_frame, 0, 0);
  lv_obj_clear_flag(main_frame, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_clear_flag(main_frame, LV_OBJ_FLAG_CLICKABLE);

  // Titre
  lv_obj_t *title = lv_label_create(main_frame);
  lv_label_set_text(title, txt->screen_calibration);
  lv_obj_set_style_text_font(title, &lv_font_montserrat_32, 0);
  lv_obj_set_style_text_color(title, lv_color_hex(0x00d4ff), 0);
  lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 20);
  lv_obj_clear_flag(title, LV_OBJ_FLAG_CLICKABLE);

  // Instructions
  instruction_label = lv_label_create(main_frame);
  lv_obj_set_style_text_font(instruction_label, &lv_font_montserrat_24, 0);
  lv_obj_set_style_text_color(instruction_label, lv_color_white(), 0);
  lv_obj_set_style_text_align(instruction_label, LV_TEXT_ALIGN_CENTER, 0);
  lv_obj_align(instruction_label, LV_ALIGN_CENTER, 0, 0);
  lv_obj_clear_flag(instruction_label, LV_OBJ_FLAG_CLICKABLE);

  // Cible
  target_obj = lv_obj_create(main_frame);
  lv_obj_set_size(target_obj, 60, 60);
  lv_obj_set_style_radius(target_obj, 30, 0);
  lv_obj_set_style_bg_color(target_obj, lv_color_hex(0xff3b30), 0);
  lv_obj_set_style_border_width(target_obj, 3, 0);
  lv_obj_set_style_border_color(target_obj, lv_color_white(), 0);
  lv_obj_clear_flag(target_obj, LV_OBJ_FLAG_CLICKABLE);

  // Cercle interieur
  lv_obj_t *inner_circle = lv_obj_create(target_obj);
  lv_obj_set_size(inner_circle, 20, 20);
  lv_obj_center(inner_circle);
  lv_obj_set_style_radius(inner_circle, 10, 0);
  lv_obj_set_style_bg_color(inner_circle, lv_color_white(), 0);
  lv_obj_set_style_border_width(inner_circle, 0, 0);
  lv_obj_clear_flag(inner_circle, LV_OBJ_FLAG_CLICKABLE);

  // Boutons
  lv_obj_t *btn_container = lv_obj_create(main_frame);
  lv_obj_set_size(btn_container, lv_pct(100), LV_SIZE_CONTENT);
  lv_obj_align(btn_container, LV_ALIGN_BOTTOM_MID, 0, -20);
  lv_obj_set_style_bg_opa(btn_container, LV_OPA_TRANSP, 0);
  lv_obj_set_style_border_width(btn_container, 0, 0);
  lv_obj_set_style_pad_all(btn_container, 0, 0);
  lv_obj_set_flex_flow(btn_container, LV_FLEX_FLOW_ROW);
  lv_obj_set_flex_align(btn_container, LV_FLEX_ALIGN_SPACE_EVENLY,
                        LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
  lv_obj_clear_flag(btn_container, LV_OBJ_FLAG_CLICKABLE);
  lv_obj_clear_flag(btn_container, LV_OBJ_FLAG_SCROLLABLE);

  // Bouton Enregistrer
  lv_obj_t *btn_save = lv_button_create(btn_container);
  lv_obj_set_size(btn_save, 300, 60);
  lv_obj_set_style_bg_color(btn_save, lv_color_hex(0x34c759), 0);
  lv_obj_set_style_radius(btn_save, 12, 0);
  lv_obj_add_event_cb(btn_save, btn_save_calib_cb, LV_EVENT_CLICKED, NULL);

  lv_obj_t *label_save = lv_label_create(btn_save);
  lv_label_set_text(label_save, txt->save);
  lv_obj_set_style_text_font(label_save, &lv_font_montserrat_24, 0);
  lv_obj_center(label_save);

  // Bouton Annuler
  lv_obj_t *btn_cancel = lv_button_create(btn_container);
  lv_obj_set_size(btn_cancel, 300, 60);
  lv_obj_set_style_bg_color(btn_cancel, lv_color_hex(0xff3b30), 0);
  lv_obj_set_style_radius(btn_cancel, 12, 0);
  lv_obj_add_event_cb(btn_cancel, btn_cancel_calib_cb, LV_EVENT_CLICKED, NULL);

  lv_obj_t *label_cancel = lv_label_create(btn_cancel);
  lv_label_set_text(label_cancel, txt->cancel);
  lv_obj_set_style_text_font(label_cancel, &lv_font_montserrat_24, 0);
  lv_obj_center(label_cancel);

  // Positionner premiere cible
  update_target_position();

#ifdef DEBUG_MODE
  Serial.println("Screen calibration initialized");
#endif
}

void ui_settings_screen_show(void) {
  if (screen_calibration == NULL) {
    ui_settings_screen_init();
  }
  lv_screen_load(screen_calibration);

#ifdef DEBUG_MODE
  Serial.println("Screen calibration shown");
#endif
}

#endif