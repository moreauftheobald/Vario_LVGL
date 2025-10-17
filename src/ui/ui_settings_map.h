#ifndef UI_SETTINGS_MAP_H
#define UI_SETTINGS_MAP_H

#include "lvgl.h"
#include "constants.h"
#include "UI_helper.h"
#include "lang.h"
#include "globals.h"

void ui_settings_show(void);

static lv_obj_t *screen_settings_map = NULL;
static lv_obj_t *slider_zoom = NULL;
static lv_obj_t *dropdown_tile_server = NULL;
static lv_obj_t *slider_track_points = NULL;
static lv_obj_t *switch_vario_colors = NULL;
static lv_obj_t *switch_indicator = NULL;
static lv_obj_t *label_zoom_value = NULL;
static lv_obj_t *label_track_value = NULL;

// Structure des serveurs de tuiles
typedef struct {
  const char* name;
  const char* url;
} TileServer;

static const TileServer tile_servers[] = {
  {"IGN", "http://{a-c}.tile.opentopomap.org/{z}/{x}/{y}.png"},
  {"Mappy", "https://map3.mappy.net/map/1.0/slab/standard_hd/256/{z}/{x}/{y}"},
  {"OSM Standart", "http://{a-c}.tile.openstreetmap.fr/osmfr/{z}/{x}/{y}.png"},
  {"OSM Humanitarian", "http://{a-b}.tile.openstreetmap.fr/hot/{z}/{x}/{y}.png"},
  {"Outdoor Interactiv Winter", "https://w2.outdooractive.com/map/v1/png/osm_winter/{z}/{x}/{y}/t.png"},
  {"Outdoor Interactiv Summer", "https://w3.outdooractive.com/map/v1/png/osm/{z}/{x}/{y}/t.png"},
  {"Stamen terrain", "https://tiles.stadiamaps.com/tiles/stamen_terrain/{z}/{x}/{y}.png"}
};

static const int tile_servers_count = sizeof(tile_servers) / sizeof(TileServer);

// Fonctions de sauvegarde/chargement
static void load_map_settings(void) {
  prefs.begin("map", true);
  
  int zoom = prefs.getInt("zoom", 15);
  int server = prefs.getInt("server", 0);
  int track_pts = prefs.getInt("track_pts", 50);
  bool vario_col = prefs.getBool("vario_col", true);
  
  if (slider_zoom) {
    lv_slider_set_value(slider_zoom, zoom, LV_ANIM_OFF);
    if (label_zoom_value) {
      lv_label_set_text_fmt(label_zoom_value, "%d", zoom);
    }
  }
  
  if (dropdown_tile_server) {
    lv_dropdown_set_selected(dropdown_tile_server, server);
  }
  
  if (slider_track_points) {
    lv_slider_set_value(slider_track_points, track_pts, LV_ANIM_OFF);
    if (label_track_value) {
      lv_label_set_text_fmt(label_track_value, "%d", track_pts);
    }
  }
  
  if (switch_vario_colors) {
    if (vario_col) {
      lv_obj_add_state(switch_vario_colors, LV_STATE_CHECKED);
    } else {
      lv_obj_clear_state(switch_vario_colors, LV_STATE_CHECKED);
    }
  }
  
  prefs.end();
  
#ifdef DEBUG_MODE
  Serial.println("Map settings loaded");
  Serial.printf("Zoom: %d, Server: %d, Track points: %d, Vario colors: %d\n", 
                zoom, server, track_pts, vario_col);
#endif
}

static void save_map_settings(void) {
  prefs.begin("map", false);
  
  int zoom = (int)lv_slider_get_value(slider_zoom);
  int server = lv_dropdown_get_selected(dropdown_tile_server);
  int track_pts = (int)lv_slider_get_value(slider_track_points);
  bool vario_col = lv_obj_has_state(switch_vario_colors, LV_STATE_CHECKED);
  
  prefs.putInt("zoom", zoom);
  prefs.putInt("server", server);
  prefs.putInt("track_pts", track_pts);
  prefs.putBool("vario_col", vario_col);
  
  prefs.end();
  
#ifdef DEBUG_MODE
  Serial.println("Map settings saved");
  Serial.printf("Zoom: %d, Server: %d, Track points: %d, Vario colors: %d\n", 
                zoom, server, track_pts, vario_col);
#endif
}

// Callbacks
static void slider_zoom_event_cb(lv_event_t *e) {
  lv_event_code_t code = lv_event_get_code(e);
  if (code == LV_EVENT_VALUE_CHANGED) {
    int value = (int)lv_slider_get_value(slider_zoom);
    lv_label_set_text_fmt(label_zoom_value, "%d", value);
    
#ifdef DEBUG_MODE
    Serial.printf("Zoom changed to: %d\n", value);
#endif
  }
}

static void slider_track_event_cb(lv_event_t *e) {
  lv_event_code_t code = lv_event_get_code(e);
  if (code == LV_EVENT_VALUE_CHANGED) {
    int value = (int)lv_slider_get_value(slider_track_points);
    lv_label_set_text_fmt(label_track_value, "%d", value);
    
#ifdef DEBUG_MODE
    Serial.printf("Track points changed to: %d\n", value);
#endif
  }
}

static void dropdown_map_event_cb(lv_event_t *e) {
  lv_event_code_t code = lv_event_get_code(e);
  if (code == LV_EVENT_VALUE_CHANGED) {
    int selected = lv_dropdown_get_selected(dropdown_tile_server);
    
#ifdef DEBUG_MODE
    Serial.printf("Tile server changed to: %s\n", tile_servers[selected].name);
#endif
  }
}

static void switch_vario_event_cb(lv_event_t *e) {
  lv_event_code_t code = lv_event_get_code(e);
  if (code == LV_EVENT_VALUE_CHANGED) {
    bool checked = lv_obj_has_state(switch_vario_colors, LV_STATE_CHECKED);
    
    // Déplacer l'indicateur
    if (switch_indicator) {
      if (checked) {
        lv_obj_align(switch_indicator, LV_ALIGN_RIGHT_MID, -2, 0);
      } else {
        lv_obj_align(switch_indicator, LV_ALIGN_LEFT_MID, 2, 0);
      }
    }
    
#ifdef DEBUG_MODE
    Serial.printf("Vario colors: %s\n", checked ? "ON" : "OFF");
#endif
  }
}

static void btn_save_map_cb(lv_event_t *e) {
#ifdef DEBUG_MODE
  Serial.println("Save map settings clicked");
#endif
  save_map_settings();
  ui_settings_show();
}

static void btn_cancel_map_cb(lv_event_t *e) {
#ifdef DEBUG_MODE
  Serial.println("Cancel map settings clicked");
#endif
  ui_settings_show();
}

void ui_settings_map_init(void) {
  const TextStrings *txt = get_text();

  // Ecran et frame
  screen_settings_map = ui_create_screen();
  lv_obj_t *main_frame = ui_create_main_frame(screen_settings_map);
  lv_obj_clear_flag(main_frame, LV_OBJ_FLAG_SCROLLABLE);

  // Titre
  lv_obj_t *label_title = ui_create_label(main_frame, txt->map_settings,
                                           &lv_font_montserrat_32, lv_color_hex(0x00d4ff));
  lv_obj_align(label_title, LV_ALIGN_TOP_MID, 0, 0);

  // Container principal (2 colonnes)
  lv_obj_t *main_container = lv_obj_create(main_frame);
  lv_obj_set_size(main_container, lv_pct(100), 420);
  lv_obj_align(main_container, LV_ALIGN_TOP_MID, 0, 45);
  lv_obj_set_style_bg_opa(main_container, LV_OPA_TRANSP, 0);
  lv_obj_set_style_border_width(main_container, 0, 0);
  lv_obj_set_style_pad_all(main_container, 0, 0);
  lv_obj_set_flex_flow(main_container, LV_FLEX_FLOW_ROW);
  lv_obj_set_flex_align(main_container, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
  lv_obj_clear_flag(main_container, LV_OBJ_FLAG_SCROLLABLE);

  // Colonne gauche (parametres)
  lv_obj_t *col_left = lv_obj_create(main_container);
  lv_obj_set_size(col_left, 480, 420);
  lv_obj_set_style_bg_color(col_left, lv_color_hex(0x1a2035), 0);
  lv_obj_set_style_bg_opa(col_left, LV_OPA_80, 0);
  lv_obj_set_style_border_width(col_left, 2, 0);
  lv_obj_set_style_border_color(col_left, lv_color_hex(0x6080a0), 0);
  lv_obj_set_style_radius(col_left, 15, 0);
  lv_obj_set_style_pad_all(col_left, 15, 0);
  lv_obj_set_flex_flow(col_left, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_flex_align(col_left, LV_FLEX_ALIGN_SPACE_EVENLY, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
  lv_obj_clear_flag(col_left, LV_OBJ_FLAG_SCROLLABLE);

  // 1. Zoom par defaut
  lv_obj_t *label_zoom = ui_create_label(col_left, "Zoom par defaut",
                                          &lv_font_montserrat_20, lv_color_hex(0x00d4ff));
  
  lv_obj_t *zoom_container = ui_create_flex_container(col_left, LV_FLEX_FLOW_ROW);
  lv_obj_set_width(zoom_container, lv_pct(100));
  lv_obj_set_flex_align(zoom_container, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
  lv_obj_set_style_pad_column(zoom_container, 10, 0);
  
  slider_zoom = lv_slider_create(zoom_container);
  lv_obj_set_size(slider_zoom, 350, 15);
  lv_slider_set_range(slider_zoom, 5, 20);
  lv_slider_set_value(slider_zoom, 15, LV_ANIM_OFF);
  lv_obj_set_style_bg_color(slider_zoom, lv_color_hex(0x2a3f5f), LV_PART_MAIN);
  lv_obj_set_style_bg_color(slider_zoom, lv_color_hex(0x00d4ff), LV_PART_INDICATOR);
  lv_obj_set_style_bg_color(slider_zoom, lv_color_hex(0x00d4ff), LV_PART_KNOB);
  lv_obj_add_event_cb(slider_zoom, slider_zoom_event_cb, LV_EVENT_VALUE_CHANGED, NULL);
  
  label_zoom_value = ui_create_label(zoom_container, "15",
                                      &lv_font_montserrat_20, lv_color_white());
  lv_obj_set_width(label_zoom_value, 40);
  lv_label_set_text_fmt(label_zoom_value, "15");

  // 2. Fond de carte
  lv_obj_t *label_server = ui_create_label(col_left, "Fond de carte",
                                            &lv_font_montserrat_20, lv_color_hex(0x00d4ff));
  
  dropdown_tile_server = lv_dropdown_create(col_left);
  lv_obj_set_width(dropdown_tile_server, lv_pct(100));
  
  // Construction de la liste des serveurs
  String server_list = "";
  for (int i = 0; i < tile_servers_count; i++) {
    server_list += tile_servers[i].name;
    if (i < tile_servers_count - 1) {
      server_list += "\n";
    }
  }
  lv_dropdown_set_options(dropdown_tile_server, server_list.c_str());
  lv_obj_set_style_bg_color(dropdown_tile_server, lv_color_hex(0x1a2035), 0);
  lv_obj_set_style_border_color(dropdown_tile_server, lv_color_hex(0x6080a0), 0);
  lv_obj_set_style_text_font(dropdown_tile_server, &lv_font_montserrat_20, 0);
  lv_obj_add_event_cb(dropdown_tile_server, dropdown_map_event_cb, LV_EVENT_VALUE_CHANGED, NULL);

  // 3. Points de trace
  lv_obj_t *label_track = ui_create_label(col_left, "Points de trace affiches",
                                           &lv_font_montserrat_20, lv_color_hex(0x00d4ff));
  
  lv_obj_t *track_container = ui_create_flex_container(col_left, LV_FLEX_FLOW_ROW);
  lv_obj_set_width(track_container, lv_pct(100));
  lv_obj_set_flex_align(track_container, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
  lv_obj_set_style_pad_column(track_container, 10, 0);
  
  slider_track_points = lv_slider_create(track_container);
  lv_obj_set_size(slider_track_points, 350, 15);
  lv_slider_set_range(slider_track_points, 0, 100);
  lv_slider_set_value(slider_track_points, 50, LV_ANIM_OFF);
  lv_obj_set_style_bg_color(slider_track_points, lv_color_hex(0x2a3f5f), LV_PART_MAIN);
  lv_obj_set_style_bg_color(slider_track_points, lv_color_hex(0x00d4ff), LV_PART_INDICATOR);
  lv_obj_set_style_bg_color(slider_track_points, lv_color_hex(0x00d4ff), LV_PART_KNOB);
  lv_obj_add_event_cb(slider_track_points, slider_track_event_cb, LV_EVENT_VALUE_CHANGED, NULL);
  
  label_track_value = ui_create_label(track_container, "50",
                                       &lv_font_montserrat_20, lv_color_white());
  lv_obj_set_width(label_track_value, 40);
  lv_label_set_text_fmt(label_track_value, "50");

  // 4. Vario colorimetrique
  lv_obj_t *vario_container = ui_create_flex_container(col_left, LV_FLEX_FLOW_ROW);
  lv_obj_set_width(vario_container, lv_pct(100));
  lv_obj_set_flex_align(vario_container, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
  
  lv_obj_t *label_vario = ui_create_label(vario_container, "Vario colorimetrique",
                                           &lv_font_montserrat_20, lv_color_hex(0x00d4ff));
  
  // Création du switch personnalisé
  switch_vario_colors = lv_obj_create(vario_container);
  lv_obj_set_size(switch_vario_colors, 60, 30);
  lv_obj_set_style_radius(switch_vario_colors, 15, 0);
  lv_obj_set_style_bg_color(switch_vario_colors, lv_color_hex(0x00d4ff), LV_STATE_CHECKED);
  lv_obj_set_style_bg_color(switch_vario_colors, lv_color_hex(0x3a3a3a), LV_STATE_DEFAULT);
  lv_obj_set_style_border_width(switch_vario_colors, 2, 0);
  lv_obj_set_style_border_color(switch_vario_colors, lv_color_hex(0x6080a0), 0);
  lv_obj_set_style_pad_all(switch_vario_colors, 2, 0);
  lv_obj_clear_flag(switch_vario_colors, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_add_flag(switch_vario_colors, LV_OBJ_FLAG_CHECKABLE);
  lv_obj_add_flag(switch_vario_colors, LV_OBJ_FLAG_CLICKABLE);
  lv_obj_add_state(switch_vario_colors, LV_STATE_CHECKED);
  
  // Indicateur (cercle qui glisse)
  switch_indicator = lv_obj_create(switch_vario_colors);
  lv_obj_set_size(switch_indicator, 24, 24);
  lv_obj_set_style_radius(switch_indicator, 12, 0);
  lv_obj_set_style_bg_color(switch_indicator, lv_color_white(), 0);
  lv_obj_set_style_border_width(switch_indicator, 0, 0);
  lv_obj_clear_flag(switch_indicator, LV_OBJ_FLAG_CLICKABLE);
  lv_obj_clear_flag(switch_indicator, LV_OBJ_FLAG_SCROLLABLE);
  
  // Position de l'indicateur selon l'état
  lv_obj_align(switch_indicator, LV_ALIGN_RIGHT_MID, -2, 0);
  
  lv_obj_add_event_cb(switch_vario_colors, switch_vario_event_cb, LV_EVENT_VALUE_CHANGED, NULL);

  // Colonne droite (apercu carte - pour plus tard)
  lv_obj_t *col_right = lv_obj_create(main_container);
  lv_obj_set_size(col_right, 480, 420);
  lv_obj_set_style_bg_color(col_right, lv_color_hex(0x1a2035), 0);
  lv_obj_set_style_bg_opa(col_right, LV_OPA_50, 0);
  lv_obj_set_style_border_width(col_right, 2, 0);
  lv_obj_set_style_border_color(col_right, lv_color_hex(0x6080a0), 0);
  lv_obj_set_style_radius(col_right, 15, 0);
  lv_obj_clear_flag(col_right, LV_OBJ_FLAG_SCROLLABLE);
  
  // Placeholder pour l'apercu
  lv_obj_t *label_preview = ui_create_label(col_right, "Apercu carte\n(bientot disponible)",
                                             &lv_font_montserrat_20, lv_color_hex(0x6080a0));
  lv_obj_center(label_preview);
  lv_obj_set_style_text_align(label_preview, LV_TEXT_ALIGN_CENTER, 0);

  // Boutons Enregistrer et Annuler
  lv_obj_t *buttons_container = ui_create_flex_container(main_frame, LV_FLEX_FLOW_ROW);
  lv_obj_set_width(buttons_container, 460);
  lv_obj_align(buttons_container, LV_ALIGN_BOTTOM_MID, 0, -15);
  lv_obj_set_flex_align(buttons_container, LV_FLEX_ALIGN_SPACE_EVENLY, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
  lv_obj_set_style_pad_column(buttons_container, 20, 0);

  lv_obj_t *btn_save = ui_create_simple_button(buttons_container, txt->save,
                                                 lv_color_hex(0x34c759), 220, 50);
  lv_obj_add_event_cb(btn_save, btn_save_map_cb, LV_EVENT_CLICKED, NULL);

  lv_obj_t *btn_cancel = ui_create_simple_button(buttons_container, txt->cancel,
                                                   lv_color_hex(0xff3b30), 220, 50);
  lv_obj_add_event_cb(btn_cancel, btn_cancel_map_cb, LV_EVENT_CLICKED, NULL);

  // Charger les valeurs sauvegardees
  load_map_settings();
}

void ui_settings_map_show(void) {
  if (screen_settings_map == NULL) {
    if (lvgl_port_lock(-1)) {
      ui_settings_map_init();
      lvgl_port_unlock();
    }
  }

  if (lvgl_port_lock(-1)) {
    lv_screen_load(screen_settings_map);
    force_full_refresh();
    lvgl_port_unlock();
  }

#ifdef DEBUG_MODE
  Serial.println("Map settings screen displayed");
#endif
}

#endif