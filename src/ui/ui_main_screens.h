#ifndef UI_MAIN_SCREENS_H
#define UI_MAIN_SCREENS_H

#include "lvgl.h"
#include "constants.h"
#include "UI_helper.h"
#include "globals.h"

// Indice ecran courant (0=gauche, 1=centre, 2=droite)
static uint8_t current_screen_index = 1;

// Forward declarations
void ui_screen_left_init(void);
void ui_screen_center_init(void);
void ui_screen_right_init(void);

// Variables pour gestion du swipe
static lv_point_t touch_start_point;
static bool touch_started = false;

// Fonction pour changer d'ecran
static void switch_to_screen(uint8_t index) {
  current_screen_index = index;
  
  // Recréer l'écran selon l'index
  switch(index) {
    case 0:
      ui_screen_left_init();
      break;
    case 1:
      ui_screen_center_init();
      break;
    case 2:
      ui_screen_right_init();
      break;
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
  // Nettoyer l'ecran s'il existe
  if (main_screen != NULL) {
    lv_obj_clean(main_screen);
  } else {
    main_screen = lv_obj_create(NULL);
  }
  
  lv_obj_set_style_bg_color(main_screen, lv_color_hex(0x000000), 0);
  //lv_obj_set_style_bg_grad_color(main_screen, lv_color_hex(0x1a1f3a), 0);
  //lv_obj_set_style_bg_grad_dir(main_screen, LV_GRAD_DIR_VER, 0);
  lv_obj_clear_flag(main_screen, LV_OBJ_FLAG_SCROLLABLE);
  
  // Barre de statut
  ui_create_status_bar(main_screen);
  
  lv_obj_t *frame = lv_obj_create(main_screen);
  lv_obj_set_size(frame, LCD_H_RES, LCD_V_RES - 60);
  lv_obj_align(frame, LV_ALIGN_TOP_MID, 0, 60);
  lv_obj_set_style_bg_color(frame, lv_color_hex(0x000000), 0);
  lv_obj_set_style_bg_opa(frame, LV_OPA_COVER, 0);
  lv_obj_set_style_border_width(frame, 3, 0);
  lv_obj_set_style_border_color(frame, lv_color_hex(0xFFFFFF), 0);
  lv_obj_set_style_radius(frame, ROUND_FRANE_RADUIS_SMALL, 0);
  lv_obj_set_style_pad_all(frame, 20, 0);
  lv_obj_clear_flag(frame, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_clear_flag(frame, LV_OBJ_FLAG_CLICKABLE);
  
  // Texte identifiant
  lv_obj_t *label = lv_label_create(frame);
  lv_label_set_text(label, "Ecran Gauche");
  lv_obj_set_style_text_font(label, &lv_font_montserrat_32, 0);
  lv_obj_set_style_text_color(label, lv_color_hex(0x00d4ff), 0);
  lv_obj_center(label);
  
  // Ajouter handler swipe sur l'ecran complet
  lv_obj_add_event_cb(main_screen, swipe_event_handler, LV_EVENT_PRESSED, NULL);
  lv_obj_add_event_cb(main_screen, swipe_event_handler, LV_EVENT_RELEASED, NULL);
  
  lv_screen_load(main_screen);
  
#ifdef DEBUG_MODE
  Serial.println("Left screen initialized");
#endif
}

// Initialisation ecran central
void ui_screen_center_init(void) {
  // Nettoyer l'ecran s'il existe
  if (main_screen != NULL) {
    lv_obj_clean(main_screen);
  } else {
    main_screen = lv_obj_create(NULL);
  }
  
  lv_obj_set_style_bg_color(main_screen, lv_color_hex(0x000000), 0);
  //lv_obj_set_style_bg_grad_color(main_screen, lv_color_hex(0x1a1f3a), 0);
  //lv_obj_set_style_bg_grad_dir(main_screen, LV_GRAD_DIR_VER, 0);
  lv_obj_clear_flag(main_screen, LV_OBJ_FLAG_SCROLLABLE);
  
  // Barre de statut
  ui_create_status_bar(main_screen);
  
  lv_obj_t *frame = lv_obj_create(main_screen);
  lv_obj_set_size(frame, LCD_H_RES, LCD_V_RES - 55);
  lv_obj_align(frame, LV_ALIGN_TOP_MID, 0, 55);
  lv_obj_set_style_bg_color(frame, lv_color_hex(0x000000), 0);
  lv_obj_set_style_bg_opa(frame, LV_OPA_COVER, 0);
  lv_obj_set_style_border_width(frame, 0, 0);
  lv_obj_set_style_border_color(frame, lv_color_hex(0x000000), 0);
  lv_obj_set_style_radius(frame, 0, 0);
  lv_obj_set_style_pad_all(frame, 0, 0);
  lv_obj_clear_flag(frame, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_clear_flag(frame, LV_OBJ_FLAG_CLICKABLE);
  
  int16_t col_height = 540; 
  int16_t col_center_width = 540;
  int16_t col_side_width = 479;
  
  // Colonne gauche
  lv_obj_t *col_left = lv_obj_create(frame);
  lv_obj_set_size(col_left, col_side_width, col_height);
  lv_obj_set_pos(col_left, 0, 5);
  lv_obj_set_style_bg_color(col_left, lv_color_hex(0x000000), 0);
  lv_obj_set_style_bg_opa(col_left, LV_OPA_COVER, 0);
  lv_obj_set_style_border_width(col_left, 2, 0);
  lv_obj_set_style_border_color(col_left, lv_color_hex(0xFFFFFF), 0);
  lv_obj_set_style_radius(col_left, ROUND_FRANE_RADUIS_SMALL, 0);
  lv_obj_set_style_pad_all(col_left, 10, 0);
  lv_obj_clear_flag(col_left, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_clear_flag(col_left, LV_OBJ_FLAG_CLICKABLE);
  
  lv_obj_t *label_left = lv_label_create(col_left);
  lv_label_set_text(label_left, "Gauche");
  lv_obj_set_style_text_font(label_left, &lv_font_montserrat_20, 0);
  lv_obj_set_style_text_color(label_left, lv_color_hex(0x00d4ff), 0);
  lv_obj_center(label_left);
  
  // Colonne centrale (carte)
  lv_obj_t *col_center = lv_obj_create(frame);
  lv_obj_set_size(col_center, col_center_width, col_height);
  lv_obj_set_pos(col_center, col_side_width + 5, 5);
  lv_obj_set_style_bg_color(col_center, lv_color_hex(0x000000), 0);
  lv_obj_set_style_bg_opa(col_center, LV_OPA_80, 0);
  lv_obj_set_style_border_width(col_center, 2, 0);
  lv_obj_set_style_border_color(col_center, lv_color_hex(0xFFFFFF), 0);
  lv_obj_set_style_radius(col_center, ROUND_FRANE_RADUIS_SMALL, 0);
  lv_obj_set_style_pad_all(col_center, 10, 0);
  lv_obj_clear_flag(col_center, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_clear_flag(col_center, LV_OBJ_FLAG_CLICKABLE);
  
  lv_obj_t *label_center = lv_label_create(col_center);
  lv_label_set_text(label_center, "Carte");
  lv_obj_set_style_text_font(label_center, &lv_font_montserrat_20, 0);
  lv_obj_set_style_text_color(label_center, lv_color_hex(0x00d4ff), 0);
  lv_obj_center(label_center);
    
  // Ajouter handler swipe sur l'ecran complet
  lv_obj_add_event_cb(main_screen, swipe_event_handler, LV_EVENT_PRESSED, NULL);
  lv_obj_add_event_cb(main_screen, swipe_event_handler, LV_EVENT_RELEASED, NULL);
  
  lv_screen_load(main_screen);
  
#ifdef DEBUG_MODE
  Serial.println("Center screen initialized");
#endif
}

// Initialisation ecran droite
void ui_screen_right_init(void) {
  // Nettoyer l'ecran s'il existe
  if (main_screen != NULL) {
    lv_obj_clean(main_screen);
  } else {
    main_screen = lv_obj_create(NULL);
  }
  
  lv_obj_set_style_bg_color(main_screen, lv_color_hex(0x000000), 0);
  //lv_obj_set_style_bg_grad_color(main_screen, lv_color_hex(0x1a1f3a), 0);
  //lv_obj_set_style_bg_grad_dir(main_screen, LV_GRAD_DIR_VER, 0);
  lv_obj_clear_flag(main_screen, LV_OBJ_FLAG_SCROLLABLE);
  
  // Barre de statut
  ui_create_status_bar(main_screen);
  
  lv_obj_t *frame = lv_obj_create(main_screen);
  lv_obj_set_size(frame, LCD_H_RES , LCD_V_RES - 60);
  lv_obj_align(frame, LV_ALIGN_TOP_MID, 0, 60);
  lv_obj_set_style_bg_color(frame, lv_color_hex(0x000000), 0);
  lv_obj_set_style_bg_opa(frame, LV_OPA_COVER, 0);
  lv_obj_set_style_border_width(frame, 3, 0);
  lv_obj_set_style_border_color(frame, lv_color_hex(0xFFFFFF), 0);
  lv_obj_set_style_radius(frame, ROUND_FRANE_RADUIS_SMALL, 0);
  lv_obj_set_style_pad_all(frame, 20, 0);
  lv_obj_clear_flag(frame, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_clear_flag(frame, LV_OBJ_FLAG_CLICKABLE);
  
  // Texte identifiant
  lv_obj_t *label = lv_label_create(frame);
  lv_label_set_text(label, "Ecran Droite");
  lv_obj_set_style_text_font(label, &lv_font_montserrat_32, 0);
  lv_obj_set_style_text_color(label, lv_color_hex(0x00d4ff), 0);
  lv_obj_center(label);
  
  // Ajouter handler swipe sur l'ecran complet
  lv_obj_add_event_cb(main_screen, swipe_event_handler, LV_EVENT_PRESSED, NULL);
  lv_obj_add_event_cb(main_screen, swipe_event_handler, LV_EVENT_RELEASED, NULL);
  
  lv_screen_load(main_screen);
  
#ifdef DEBUG_MODE
  Serial.println("Right screen initialized");
#endif
}

// Initialisation des 3 ecrans
void ui_main_screens_init(void) {
  // Nettoyer les objets globaux
  if (keyboard != NULL) {
    lv_obj_del(keyboard);
    keyboard = NULL;
  }
  
  if (ta_active != NULL) {
    ta_active = NULL;
  }
  
  // Afficher ecran central au demarrage
  ui_screen_center_init();
  current_screen_index = 1;
  
#ifdef DEBUG_MODE
  Serial.println("Main screens system initialized");
#endif
}

// Fonction pour afficher les ecrans principaux depuis le menu
void ui_main_screens_show(void) {
  // Nettoyer les objets globaux
  if (keyboard != NULL) {
    lv_obj_del(keyboard);
    keyboard = NULL;
  }
  
  if (ta_active != NULL) {
    ta_active = NULL;
  }
  
  ui_main_screens_init();
  
  // Activer flag pour test logger
  mainscreen_active = true;
  
#ifdef DEBUG_MODE
  Serial.println("Main screens displayed");
#endif
}

#endif