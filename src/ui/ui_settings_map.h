#ifndef UI_SETTINGS_MAP_H
#define UI_SETTINGS_MAP_H

#include "lvgl.h"
#include "constants.h"
#include "UI_helper.h"
#include "lang.h"
#include "globals.h"
#include "src/params/params.h"

void ui_settings_show(void);

// Structure des serveurs de tuiles
typedef struct {
  const char* name;
  const char* url;
} TileServer;

// Strings en PROGMEM
static const char tile_name_0[] PROGMEM = "IGN";
static const char tile_url_0[] PROGMEM = "http://{a-c}.tile.opentopomap.org/{z}/{x}/{y}.png";

static const char tile_name_1[] PROGMEM = "Mappy";
static const char tile_url_1[] PROGMEM = "https://map3.mappy.net/map/1.0/slab/standard_hd/256/{z}/{x}/{y}";

static const char tile_name_2[] PROGMEM = "OSM Standart";
static const char tile_url_2[] PROGMEM = "http://{a-c}.tile.openstreetmap.fr/osmfr/{z}/{x}/{y}.png";

static const char tile_name_3[] PROGMEM = "OSM Humanitarian";
static const char tile_url_3[] PROGMEM = "http://{a-b}.tile.openstreetmap.fr/hot/{z}/{x}/{y}.png";

static const char tile_name_4[] PROGMEM = "Outdoor Interactiv Winter";
static const char tile_url_4[] PROGMEM = "https://w2.outdooractive.com/map/v1/png/osm_winter/{z}/{x}/{y}/t.png";

static const char tile_name_5[] PROGMEM = "Outdoor Interactiv Summer";
static const char tile_url_5[] PROGMEM = "https://w3.outdooractive.com/map/v1/png/osm/{z}/{x}/{y}/t.png";

static const char tile_name_6[] PROGMEM = "Stamen terrain";
static const char tile_url_6[] PROGMEM = "https://tiles.stadiamaps.com/tiles/stamen_terrain/{z}/{x}/{y}.png";

// Table en PROGMEM
static const TileServer tile_servers[] PROGMEM = {
  {tile_name_0, tile_url_0},
  {tile_name_1, tile_url_1},
  {tile_name_2, tile_url_2},
  {tile_name_3, tile_url_3},
  {tile_name_4, tile_url_4},
  {tile_name_5, tile_url_5},
  {tile_name_6, tile_url_6}
};

static const int tile_servers_count = sizeof(tile_servers) / sizeof(TileServer);

// Helper pour lire un nom de serveur depuis PROGMEM
static void get_tile_server_name(int index, char* buffer, size_t buf_size) {
  if (index >= 0 && index < tile_servers_count) {
    TileServer ts;
    memcpy_P(&ts, &tile_servers[index], sizeof(TileServer));
    strncpy_P(buffer, (PGM_P)ts.name, buf_size - 1);
    buffer[buf_size - 1] = '\0';
  } else {
    buffer[0] = '\0';
  }
}

// Helper pour lire une URL de serveur depuis PROGMEM
static void get_tile_server_url(int index, char* buffer, size_t buf_size) {
  if (index >= 0 && index < tile_servers_count) {
    TileServer ts;
    memcpy_P(&ts, &tile_servers[index], sizeof(TileServer));
    strncpy_P(buffer, (PGM_P)ts.url, buf_size - 1);
    buffer[buf_size - 1] = '\0';
  } else {
    buffer[0] = '\0';
  }
}

static void load_map_settings(lv_obj_t *slider_zoom, lv_obj_t *label_zoom_value,
                               lv_obj_t *dropdown_tile_server, lv_obj_t *slider_track_points, 
                               lv_obj_t *label_track_value, lv_obj_t *switch_vario_colors,
                               lv_obj_t *switch_indicator) {
  if (slider_zoom) {
    lv_slider_set_value(slider_zoom, params.map_zoom, LV_ANIM_OFF);
    lv_label_set_text_fmt(label_zoom_value, "%d", params.map_zoom);
  }
  
  if (dropdown_tile_server) {
    lv_dropdown_set_selected(dropdown_tile_server, params.map_tile_server);
  }
  
  if (slider_track_points) {
    lv_slider_set_value(slider_track_points, params.map_track_points, LV_ANIM_OFF);
    lv_label_set_text_fmt(label_track_value, "%d", params.map_track_points);
  }
  
  if (switch_vario_colors) {
    if (params.map_vario_colors) {
      lv_obj_add_state(switch_vario_colors, LV_STATE_CHECKED);
      if (switch_indicator) {
        lv_obj_align(switch_indicator, LV_ALIGN_RIGHT_MID, -2, 0);
      }
    } else {
      lv_obj_clear_state(switch_vario_colors, LV_STATE_CHECKED);
      if (switch_indicator) {
        lv_obj_align(switch_indicator, LV_ALIGN_LEFT_MID, 2, 0);
      }
    }
  }
  
#ifdef DEBUG_MODE
  Serial.println("Map settings loaded from params");
#endif
}

static void save_map_settings(lv_obj_t *slider_zoom, lv_obj_t *dropdown_tile_server,
                               lv_obj_t *slider_track_points, lv_obj_t *switch_vario_colors) {
  params.map_zoom = (int)lv_slider_get_value(slider_zoom);
  params.map_tile_server = lv_dropdown_get_selected(dropdown_tile_server);
  params.map_track_points = (int)lv_slider_get_value(slider_track_points);
  params.map_vario_colors = lv_obj_has_state(switch_vario_colors, LV_STATE_CHECKED);
  
  params_save_map();
  
#ifdef DEBUG_MODE
  Serial.println("Map settings saved to params");
#endif
}

// Callbacks
static void slider_zoom_event_cb(lv_event_t *e) {
  lv_event_code_t code = lv_event_get_code(e);
  if (code == LV_EVENT_VALUE_CHANGED) {
    lv_obj_t *slider = (lv_obj_t*)lv_event_get_target(e);
    lv_obj_t *label = (lv_obj_t*)lv_event_get_user_data(e);
    int value = (int)lv_slider_get_value(slider);
    lv_label_set_text_fmt(label, "%d", value);
    
#ifdef DEBUG_MODE
    Serial.printf("Zoom changed to: %d\n", value);
#endif
  }
}

static void slider_track_event_cb(lv_event_t *e) {
  lv_event_code_t code = lv_event_get_code(e);
  if (code == LV_EVENT_VALUE_CHANGED) {
    lv_obj_t *slider = (lv_obj_t*)lv_event_get_target(e);
    lv_obj_t *label = (lv_obj_t*)lv_event_get_user_data(e);
    int value = (int)lv_slider_get_value(slider);
    lv_label_set_text_fmt(label, "%d", value);
    
#ifdef DEBUG_MODE
    Serial.printf("Track points changed to: %d\n", value);
#endif
  }
}

static void dropdown_map_event_cb(lv_event_t *e) {
  lv_event_code_t code = lv_event_get_code(e);
  if (code == LV_EVENT_VALUE_CHANGED) {
    lv_obj_t *dropdown = (lv_obj_t*)lv_event_get_target(e);
    int selected = lv_dropdown_get_selected(dropdown);
    
#ifdef DEBUG_MODE
    char name[64];
    get_tile_server_name(selected, name, sizeof(name));
    Serial.printf("Tile server changed to: %s\n", name);
#endif
  }
}

static void switch_vario_event_cb(lv_event_t *e) {
  lv_event_code_t code = lv_event_get_code(e);
  if (code == LV_EVENT_VALUE_CHANGED) {
    lv_obj_t *switch_obj = (lv_obj_t*)lv_event_get_target(e);
    lv_obj_t *indicator = (lv_obj_t*)lv_event_get_user_data(e);
    bool checked = lv_obj_has_state(switch_obj, LV_STATE_CHECKED);
    
    if (indicator) {
      if (checked) {
        lv_obj_align(indicator, LV_ALIGN_RIGHT_MID, -2, 0);
      } else {
        lv_obj_align(indicator, LV_ALIGN_LEFT_MID, 2, 0);
      }
    }
    
#ifdef DEBUG_MODE
    Serial.printf("Vario colors: %s\n", checked ? "ON" : "OFF");
#endif
  }
}

typedef struct {
  lv_obj_t *slider_zoom;
  lv_obj_t *dropdown_tile_server;
  lv_obj_t *slider_track_points;
  lv_obj_t *switch_vario_colors;
} map_widgets_t;

static void btn_save_map_cb(lv_event_t *e) {
  map_widgets_t *widgets = (map_widgets_t*)lv_event_get_user_data(e);
  
#ifdef DEBUG_MODE
  Serial.println("Save map settings clicked");
#endif
  save_map_settings(widgets->slider_zoom, widgets->dropdown_tile_server,
                    widgets->slider_track_points, widgets->switch_vario_colors);
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

  lv_obj_t *main_frame = ui_create_black_screen_with_frame(3, 20, &main_screen);

  // Titre
  lv_obj_t *label_title = ui_create_label(main_frame, txt->map_settings,
                                           &lv_font_montserrat_32, lv_color_hex(0x00d4ff));
  lv_obj_align(label_title, LV_ALIGN_TOP_MID, 0, 0);

  // Widgets structure pour les callbacks
  static map_widgets_t widgets;

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

  // 1. Niveau de zoom
  lv_obj_t *label_zoom = ui_create_label(col_left, "Niveau de zoom",
                                          &lv_font_montserrat_20, lv_color_hex(0x00d4ff));
  
  widgets.slider_zoom = lv_slider_create(col_left);
  lv_slider_set_range(widgets.slider_zoom, 1, 18);
  lv_obj_set_width(widgets.slider_zoom, lv_pct(90));
  
  lv_obj_t *label_zoom_value = ui_create_label(col_left, "13",
                                                &lv_font_montserrat_20, lv_color_hex(0x808080));
  lv_obj_add_event_cb(widgets.slider_zoom, slider_zoom_event_cb, LV_EVENT_VALUE_CHANGED, label_zoom_value);

  // 2. Serveur de tuiles
  lv_obj_t *label_server = ui_create_label(col_left, "Serveur de tuiles",
                                            &lv_font_montserrat_20, lv_color_hex(0x00d4ff));
  
  widgets.dropdown_tile_server = lv_dropdown_create(col_left);
  lv_obj_set_width(widgets.dropdown_tile_server, lv_pct(90));
  
  // Construire la liste des serveurs depuis PROGMEM
  char names_buffer[256] = "";
  char temp_name[64];
  for (int i = 0; i < tile_servers_count; i++) {
    get_tile_server_name(i, temp_name, sizeof(temp_name));
    if (i > 0) strcat(names_buffer, "\n");
    strcat(names_buffer, temp_name);
  }
  lv_dropdown_set_options(widgets.dropdown_tile_server, names_buffer);
  lv_obj_set_style_bg_color(widgets.dropdown_tile_server, lv_color_hex(0x0f1520), 0);
  lv_obj_set_style_text_color(widgets.dropdown_tile_server, lv_color_white(), 0);
  lv_obj_set_style_text_font(widgets.dropdown_tile_server, &lv_font_montserrat_20, 0);
  lv_obj_add_event_cb(widgets.dropdown_tile_server, dropdown_map_event_cb, LV_EVENT_VALUE_CHANGED, NULL);

  // 3. Points de trace
  lv_obj_t *label_track = ui_create_label(col_left, "Points de trace",
                                           &lv_font_montserrat_20, lv_color_hex(0x00d4ff));
  
  widgets.slider_track_points = lv_slider_create(col_left);
  lv_slider_set_range(widgets.slider_track_points, 50, 500);
  lv_obj_set_width(widgets.slider_track_points, lv_pct(90));
  
  lv_obj_t *label_track_value = ui_create_label(col_left, "200",
                                                 &lv_font_montserrat_20, lv_color_hex(0x808080));
  lv_obj_add_event_cb(widgets.slider_track_points, slider_track_event_cb, LV_EVENT_VALUE_CHANGED, label_track_value);

  // 4. Switch couleurs vario
  lv_obj_t *label_vario = ui_create_label(col_left, "Couleurs vario sur trace",
                                          &lv_font_montserrat_20, lv_color_hex(0x00d4ff));
  
  widgets.switch_vario_colors = lv_obj_create(col_left);
  lv_obj_set_size(widgets.switch_vario_colors, 60, 30);
  lv_obj_set_style_radius(widgets.switch_vario_colors, 15, 0);
  lv_obj_set_style_bg_color(widgets.switch_vario_colors, lv_color_hex(0x34c759), LV_STATE_CHECKED);
  lv_obj_set_style_bg_color(widgets.switch_vario_colors, lv_color_hex(0x444444), 0);
  lv_obj_add_flag(widgets.switch_vario_colors, LV_OBJ_FLAG_CHECKABLE);
  lv_obj_add_flag(widgets.switch_vario_colors, LV_OBJ_FLAG_CLICKABLE);
  lv_obj_clear_flag(widgets.switch_vario_colors, LV_OBJ_FLAG_SCROLLABLE);
  
  lv_obj_t *switch_indicator = lv_obj_create(widgets.switch_vario_colors);
  lv_obj_set_size(switch_indicator, 24, 24);
  lv_obj_set_style_radius(switch_indicator, 12, 0);
  lv_obj_set_style_bg_color(switch_indicator, lv_color_white(), 0);
  lv_obj_set_style_border_width(switch_indicator, 0, 0);
  lv_obj_clear_flag(switch_indicator, LV_OBJ_FLAG_CLICKABLE);
  lv_obj_clear_flag(switch_indicator, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_align(switch_indicator, LV_ALIGN_RIGHT_MID, -2, 0);
  
  lv_obj_add_event_cb(widgets.switch_vario_colors, switch_vario_event_cb, LV_EVENT_VALUE_CHANGED, switch_indicator);

  // Colonne droite (apercu carte)
  lv_obj_t *col_right = lv_obj_create(main_container);
  lv_obj_set_size(col_right, 480, 420);
  lv_obj_set_style_bg_color(col_right, lv_color_hex(0x1a2035), 0);
  lv_obj_set_style_bg_opa(col_right, LV_OPA_50, 0);
  lv_obj_set_style_border_width(col_right, 2, 0);
  lv_obj_set_style_border_color(col_right, lv_color_hex(0x6080a0), 0);
  lv_obj_set_style_radius(col_right, 15, 0);
  lv_obj_clear_flag(col_right, LV_OBJ_FLAG_SCROLLABLE);
  
  lv_obj_t *label_preview = ui_create_label(col_right, "Apercu carte\n(bientot disponible)",
                                             &lv_font_montserrat_20, lv_color_hex(0x6080a0));
  lv_obj_center(label_preview);
  lv_obj_set_style_text_align(label_preview, LV_TEXT_ALIGN_CENTER, 0);

  // Boutons Enregistrer et Annuler
  ui_button_pair_t buttons = ui_create_save_cancel_buttons(main_frame, txt->save, txt->cancel, 
                                                            nullptr, true, true, false, 
                                                            btn_save_map_cb, btn_cancel_map_cb, nullptr, 
                                                            &widgets, NULL, NULL);

  // Charger les valeurs sauvegardees
  load_map_settings(widgets.slider_zoom, label_zoom_value, widgets.dropdown_tile_server,
                    widgets.slider_track_points, label_track_value, widgets.switch_vario_colors,
                    switch_indicator);

  lv_screen_load(main_screen);

#ifdef DEBUG_MODE
  Serial.println("Map settings screen initialized");
#endif
}

void ui_settings_map_show(void) {
  ui_settings_map_init();

#ifdef DEBUG_MODE
  Serial.println("Map settings screen displayed");
#endif
}

#endif