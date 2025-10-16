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
void ui_settings_screen_show(void);  // Ajout de la declaration
void get_touch_calibration(float *a, float *b, float *c, float *d, float *e, float *f);

static lv_obj_t *screen_calibration = NULL;
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

static calib_point_t calib_points[4];
static uint8_t current_point = 0;
static bool calibration_done = false;

// Parametres de calibration (transformation affine)
static float calib_a = 1.0f;  // scale X
static float calib_b = 0.0f;  // rotation/skew
static float calib_c = 0.0f;  // offset X
static float calib_d = 0.0f;  // rotation/skew
static float calib_e = 1.0f;  // scale Y
static float calib_f = 0.0f;  // offset Y

static void load_calibration(void) {
  prefs.begin("touch_calib", true);
  calib_a = prefs.getFloat("a", 1.0f);
  calib_b = prefs.getFloat("b", 0.0f);
  calib_c = prefs.getFloat("c", 0.0f);
  calib_d = prefs.getFloat("d", 0.0f);
  calib_e = prefs.getFloat("e", 1.0f);
  calib_f = prefs.getFloat("f", 0.0f);
  prefs.end();

#ifdef DEBUG_MODE
  Serial.printf("Calibration loaded: a=%.3f b=%.3f c=%.3f d=%.3f e=%.3f f=%.3f\n",
                calib_a, calib_b, calib_c, calib_d, calib_e, calib_f);
#endif
}

static void save_calibration(void) {
  prefs.begin("touch_calib", false);
  prefs.putFloat("a", calib_a);
  prefs.putFloat("b", calib_b);
  prefs.putFloat("c", calib_c);
  prefs.putFloat("d", calib_d);
  prefs.putFloat("e", calib_e);
  prefs.putFloat("f", calib_f);
  prefs.end();

#ifdef DEBUG_MODE
  Serial.printf("Calibration saved: a=%.3f b=%.3f c=%.3f d=%.3f e=%.3f f=%.3f\n",
                calib_a, calib_b, calib_c, calib_d, calib_e, calib_f);
#endif
}

static void calculate_calibration(void) {
  // Calcul de la transformation affine par methode des moindres carres
  // X_screen = a * X_raw + b * Y_raw + c
  // Y_screen = d * X_raw + e * Y_raw + f
  
  float sumX = 0, sumY = 0, sumXX = 0, sumYY = 0, sumXY = 0;
  float sumXs = 0, sumYs = 0, sumXXs = 0, sumYYs = 0, sumXYs = 0;
  
  // Calcul des sommes pour les 4 points
  for (int i = 0; i < 4; i++) {
    float x = calib_points[i].raw_x;
    float y = calib_points[i].raw_y;
    float xs = calib_points[i].screen_x;
    float ys = calib_points[i].screen_y;
    
    sumX += x;
    sumY += y;
    sumXX += x * x;
    sumYY += y * y;
    sumXY += x * y;
    
    sumXs += xs;
    sumYs += ys;
    sumXXs += x * xs;
    sumYYs += y * ys;
    sumXYs += y * xs;
  }
  
  // Resolution du systeme pour X
  float det = 4 * (sumXX * sumYY - sumXY * sumXY) - (sumX * sumX * sumYY + sumY * sumY * sumXX - 2 * sumX * sumY * sumXY);
  
  if (fabs(det) > 0.001f) {
    // Calcul des coefficients pour X
    calib_a = (4 * sumXXs * sumYY - sumXs * sumX * sumYY - 4 * sumXYs * sumXY + sumXs * sumY * sumXY + sumX * sumY * sumYs - sumX * sumYY * sumXs) / det;
    calib_b = (4 * sumXYs * sumXX - sumXs * sumY * sumXX - 4 * sumXXs * sumXY + sumXs * sumX * sumXY + sumY * sumX * sumXs - sumY * sumXX * sumXs) / det;
    calib_c = (sumXs * sumXX * sumYY - sumXs * sumXY * sumXY - sumXXs * sumX * sumYY - sumXYs * sumY * sumXX + sumXXs * sumY * sumXY + sumXYs * sumX * sumXY) / det;
    
    // Calcul des coefficients pour Y
    float sumXYsy = 0, sumYYsy = 0;
    for (int i = 0; i < 4; i++) {
      sumXYsy += calib_points[i].raw_x * calib_points[i].screen_y;
      sumYYsy += calib_points[i].raw_y * calib_points[i].screen_y;
    }
    
    calib_d = (4 * sumXYsy * sumYY - sumYs * sumX * sumYY - 4 * sumYYsy * sumXY + sumYs * sumY * sumXY + sumX * sumY * sumYs - sumX * sumYY * sumYs) / det;
    calib_e = (4 * sumYYsy * sumXX - sumYs * sumY * sumXX - 4 * sumXYsy * sumXY + sumYs * sumX * sumXY + sumY * sumX * sumYs - sumY * sumXX * sumYs) / det;
    calib_f = (sumYs * sumXX * sumYY - sumYs * sumXY * sumXY - sumXYsy * sumX * sumYY - sumYYsy * sumY * sumXX + sumXYsy * sumY * sumXY + sumYYsy * sumX * sumXY) / det;
  } else {
    // Fallback: calibration simple par moyennage
#ifdef DEBUG_MODE
    Serial.println("Calibration: determinant trop faible, utilisation methode simple");
#endif
    
    float scale_x = (float)SCREEN_WIDTH / (calib_points[1].raw_x - calib_points[0].raw_x + calib_points[3].raw_x - calib_points[2].raw_x) * 2;
    float scale_y = (float)SCREEN_HEIGHT / (calib_points[2].raw_y - calib_points[0].raw_y + calib_points[3].raw_y - calib_points[1].raw_y) * 2;
    
    calib_a = scale_x;
    calib_b = 0;
    calib_c = 30 - calib_points[0].raw_x * scale_x;
    calib_d = 0;
    calib_e = scale_y;
    calib_f = 30 - calib_points[0].raw_y * scale_y;
  }

#ifdef DEBUG_MODE
  Serial.printf("Calibration calculee: a=%.3f b=%.3f c=%.3f d=%.3f e=%.3f f=%.3f\n",
                calib_a, calib_b, calib_c, calib_d, calib_e, calib_f);
  
  // Verification
  for (int i = 0; i < 4; i++) {
    int16_t calc_x = calib_a * calib_points[i].raw_x + calib_b * calib_points[i].raw_y + calib_c;
    int16_t calc_y = calib_d * calib_points[i].raw_x + calib_e * calib_points[i].raw_y + calib_f;
    Serial.printf("Point %d: attendu(%d,%d) calcule(%d,%d) erreur(%d,%d)\n", 
                  i, calib_points[i].screen_x, calib_points[i].screen_y,
                  calc_x, calc_y,
                  calc_x - calib_points[i].screen_x, calc_y - calib_points[i].screen_y);
  }
#endif
}

// Fonction pour appliquer la calibration aux coordonnees brutes
static void apply_calibration(int16_t raw_x, int16_t raw_y, int16_t *calib_x, int16_t *calib_y) {
  *calib_x = (int16_t)(calib_a * raw_x + calib_b * raw_y + calib_c);
  *calib_y = (int16_t)(calib_d * raw_x + calib_e * raw_y + calib_f);
  
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
      y = 30;
      calib_points[1].screen_x = x;
      calib_points[1].screen_y = y;
      lv_label_set_text(instruction_label, "Touchez la cible en haut a droite");
      break;
    case 2:
      x = SCREEN_WIDTH - 30; 
      y = SCREEN_HEIGHT - 30;
      calib_points[2].screen_x = x;
      calib_points[2].screen_y = y;
      lv_label_set_text(instruction_label, "Touchez la cible en bas a droite");
      break;
    case 3:
      x = 30; 
      y = SCREEN_HEIGHT - 30;
      calib_points[3].screen_x = x;
      calib_points[3].screen_y = y;
      lv_label_set_text(instruction_label, "Touchez la cible en bas a gauche");
      break;
    default:
      return;
  }
  
  // Centrer la cible sur les coordonnees
  lv_obj_set_pos(target_obj, x - 40, y - 40);  // 40 = moitie de 80 (taille de la cible)
}

static void btn_test_calib_cb(lv_event_t *e) {
  // Activer le mode test
  test_mode = true;
  
  // Creer le point de test s'il n'existe pas
  if (test_point == NULL) {
    test_point = lv_obj_create(screen_calibration);
    lv_obj_set_size(test_point, 20, 20);
    lv_obj_set_style_radius(test_point, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(test_point, lv_color_hex(0x00FF00), 0);  // Vert pour le point calibre
    lv_obj_set_style_border_color(test_point, lv_color_hex(0xFF0000), 0);  // Bordure rouge
    lv_obj_set_style_border_width(test_point, 2, 0);
    lv_obj_clear_flag(test_point, LV_OBJ_FLAG_CLICKABLE);
  }
  
  lv_obj_clear_flag(test_point, LV_OBJ_FLAG_HIDDEN);
  lv_label_set_text(instruction_label, "Mode test: touchez l'ecran\nLe point vert suit votre doigt");
  
  // Cacher le bouton test en utilisant lv_event_get_target
  //lv_obj_t *btn = lv_event_get_target(e);
  //lv_obj_add_flag(btn, LV_OBJ_FLAG_HIDDEN);
}

static void screen_touch_cb(lv_event_t *e) {
  lv_event_code_t code = lv_event_get_code(e);
  
  if (code == LV_EVENT_PRESSED || code == LV_EVENT_PRESSING) {
    lv_point_t point;
    lv_indev_t *indev = lv_indev_get_act();
    lv_indev_get_point(indev, &point);
    
    // Mode test: deplacer le point selon le toucher
    if (test_mode && test_point != NULL) {
      // Appliquer la calibration aux coordonnees brutes
      int16_t calib_x, calib_y;
      apply_calibration(point.x, point.y, &calib_x, &calib_y);
      
      // Positionner le point calibre
      lv_obj_set_pos(test_point, calib_x - 10, calib_y - 10);
      
#ifdef DEBUG_MODE
      Serial.printf("Test mode: raw(%d,%d) -> calibrated(%d,%d)\n", 
                    point.x, point.y, calib_x, calib_y);
#endif
      return;
    }
    
    // Mode calibration normale
    if (calibration_done || current_point >= 4) return;
    
    // Verifier que le toucher est proche de la cible (tolerance de 150 pixels)
    int16_t target_x = calib_points[current_point].screen_x;
    int16_t target_y = calib_points[current_point].screen_y;
    int16_t distance = abs(point.x - target_x) + abs(point.y - target_y);
    
#ifdef DEBUG_MODE
    Serial.printf("Touch at (%d,%d), target at (%d,%d), distance=%d\n", 
                  point.x, point.y, target_x, target_y, distance);
#endif
    
    // Si le toucher est dans la zone de tolerance autour de la cible
    if (distance < 150) {  // Tolerance de 150 pixels
      calib_points[current_point].raw_x = point.x;
      calib_points[current_point].raw_y = point.y;

#ifdef DEBUG_MODE
      Serial.printf("Point %d: ecran(%d,%d) brut(%d,%d)\n", 
                    current_point, 
                    calib_points[current_point].screen_x, calib_points[current_point].screen_y,
                    calib_points[current_point].raw_x, calib_points[current_point].raw_y);
#endif
      
      current_point++;
      
      if (current_point < 4) {
        update_target_position();
      } else {
        calculate_calibration();
        calibration_done = true;
        lv_obj_add_flag(target_obj, LV_OBJ_FLAG_HIDDEN);
        lv_label_set_text(instruction_label, "Calibration terminee!\nCliquez sur Enregistrer ou Tester");
        
        // Creer un bouton de test (au milieu pour eviter les coins)
        lv_obj_t *btn_test = lv_btn_create(screen_calibration);
        lv_obj_set_size(btn_test, 120, 40);
        lv_obj_align(btn_test, LV_ALIGN_CENTER, 0, 20);  // Au centre de l'ecran
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
    Serial.println("Calibration ecran enregistree");
#endif
  }
  screen_calibration = NULL;
  test_point = NULL;  // Reset du pointeur
  test_mode = false;  // Reset du mode test
  // Retour a l'ecran des parametres - utiliser ui_settings_show definie dans ui_settings.h
  ui_settings_show();
}

static void btn_cancel_calib_cb(lv_event_t *e) {
#ifdef DEBUG_MODE
  Serial.println("Calibration ecran annulee");
#endif
  screen_calibration = NULL;
  test_point = NULL;  // Reset du pointeur
  test_mode = false;  // Reset du mode test
  // Retour a l'ecran des parametres - utiliser ui_settings_show definie dans ui_settings.h
  ui_settings_show();
}

void ui_settings_screen_init(void) {
  const TextStrings *txt = get_text();
  
  current_point = 0;
  calibration_done = false;
  test_mode = false;
  test_point = NULL;
  
  screen_calibration = lv_obj_create(NULL);
  lv_obj_set_style_bg_color(screen_calibration, lv_color_hex(0x0a0e27), 0);
  // Utiliser PRESSED et PRESSING pour detection continue
  lv_obj_add_event_cb(screen_calibration, screen_touch_cb, LV_EVENT_PRESSED, NULL);
  lv_obj_add_event_cb(screen_calibration, screen_touch_cb, LV_EVENT_PRESSING, NULL);
  
  // Frame principal
  lv_obj_t *main_frame = lv_obj_create(screen_calibration);
  lv_obj_set_size(main_frame, SCREEN_WIDTH, SCREEN_HEIGHT);
  lv_obj_center(main_frame);
  lv_obj_set_style_bg_opa(main_frame, LV_OPA_TRANSP, 0);
  lv_obj_set_style_border_width(main_frame, 0, 0);
  lv_obj_clear_flag(main_frame, LV_OBJ_FLAG_CLICKABLE);  // Important: ne pas capturer les clics
  
  // Instructions
  instruction_label = lv_label_create(main_frame);
  lv_label_set_text(instruction_label, "Touchez la cible en haut a gauche");
  lv_obj_set_style_text_align(instruction_label, LV_TEXT_ALIGN_CENTER, 0);
  lv_obj_set_style_text_color(instruction_label, lv_color_hex(0xFFFFFF), 0);
  lv_obj_set_style_text_font(instruction_label, &lv_font_montserrat_24, 0);  // Police plus grande
  lv_obj_align(instruction_label, LV_ALIGN_CENTER, 0, -150);  // Plus haut pour eviter les cibles
  
  // Cible - la rendre plus grande et plus visible
  target_obj = lv_obj_create(screen_calibration);
  lv_obj_set_size(target_obj, 80, 80);  // Plus grande
  lv_obj_set_style_bg_color(target_obj, lv_color_hex(0x202040), 0);  // Fond semi-transparent
  lv_obj_set_style_bg_opa(target_obj, LV_OPA_50, 0);
  lv_obj_set_style_border_width(target_obj, 0, 0);
  lv_obj_clear_flag(target_obj, LV_OBJ_FLAG_CLICKABLE);
  
  // Croix de la cible - plus visible
  lv_obj_t *cross_h = lv_obj_create(target_obj);
  lv_obj_set_size(cross_h, 80, 3);  // Plus epaisse
  lv_obj_set_style_bg_color(cross_h, lv_color_hex(0xFF0000), 0);
  lv_obj_set_style_border_width(cross_h, 0, 0);
  lv_obj_center(cross_h);
  lv_obj_clear_flag(cross_h, LV_OBJ_FLAG_CLICKABLE);
  
  lv_obj_t *cross_v = lv_obj_create(target_obj);
  lv_obj_set_size(cross_v, 3, 80);  // Plus epaisse
  lv_obj_set_style_bg_color(cross_v, lv_color_hex(0xFF0000), 0);
  lv_obj_set_style_border_width(cross_v, 0, 0);
  lv_obj_center(cross_v);
  lv_obj_clear_flag(cross_v, LV_OBJ_FLAG_CLICKABLE);
  
  // Cercle de la cible - plus visible
  lv_obj_t *circle = lv_obj_create(target_obj);
  lv_obj_set_size(circle, 50, 50);
  lv_obj_set_style_radius(circle, LV_RADIUS_CIRCLE, 0);
  lv_obj_set_style_bg_opa(circle, LV_OPA_TRANSP, 0);
  lv_obj_set_style_border_color(circle, lv_color_hex(0xFF0000), 0);
  lv_obj_set_style_border_width(circle, 3, 0);  // Plus epais
  lv_obj_center(circle);
  lv_obj_clear_flag(circle, LV_OBJ_FLAG_CLICKABLE);
  
  // Point central
  lv_obj_t *center_point = lv_obj_create(target_obj);
  lv_obj_set_size(center_point, 10, 10);
  lv_obj_set_style_radius(center_point, LV_RADIUS_CIRCLE, 0);
  lv_obj_set_style_bg_color(center_point, lv_color_hex(0xFFFF00), 0);
  lv_obj_set_style_border_width(center_point, 0, 0);
  lv_obj_center(center_point);
  lv_obj_clear_flag(center_point, LV_OBJ_FLAG_CLICKABLE);
  
  // Position initiale
  update_target_position();
  
  // Boutons - positionnes au centre vertical pour eviter les coins
  lv_obj_t *btn_save = lv_btn_create(main_frame);
  lv_obj_set_size(btn_save, 120, 40);
  lv_obj_align(btn_save, LV_ALIGN_CENTER, -80, 100);  // Centre vertical decale
  lv_obj_t *label_save = lv_label_create(btn_save);
  lv_label_set_text(label_save, "Enregistrer");
  lv_obj_center(label_save);
  lv_obj_add_event_cb(btn_save, btn_save_calib_cb, LV_EVENT_CLICKED, NULL);
  
  lv_obj_t *btn_cancel = lv_btn_create(main_frame);
  lv_obj_set_size(btn_cancel, 120, 40);
  lv_obj_align(btn_cancel, LV_ALIGN_CENTER, 80, 100);  // Centre vertical decale
  lv_obj_t *label_cancel = lv_label_create(btn_cancel);
  lv_label_set_text(label_cancel, "Annuler");
  lv_obj_center(label_cancel);
  lv_obj_add_event_cb(btn_cancel, btn_cancel_calib_cb, LV_EVENT_CLICKED, NULL);
  
  lv_scr_load(screen_calibration);
}

// Fonction pour obtenir les parametres de calibration (a appeler depuis le driver touch)
void get_touch_calibration(float *a, float *b, float *c, float *d, float *e, float *f) {
  if (a) *a = calib_a;
  if (b) *b = calib_b;
  if (c) *c = calib_c;
  if (d) *d = calib_d;
  if (e) *e = calib_e;
  if (f) *f = calib_f;
}

// Fonction pour afficher l'ecran de calibration
void ui_settings_screen_show(void) {
  if (screen_calibration == NULL) {
    ui_settings_screen_init();
  }
  lv_scr_load(screen_calibration);
}

#endif