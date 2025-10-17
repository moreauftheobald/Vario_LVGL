#ifndef UI_MAIN_SCREENS_H
#define UI_MAIN_SCREENS_H

#include "lvgl.h"
#include "constants.h"
#include "UI_helper.h"

// Objets des 3 ecrans principaux
static lv_obj_t *screen_left = NULL;
static lv_obj_t *screen_center = NULL;
static lv_obj_t *screen_right = NULL;

// Indice ecran courant (0=gauche, 1=centre, 2=droite)
static uint8_t current_screen_index = 1;

// Variables pour gestion du swipe
static lv_point_t touch_start_point;
static bool touch_started = false;

// Fonction pour changer d'ecran
static void switch_to_screen(uint8_t index) {
  current_screen_index = index;
  
  lv_obj_t *target_screen = NULL;
  
  switch(index) {
    case 0:
      target_screen = screen_left;
      break;
    case 1:
      target_screen = screen_center;
      break;
    case 2:
      target_screen = screen_right;
      break;
  }
  
  if (target_screen) {
    lv_screen_load_anim(target_screen, LV_SCR_LOAD_ANIM_MOVE_LEFT, 300, 0, false);
  }
  
#ifdef DEBUG_MODE
  Serial.printf("Switch to screen: %d\n", index);
#endif
}

// Event handler pour gestion du swipe
static void swipe_event_handler(lv_event_t *e) {
  lv_event_code_t code = lv_event_get_code(e);
  lv_obj_t *screen = (lv_obj_t*)lv_event_get_target(e);
  
  if (code == LV_EVENT_PRESSED) {
    lv_indev_t *indev = lv_indev_active();
    if (indev) {
      lv_indev_get_point(indev, &touch_start_point);
      touch_started = true;
#ifdef DEBUG_MODE
      Serial.printf("Touch start: x=%d, y=%d\n", touch_start_point.x, touch_start_point.y);
#endif
    }
  }
  else if (code == LV_EVENT_RELEASED) {
    if (touch_started) {
      lv_indev_t *indev = lv_indev_active();
      if (indev) {
        lv_point_t touch_end_point;
        lv_indev_get_point(indev, &touch_end_point);
        
        int32_t diff_x = touch_end_point.x - touch_start_point.x;
        int32_t diff_y = touch_end_point.y - touch_start_point.y;
        
#ifdef DEBUG_MODE
        Serial.printf("Touch end: x=%d, y=%d, diff_x=%d, diff_y=%d\n", 
                     touch_end_point.x, touch_end_point.y, diff_x, diff_y);
#endif
        
        // Detection swipe horizontal (threshold: 80px)
        if (abs(diff_x) > 80 && abs(diff_x) > abs(diff_y)) {
          if (diff_x > 0) {
            // Swipe vers droite -> ecran precedent
            if (current_screen_index > 0) {
              switch_to_screen(current_screen_index - 1);
            }
          } else {
            // Swipe vers gauche -> ecran suivant
            if (current_screen_index < 2) {
              switch_to_screen(current_screen_index + 1);
            }
          }
        }
      }
      touch_started = false;
    }
  }
}

// Initialisation ecran gauche
void ui_screen_left_init(void) {
  screen_left = lv_obj_create(NULL);
  lv_obj_set_style_bg_color(screen_left, lv_color_hex(0x0a0e27), 0);
  lv_obj_set_style_bg_grad_color(screen_left, lv_color_hex(0x1a1f3a), 0);
  lv_obj_set_style_bg_grad_dir(screen_left, LV_GRAD_DIR_VER, 0);
  lv_obj_clear_flag(screen_left, LV_OBJ_FLAG_SCROLLABLE);
  
  lv_obj_t *frame = ui_create_main_frame(screen_left);
  lv_obj_clear_flag(frame, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_clear_flag(frame, LV_OBJ_FLAG_CLICKABLE);
  
  // Texte identifiant
  lv_obj_t *label = lv_label_create(frame);
  lv_label_set_text(label, "Ecran Gauche");
  lv_obj_set_style_text_font(label, &lv_font_montserrat_32, 0);
  lv_obj_set_style_text_color(label, lv_color_hex(0x00d4ff), 0);
  lv_obj_center(label);
  
  // Ajouter handler swipe sur l'ecran complet
  lv_obj_add_event_cb(screen_left, swipe_event_handler, LV_EVENT_PRESSED, NULL);
  lv_obj_add_event_cb(screen_left, swipe_event_handler, LV_EVENT_RELEASED, NULL);
  
#ifdef DEBUG_MODE
  Serial.println("Left screen initialized");
#endif
}

// Initialisation ecran central
void ui_screen_center_init(void) {
  screen_center = lv_obj_create(NULL);
  lv_obj_set_style_bg_color(screen_center, lv_color_hex(0x0a0e27), 0);
  lv_obj_set_style_bg_grad_color(screen_center, lv_color_hex(0x1a1f3a), 0);
  lv_obj_set_style_bg_grad_dir(screen_center, LV_GRAD_DIR_VER, 0);
  lv_obj_clear_flag(screen_center, LV_OBJ_FLAG_SCROLLABLE);
  
  lv_obj_t *frame = ui_create_main_frame(screen_center);
  lv_obj_clear_flag(frame, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_clear_flag(frame, LV_OBJ_FLAG_CLICKABLE);
  
  // Texte identifiant
  lv_obj_t *label = lv_label_create(frame);
  lv_label_set_text(label, "Ecran Central");
  lv_obj_set_style_text_font(label, &lv_font_montserrat_32, 0);
  lv_obj_set_style_text_color(label, lv_color_hex(0x00d4ff), 0);
  lv_obj_center(label);
  
  // Ajouter handler swipe sur l'ecran complet
  lv_obj_add_event_cb(screen_center, swipe_event_handler, LV_EVENT_PRESSED, NULL);
  lv_obj_add_event_cb(screen_center, swipe_event_handler, LV_EVENT_RELEASED, NULL);
  
#ifdef DEBUG_MODE
  Serial.println("Center screen initialized");
#endif
}

// Initialisation ecran droite
void ui_screen_right_init(void) {
  screen_right = lv_obj_create(NULL);
  lv_obj_set_style_bg_color(screen_right, lv_color_hex(0x0a0e27), 0);
  lv_obj_set_style_bg_grad_color(screen_right, lv_color_hex(0x1a1f3a), 0);
  lv_obj_set_style_bg_grad_dir(screen_right, LV_GRAD_DIR_VER, 0);
  lv_obj_clear_flag(screen_right, LV_OBJ_FLAG_SCROLLABLE);
  
  lv_obj_t *frame = ui_create_main_frame(screen_right);
  lv_obj_clear_flag(frame, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_clear_flag(frame, LV_OBJ_FLAG_CLICKABLE);
  
  // Texte identifiant
  lv_obj_t *label = lv_label_create(frame);
  lv_label_set_text(label, "Ecran Droite");
  lv_obj_set_style_text_font(label, &lv_font_montserrat_32, 0);
  lv_obj_set_style_text_color(label, lv_color_hex(0x00d4ff), 0);
  lv_obj_center(label);
  
  // Ajouter handler swipe sur l'ecran complet
  lv_obj_add_event_cb(screen_right, swipe_event_handler, LV_EVENT_PRESSED, NULL);
  lv_obj_add_event_cb(screen_right, swipe_event_handler, LV_EVENT_RELEASED, NULL);
  
#ifdef DEBUG_MODE
  Serial.println("Right screen initialized");
#endif
}

// Initialisation des 3 ecrans
void ui_main_screens_init(void) {
  ui_screen_left_init();
  ui_screen_center_init();
  ui_screen_right_init();
  
  // Afficher ecran central au demarrage
  lv_screen_load(screen_center);
  
#ifdef DEBUG_MODE
  Serial.println("Main screens system initialized");
#endif
}

// Fonction pour afficher les ecrans principaux depuis le menu
void ui_main_screens_show(void) {
  if (screen_left == NULL || screen_center == NULL || screen_right == NULL) {
    if (lvgl_port_lock(-1)) {
      ui_main_screens_init();
      lvgl_port_unlock();
    }
  } else {
    if (lvgl_port_lock(-1)) {
      lv_screen_load(screen_center);
      current_screen_index = 1;
      lvgl_port_unlock();
    }
  }
  
#ifdef DEBUG_MODE
  Serial.println("Main screens displayed");
#endif
}

#endif