#ifndef UI_MAIN_SCREENS_H
#define UI_MAIN_SCREENS_H

#include "lvgl.h"
#include "constants.h"
#include "UI_helper.h"
#include "graphical.h"
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
static int current_map_zoom = 0;
static lv_obj_t *map_canvas = NULL;
static lv_obj_t *map_container = NULL;
static lv_obj_t *btn_zoom_in = NULL;
static lv_obj_t *btn_zoom_out = NULL;
static lv_obj_t *position_marker = NULL;

// Fonction pour changer d'ecran
static void switch_to_screen(uint8_t index) {
  current_screen_index = index;

  // Recreer l'ecran selon l'index
  switch (index) {
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
  lv_obj_t *screen = (lv_obj_t *)lv_event_get_target(e);

  if (code == LV_EVENT_PRESSED) {
    lv_indev_t *indev = lv_indev_active();
    if (indev) {
      lv_indev_get_point(indev, &touch_start_point);
      touch_started = true;
    }
  } else if (code == LV_EVENT_RELEASED) {
    if (touch_started) {
      lv_indev_t *indev = lv_indev_active();
      if (indev) {
        lv_point_t touch_end_point;
        lv_indev_get_point(indev, &touch_end_point);

        int32_t diff_x = touch_end_point.x - touch_start_point.x;
        int32_t diff_y = touch_end_point.y - touch_start_point.y;

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

// Fonction pour mettre a jour l'etat des boutons zoom
static void update_zoom_buttons_state(void) {
  if (btn_zoom_in && btn_zoom_out) {
    if (current_map_zoom >= MAP_ZOOM_MAX) {
      lv_obj_add_state(btn_zoom_in, LV_STATE_DISABLED);
    } else {
      lv_obj_clear_state(btn_zoom_in, LV_STATE_DISABLED);
    }

    if (current_map_zoom <= MAP_ZOOM_MIN) {
      lv_obj_add_state(btn_zoom_out, LV_STATE_DISABLED);
    } else {
      lv_obj_clear_state(btn_zoom_out, LV_STATE_DISABLED);
    }
  }
}

// Callbacks pour les boutons zoom
static void btn_zoom_in_cb(lv_event_t *e) {
  if (current_map_zoom < MAP_ZOOM_MAX) {
    current_map_zoom++;
    update_zoom_buttons_state();

#ifdef DEBUG_MODE
    Serial.printf("Zoom in: level=%d\n", current_map_zoom);
#endif

    // Recreer la carte avec nouveau zoom
    if (map_canvas) {
      lv_obj_del(map_canvas);
#ifdef FLIGHT_TEST_MODE
      map_canvas = create_map_view(map_container, TEST_LAT, TEST_LON,
                                   current_map_zoom, 527, 527);
#else
      double display_lat = g_sensor_data.gps.valid ? g_sensor_data.gps.latitude : TEST_LAT;
      double display_lon = g_sensor_data.gps.valid ? g_sensor_data.gps.longitude : TEST_LON;
      map_canvas = create_map_view(map_container, display_lat, display_lon,
                                   current_map_zoom, 527, 527);
#endif
      if (map_canvas) {
        lv_obj_align(map_canvas, LV_ALIGN_CENTER, 0, 0);
      }
      
      // Remonter les boutons au premier plan
      if (btn_zoom_in) lv_obj_move_foreground(btn_zoom_in);
      if (btn_zoom_out) lv_obj_move_foreground(btn_zoom_out);
      
      // Recreer le marqueur position
      if (position_marker) {
        lv_obj_del(position_marker);
      }
      position_marker = lv_label_create(map_container);
      lv_label_set_text(position_marker, LV_SYMBOL_GPS);
      lv_obj_set_style_text_font(position_marker, &lv_font_montserrat_32, 0);
      lv_obj_set_style_text_color(position_marker, lv_color_hex(UI_COLOR_GPS_MARKER), 0);
      lv_obj_set_style_bg_opa(position_marker, LV_OPA_TRANSP, 0);
      lv_obj_center(position_marker);
      lv_obj_move_foreground(position_marker);
    }
  }
}

static void btn_zoom_out_cb(lv_event_t *e) {
  if (current_map_zoom > MAP_ZOOM_MIN) {
    current_map_zoom--;
    update_zoom_buttons_state();

#ifdef DEBUG_MODE
    Serial.printf("Zoom out: level=%d\n", current_map_zoom);
#endif

    // Recreer la carte avec nouveau zoom
    if (map_canvas) {
      lv_obj_del(map_canvas);
#ifdef FLIGHT_TEST_MODE
      map_canvas = create_map_view(map_container, TEST_LAT, TEST_LON,
                                   current_map_zoom, 527, 527);
#else
      double display_lat = g_sensor_data.gps.valid ? g_sensor_data.gps.latitude : TEST_LAT;
      double display_lon = g_sensor_data.gps.valid ? g_sensor_data.gps.longitude : TEST_LON;
      map_canvas = create_map_view(map_container, display_lat, display_lon,
                                   current_map_zoom, 527, 527);
#endif
      if (map_canvas) {
        lv_obj_align(map_canvas, LV_ALIGN_CENTER, 0, 0);
      }
      
      // Remonter les boutons au premier plan
      if (btn_zoom_in) lv_obj_move_foreground(btn_zoom_in);
      if (btn_zoom_out) lv_obj_move_foreground(btn_zoom_out);
      
      // Recreer le marqueur position
      if (position_marker) {
        lv_obj_del(position_marker);
      }
      position_marker = lv_label_create(map_container);
      lv_label_set_text(position_marker, LV_SYMBOL_GPS);
      lv_obj_set_style_text_font(position_marker, &lv_font_montserrat_32, 0);
      lv_obj_set_style_text_color(position_marker, lv_color_hex(UI_COLOR_GPS_MARKER), 0);
      lv_obj_set_style_bg_opa(position_marker, LV_OPA_TRANSP, 0);
      lv_obj_center(position_marker);
      lv_obj_move_foreground(position_marker);
    }
  }
}

// Initialisation ecran gauche
void ui_screen_left_init(void) {
  // Nettoyer l'ecran s'il existe
  if (current_screen != NULL) {
    lv_obj_clean(current_screen);
  } else {
    current_screen = lv_obj_create(NULL);
  }

  lv_obj_set_style_bg_color(current_screen, lv_color_hex(UI_COLOR_BACKGROUND), 0);
  lv_obj_clear_flag(current_screen, LV_OBJ_FLAG_SCROLLABLE);

  // Barre de statut
  ui_create_status_bar(current_screen);

  lv_obj_t *frame = lv_obj_create(current_screen);
  lv_obj_set_size(frame, LCD_H_RES, LCD_V_RES - 60);
  lv_obj_align(frame, LV_ALIGN_TOP_MID, 0, 60);
  lv_obj_set_style_bg_color(frame, lv_color_hex(UI_COLOR_BACKGROUND), 0);
  lv_obj_set_style_bg_opa(frame, LV_OPA_COVER, 0);
  lv_obj_set_style_border_width(frame, 3, 0);
  lv_obj_set_style_border_color(frame, lv_color_hex(UI_COLOR_BORDER_PRIMARY), 0);
  lv_obj_set_style_radius(frame, UI_RADIUS_SMALL, 0);
  lv_obj_set_style_pad_all(frame, 20, 0);
  lv_obj_clear_flag(frame, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_clear_flag(frame, LV_OBJ_FLAG_CLICKABLE);

  // Texte identifiant
  lv_obj_t *label = lv_label_create(frame);
  lv_label_set_text(label, "Ecran Gauche");
  lv_obj_set_style_text_font(label, &lv_font_montserrat_32, 0);
  lv_obj_set_style_text_color(label, lv_color_hex(UI_COLOR_PRIMARY), 0);
  lv_obj_center(label);

  // Ajouter handler swipe sur l'ecran complet
  lv_obj_add_event_cb(current_screen, swipe_event_handler, LV_EVENT_PRESSED, NULL);
  lv_obj_add_event_cb(current_screen, swipe_event_handler, LV_EVENT_RELEASED, NULL);
  if (lvgl_port_lock(-1)) {
    lv_screen_load(current_screen);
    lvgl_port_unlock();
  }

#ifdef DEBUG_MODE
  Serial.println("Left screen initialized");
#endif
}

// Initialisation ecran central
void ui_screen_center_init(void) {
  // Nettoyer l'ecran s'il existe
  if (current_screen != NULL) {
    lv_obj_clean(current_screen);
  } else {
    current_screen = lv_obj_create(NULL);
  }

  lv_obj_set_style_bg_color(current_screen, lv_color_hex(UI_COLOR_BACKGROUND), 0);
   lv_obj_clear_flag(current_screen, LV_OBJ_FLAG_SCROLLABLE);

  // Barre de statut
  ui_create_status_bar(current_screen);

  lv_obj_t *frame = lv_obj_create(current_screen);
  lv_obj_set_size(frame, LCD_H_RES, LCD_V_RES - 55);
  lv_obj_align(frame, LV_ALIGN_TOP_MID, 0, 55);
  lv_obj_set_style_bg_color(frame, lv_color_hex(UI_COLOR_BACKGROUND), 0);
  lv_obj_set_style_bg_opa(frame, LV_OPA_COVER, 0);
  lv_obj_set_style_border_width(frame, 0, 0);
  lv_obj_set_style_border_color(frame, lv_color_hex(UI_COLOR_BACKGROUND), 0);
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
  lv_obj_set_style_bg_color(col_left, lv_color_hex(UI_COLOR_BACKGROUND), 0);
  lv_obj_set_style_bg_opa(col_left, LV_OPA_COVER, 0);
  lv_obj_set_style_border_width(col_left, 2, 0);
  lv_obj_set_style_border_color(col_left, lv_color_hex(UI_COLOR_BORDER_PRIMARY), 0);
  lv_obj_set_style_radius(col_left, UI_RADIUS_SMALL, 0);
  lv_obj_set_style_pad_all(col_left, 10, 0);
  lv_obj_clear_flag(col_left, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_clear_flag(col_left, LV_OBJ_FLAG_CLICKABLE);

  lv_obj_t *label_left = lv_label_create(col_left);
  lv_label_set_text(label_left, "Gauche");
  lv_obj_set_style_text_font(label_left, UI_FONT_NORMAL, 0);
  lv_obj_set_style_text_color(label_left, lv_color_hex(UI_COLOR_PRIMARY), 0);
  lv_obj_center(label_left);


  // Colonne centrale (carte)
  map_container = lv_obj_create(frame);
  lv_obj_set_size(map_container, col_center_width, col_height);
  lv_obj_set_pos(map_container, col_side_width + 5, 5);
  lv_obj_set_style_bg_color(map_container, lv_color_hex(UI_COLOR_BACKGROUND), 0);
  lv_obj_set_style_bg_opa(map_container, LV_OPA_COVER, 0);
  lv_obj_set_style_border_width(map_container, 2, 0);
  lv_obj_set_style_border_color(map_container, lv_color_hex(UI_COLOR_BORDER_PRIMARY), 0);
  lv_obj_set_style_radius(map_container, UI_RADIUS_SMALL, 0);
  lv_obj_set_style_pad_all(map_container, 10, 0);
  lv_obj_clear_flag(map_container, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_clear_flag(map_container, LV_OBJ_FLAG_CLICKABLE);

  // Initialiser zoom depuis parametres
  current_map_zoom = params.map_zoom;

// Affichage carte OSM initial
#ifdef FLIGHT_TEST_MODE
  map_canvas = create_map_view(map_container, TEST_LAT, TEST_LON,
                               current_map_zoom, 527, 527);
#else
  double display_lat = g_sensor_data.gps.valid ? g_sensor_data.gps.latitude : TEST_LAT;
  double display_lon = g_sensor_data.gps.valid ? g_sensor_data.gps.longitude : TEST_LON;
  map_canvas = create_map_view(map_container, display_lat, display_lon,
                               current_map_zoom, 527, 527);
#endif

  if (map_canvas) {
    lv_obj_align(map_canvas, LV_ALIGN_CENTER, 0, 0);
  }

  // Boutons zoom
  btn_zoom_in = lv_btn_create(map_container);
  lv_obj_set_size(btn_zoom_in, 50, 50);
  lv_obj_align(btn_zoom_in, LV_ALIGN_TOP_RIGHT, -10, 10);
  lv_obj_t *label_zoom_in = lv_label_create(btn_zoom_in);
  lv_label_set_text(label_zoom_in, LV_SYMBOL_PLUS);
  lv_obj_center(label_zoom_in);
  lv_obj_add_event_cb(btn_zoom_in, btn_zoom_in_cb, LV_EVENT_CLICKED, NULL);

  btn_zoom_out = lv_btn_create(map_container);
  lv_obj_set_size(btn_zoom_out, 50, 50);
  lv_obj_align(btn_zoom_out, LV_ALIGN_TOP_RIGHT, -10, 70);
  lv_obj_t *label_zoom_out = lv_label_create(btn_zoom_out);
  lv_label_set_text(label_zoom_out, LV_SYMBOL_MINUS);
  lv_obj_center(label_zoom_out);
  lv_obj_add_event_cb(btn_zoom_out, btn_zoom_out_cb, LV_EVENT_CLICKED, NULL);

  update_zoom_buttons_state();

  // Colonne droite
  lv_obj_t *col_right = lv_obj_create(frame);
  lv_obj_set_size(col_right, col_side_width, col_height);
  lv_obj_set_pos(col_right, col_side_width + col_center_width + 10, 5);
  lv_obj_set_style_bg_color(col_right, lv_color_hex(UI_COLOR_BACKGROUND), 0);
  lv_obj_set_style_bg_opa(col_right, LV_OPA_COVER, 0);
  lv_obj_set_style_border_width(col_right, 2, 0);
  lv_obj_set_style_border_color(col_right, lv_color_hex(UI_COLOR_BORDER_PRIMARY), 0);
  lv_obj_set_style_radius(col_right, UI_RADIUS_SMALL, 0);
  lv_obj_set_style_pad_all(col_right, 10, 0);
  lv_obj_clear_flag(col_right, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_clear_flag(col_right, LV_OBJ_FLAG_CLICKABLE);

  lv_obj_t *label_right = lv_label_create(col_right);
  lv_label_set_text(label_right, "Droite");
  lv_obj_set_style_text_font(label_right, UI_FONT_NORMAL, 0);
  lv_obj_set_style_text_color(label_right, lv_color_hex(UI_COLOR_PRIMARY), 0);
  lv_obj_center(label_right);

  // Marqueur position sur carte
  position_marker = lv_label_create(map_container);
  lv_label_set_text(position_marker, LV_SYMBOL_GPS);
  lv_obj_set_style_text_font(position_marker, &lv_font_montserrat_32, 0);
  lv_obj_set_style_text_color(position_marker, lv_color_hex(UI_COLOR_GPS_MARKER), 0);  // Bleu fonce
  lv_obj_set_style_bg_opa(position_marker, LV_OPA_TRANSP, 0);
  lv_obj_center(position_marker);
  lv_obj_move_foreground(position_marker);

  // Ajouter handler swipe sur l'ecran complet
  lv_obj_add_event_cb(current_screen, swipe_event_handler, LV_EVENT_PRESSED, NULL);
  lv_obj_add_event_cb(current_screen, swipe_event_handler, LV_EVENT_RELEASED, NULL);

  if (lvgl_port_lock(-1)) {
    lv_screen_load(current_screen);
    lvgl_port_unlock();
  }
#ifdef DEBUG_MODE
  Serial.println("Center screen initialized");
#endif
}

// Initialisation ecran droite
void ui_screen_right_init(void) {
  // Nettoyer l'ecran s'il existe
  if (current_screen != NULL) {
    lv_obj_clean(current_screen);
  } else {
    current_screen = lv_obj_create(NULL);
  }

  lv_obj_set_style_bg_color(current_screen, lv_color_hex(UI_COLOR_BACKGROUND), 0);
  lv_obj_clear_flag(current_screen, LV_OBJ_FLAG_SCROLLABLE);

  // Barre de statut
  ui_create_status_bar(current_screen);

  lv_obj_t *frame = lv_obj_create(current_screen);
  lv_obj_set_size(frame, LCD_H_RES, LCD_V_RES - 60);
  lv_obj_align(frame, LV_ALIGN_TOP_MID, 0, 60);
  lv_obj_set_style_bg_color(frame, lv_color_hex(UI_COLOR_BACKGROUND), 0);
  lv_obj_set_style_bg_opa(frame, LV_OPA_COVER, 0);
  lv_obj_set_style_border_width(frame, 3, 0);
  lv_obj_set_style_border_color(frame, lv_color_hex(UI_COLOR_BORDER_PRIMARY), 0);
  lv_obj_set_style_radius(frame, UI_RADIUS_SMALL, 0);
  lv_obj_set_style_pad_all(frame, 20, 0);
  lv_obj_clear_flag(frame, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_clear_flag(frame, LV_OBJ_FLAG_CLICKABLE);

  // Texte identifiant
  lv_obj_t *label = lv_label_create(frame);
  lv_label_set_text(label, "Ecran Droite");
  lv_obj_set_style_text_font(label, &lv_font_montserrat_32, 0);
  lv_obj_set_style_text_color(label, lv_color_hex(UI_COLOR_PRIMARY), 0);
  lv_obj_center(label);

  // Ajouter handler swipe sur l'ecran complet
  lv_obj_add_event_cb(current_screen, swipe_event_handler, LV_EVENT_PRESSED, NULL);
  lv_obj_add_event_cb(current_screen, swipe_event_handler, LV_EVENT_RELEASED, NULL);

  if (lvgl_port_lock(-1)) {
    lv_screen_load(current_screen);
    lvgl_port_unlock();
  }
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
  lv_obj_t *old_screen = lv_scr_act();
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
  // Detruire l'ancien ecran SI ce n'est pas le meme
  if (old_screen != current_screen && old_screen != NULL) {
    lv_obj_del(old_screen);
#ifdef DEBUG_MODE
    Serial.println("[PRESTART] Old screen deleted");
#endif
  }
#ifdef DEBUG_MODE
  Serial.println("Main screens displayed");
#endif
}

#endif