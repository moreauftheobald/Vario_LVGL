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
static int current_map_zoom = 0;
static lv_obj_t *map_canvas = NULL;
static lv_obj_t *map_container = NULL;
static lv_obj_t *btn_zoom_in = NULL;
static lv_obj_t *btn_zoom_out = NULL;
static lv_obj_t *position_marker = NULL;

// Fonction pour changer d'ecran
static void switch_to_screen(uint8_t index) {
  current_screen_index = index;

  // Recréer l'écran selon l'index
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

static lv_obj_t *create_simple_position_marker(lv_obj_t *parent, float heading_deg) {
  // Container pour le marqueur (30x30 pixels)
  lv_obj_t *container = lv_obj_create(parent);
  lv_obj_set_size(container, 30, 30);
  lv_obj_set_style_bg_opa(container, LV_OPA_TRANSP, 0);
  lv_obj_set_style_border_width(container, 0, 0);
  lv_obj_clear_flag(container, LV_OBJ_FLAG_CLICKABLE);
  lv_obj_clear_flag(container, LV_OBJ_FLAG_SCROLLABLE);

  // METHODE 1 : Label avec symbole triangle Unicode
  lv_obj_t *arrow = lv_label_create(container);
  lv_label_set_text(arrow, "▲");  // Triangle Unicode
  lv_obj_set_style_text_font(arrow, &lv_font_montserrat_48, 0);
  lv_obj_set_style_text_color(arrow, lv_color_hex(0x2196F3), 0);  // Bleu
  lv_obj_center(arrow);

  // Rotation du label (en 0.1 degres pour LVGL)
  lv_obj_set_style_transform_angle(arrow, (int16_t)(heading_deg * 10), 0);
  lv_obj_set_style_transform_pivot_x(arrow, 14, 0);  // Centre de rotation
  lv_obj_set_style_transform_pivot_y(arrow, 14, 0);

  // Point rouge au centre (position GPS exacte)
  lv_obj_t *center_dot = lv_obj_create(container);
  lv_obj_set_size(center_dot, 6, 6);
  lv_obj_center(center_dot);
  lv_obj_set_style_radius(center_dot, LV_RADIUS_CIRCLE, 0);
  lv_obj_set_style_bg_color(center_dot, lv_color_hex(0xFF0000), 0);  // Rouge
  lv_obj_set_style_border_width(center_dot, 1, 0);
  lv_obj_set_style_border_color(center_dot, lv_color_hex(0xFFFFFF), 0);  // Blanc
  lv_obj_clear_flag(center_dot, LV_OBJ_FLAG_CLICKABLE);

  return container;
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
#ifdef DEBUG_MODE
      Serial.printf("Touch start: x=%d, y=%d\n", touch_start_point.x, touch_start_point.y);
#endif
    }
  } else if (code == LV_EVENT_RELEASED) {
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

// Fonction pour mettre a jour l'etat des boutons zoom
static void update_zoom_buttons_state(void) {
  if (btn_zoom_in) {
    if (current_map_zoom >= MAP_ZOOM_MAX + 1) {
      lv_obj_add_state(btn_zoom_in, LV_STATE_DISABLED);
      lv_obj_set_style_bg_color(btn_zoom_in, lv_color_hex(0x808080), LV_STATE_DISABLED);
    } else {
      lv_obj_clear_state(btn_zoom_in, LV_STATE_DISABLED);
      lv_obj_set_style_bg_color(btn_zoom_in, lv_color_hex(0x2196F3), 0);
    }
  }

  if (btn_zoom_out) {
    if (current_map_zoom <= MAP_ZOOM_MIN) {
      lv_obj_add_state(btn_zoom_out, LV_STATE_DISABLED);
      lv_obj_set_style_bg_color(btn_zoom_out, lv_color_hex(0x808080), LV_STATE_DISABLED);
    } else {
      lv_obj_clear_state(btn_zoom_out, LV_STATE_DISABLED);
      lv_obj_set_style_bg_color(btn_zoom_out, lv_color_hex(0x2196F3), 0);
    }
  }
}

// Callbacks pour boutons zoom
static void btn_zoom_in_cb(lv_event_t *e) {
  if (current_map_zoom < MAP_ZOOM_MAX + 1) {  // +1 pour super zoom
    current_map_zoom++;

    // Supprimer ancien canvas
    if (map_canvas) {
      lv_obj_del(map_canvas);
      map_canvas = NULL;
    }

    // Recreer carte avec nouveau zoom
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
    float heading = g_sensor_data.gps.valid ? g_sensor_data.gps.angle : 0.0f;
    position_marker = create_simple_position_marker(map_container, heading);
    lv_obj_center(position_marker);
    lv_obj_move_foreground(position_marker);

    // Mettre a jour l'etat des boutons
    update_zoom_buttons_state();

#ifdef DEBUG_MODE
    Serial.printf("Zoom IN: %d\n", current_map_zoom);
#endif
  }
}

static void btn_zoom_out_cb(lv_event_t *e) {
  if (current_map_zoom > MAP_ZOOM_MIN) {
    current_map_zoom--;

    // Supprimer ancien canvas
    if (map_canvas) {
      lv_obj_del(map_canvas);
      map_canvas = NULL;
    }

    // Recreer carte avec nouveau zoom
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
    float heading = g_sensor_data.gps.valid ? g_sensor_data.gps.angle : 0.0f;
    position_marker = create_simple_position_marker(map_container, heading);
    lv_obj_center(position_marker);
    lv_obj_move_foreground(position_marker);

    // Mettre a jour l'etat des boutons
    update_zoom_buttons_state();

#ifdef DEBUG_MODE
    Serial.printf("Zoom OUT: %d\n", current_map_zoom);
#endif
  }
}

static void update_position_marker_timer_cb(lv_timer_t *timer) {
  if (!position_marker || !g_sensor_data.gps.valid) return;

  float heading = g_sensor_data.gps.valid ? g_sensor_data.gps.angle : 0.0f;
  position_marker = create_simple_position_marker(map_container, heading);
  lv_obj_center(position_marker);
  lv_obj_move_foreground(position_marker);
}

// Initialisation ecran gauche
void ui_screen_left_init(void) {
  // Nettoyer l'ecran s'il existe
  if (current_screen != NULL) {
    lv_obj_clean(current_screen);
  } else {
    current_screen = lv_obj_create(NULL);
  }

  lv_obj_set_style_bg_color(current_screen, lv_color_hex(0x000000), 0);
  lv_obj_clear_flag(current_screen, LV_OBJ_FLAG_SCROLLABLE);

  // Barre de statut
  ui_create_status_bar(current_screen);

  lv_obj_t *frame = lv_obj_create(current_screen);
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

  lv_obj_set_style_bg_color(current_screen, lv_color_hex(0x000000), 0);
  lv_obj_clear_flag(current_screen, LV_OBJ_FLAG_SCROLLABLE);

  // Barre de statut
  ui_create_status_bar(current_screen);

  lv_obj_t *frame = lv_obj_create(current_screen);
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
  map_container = lv_obj_create(frame);
  lv_obj_set_size(map_container, col_center_width, col_height);
  lv_obj_set_pos(map_container, col_side_width + 5, 5);
  lv_obj_set_style_bg_color(map_container, lv_color_hex(0x000000), 0);
  lv_obj_set_style_bg_opa(map_container, LV_OPA_COVER, 0);
  lv_obj_set_style_border_width(map_container, 2, 0);
  lv_obj_set_style_border_color(map_container, lv_color_hex(0xFFFFFF), 0);
  lv_obj_set_style_radius(map_container, ROUND_FRANE_RADUIS_SMALL, 0);
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

  // Boutons zoom (haut droite)
  btn_zoom_in = lv_btn_create(map_container);
  lv_obj_set_size(btn_zoom_in, 50, 50);
  lv_obj_align(btn_zoom_in, LV_ALIGN_TOP_RIGHT, -5, 5);
  lv_obj_set_style_bg_color(btn_zoom_in, lv_color_hex(0x2196F3), 0);
  lv_obj_set_style_radius(btn_zoom_in, 8, 0);
  lv_obj_add_event_cb(btn_zoom_in, btn_zoom_in_cb, LV_EVENT_CLICKED, NULL);

  lv_obj_t *label_plus = lv_label_create(btn_zoom_in);
  lv_label_set_text(label_plus, "+");
  lv_obj_set_style_text_font(label_plus, &lv_font_montserrat_32, 0);
  lv_obj_center(label_plus);

  btn_zoom_out = lv_btn_create(map_container);
  lv_obj_set_size(btn_zoom_out, 50, 50);
  lv_obj_align(btn_zoom_out, LV_ALIGN_TOP_RIGHT, -5, 60);
  lv_obj_set_style_bg_color(btn_zoom_out, lv_color_hex(0x2196F3), 0);
  lv_obj_set_style_radius(btn_zoom_out, 8, 0);
  lv_obj_add_event_cb(btn_zoom_out, btn_zoom_out_cb, LV_EVENT_CLICKED, NULL);

  lv_obj_t *label_minus = lv_label_create(btn_zoom_out);
  lv_label_set_text(label_minus, "-");
  lv_obj_set_style_text_font(label_minus, &lv_font_montserrat_32, 0);
  lv_obj_center(label_minus);

  // Mettre a jour l'etat initial des boutons
  update_zoom_buttons_state();


  // Marqueur position parapente (centre carte) avec orientation GPS
  float heading = g_sensor_data.gps.valid ? g_sensor_data.gps.angle : 0.0f;
  position_marker = create_simple_position_marker(map_container, heading);
  lv_obj_center(position_marker);
  lv_obj_move_foreground(position_marker);

  // Ajouter handler swipe sur l'ecran complet
  lv_obj_add_event_cb(current_screen, swipe_event_handler, LV_EVENT_PRESSED, NULL);
  lv_obj_add_event_cb(current_screen, swipe_event_handler, LV_EVENT_RELEASED, NULL);

  lv_timer_t *marker_timer = lv_timer_create(update_position_marker_timer_cb, 500, NULL);

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

  lv_obj_set_style_bg_color(current_screen, lv_color_hex(0x000000), 0);
  //lv_obj_set_style_bg_grad_color(current_screen, lv_color_hex(0x1a1f3a), 0);
  //lv_obj_set_style_bg_grad_dir(current_screen, LV_GRAD_DIR_VER, 0);
  lv_obj_clear_flag(current_screen, LV_OBJ_FLAG_SCROLLABLE);

  // Barre de statut
  ui_create_status_bar(current_screen);

  lv_obj_t *frame = lv_obj_create(current_screen);
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
  lv_label_set_text(label, "Ecran Droite");
  lv_obj_set_style_text_font(label, &lv_font_montserrat_32, 0);
  lv_obj_set_style_text_color(label, lv_color_hex(0x00d4ff), 0);
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
  // Détruire l'ancien écran SI ce n'est pas le même
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