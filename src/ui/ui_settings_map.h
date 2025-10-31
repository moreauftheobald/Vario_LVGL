#ifndef UI_SETTINGS_MAP_H
#define UI_SETTINGS_MAP_H

#include "lvgl.h"
#include "constants.h"
#include "UI_helper.h"
#include "lang.h"
#include "globals.h"
#include "src/params/params.h"
#include "src/osm_tile_loader.h"

// Variables statiques pour mise à jour carte
static lv_obj_t *map_canvas_preview = NULL;
static lv_obj_t *map_container_preview = NULL;

void ui_settings_show(void);

// Structure des serveurs de tuiles
typedef struct {
  const char *name;
  const char *url;
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
  { tile_name_0, tile_url_0 },
  { tile_name_1, tile_url_1 },
  { tile_name_2, tile_url_2 },
  { tile_name_3, tile_url_3 },
  { tile_name_4, tile_url_4 },
  { tile_name_5, tile_url_5 },
  { tile_name_6, tile_url_6 }
};

static const int tile_servers_count = sizeof(tile_servers) / sizeof(TileServer);

// Fonction pour rafraîchir l'aperçu carte
// Fonction pour rafraîchir l'aperçu carte
static void refresh_map_preview(int zoom) {
  if (!map_container_preview) return;

  // Supprimer ancien canvas si existe
  if (map_canvas_preview) {
    lv_obj_del(map_canvas_preview);
    map_canvas_preview = NULL;
  }

  // Créer nouvelle vue carte avec 9 tuiles (3x3)
#ifdef FLIGHT_TEST_MODE
  map_canvas_preview = create_map_view(map_container_preview, TEST_LAT, TEST_LON, zoom, 440, 350);
#else
  double display_lat = g_sensor_data.gps.valid ? g_sensor_data.gps.latitude : TEST_LAT;
  double display_lon = g_sensor_data.gps.valid ? g_sensor_data.gps.longitude : TEST_LON;
  map_canvas_preview = create_map_view(map_container_preview, display_lat, display_lon, zoom, 440, 350);
#endif

  if (map_canvas_preview) {
    lv_obj_center(map_canvas_preview);

    // Recréer marqueur position au centre
    lv_obj_t *marker = lv_obj_create(map_container_preview);
    lv_obj_set_size(marker, 16, 16);
    lv_obj_center(marker);
    lv_obj_set_style_radius(marker, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(marker, lv_color_hex(0xff0000), 0);
    lv_obj_set_style_border_width(marker, 3, 0);
    lv_obj_set_style_border_color(marker, lv_color_hex(0xffffff), 0);
  }

#ifdef DEBUG_MODE
  Serial.printf("[MAP_SETTINGS] Map preview refreshed with zoom %d\n", zoom);
#endif
}

// Helper pour lire un nom de serveur depuis PROGMEM
static void get_tile_server_name(int index, char *buffer, size_t buf_size) {
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
static void get_tile_server_url(int index, char *buffer, size_t buf_size) {
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
  ui_load_slider_with_label(slider_zoom, label_zoom_value, params.map_zoom, "%d");
  ui_load_dropdown(dropdown_tile_server, params.map_tile_server);
  ui_load_slider_with_label(slider_track_points, label_track_value, params.map_track_points, "%d");
  ui_load_switch(switch_vario_colors, switch_indicator, params.map_vario_colors);

#ifdef DEBUG_MODE
  Serial.println("Map settings loaded from params");
#endif
}

static void save_map_settings(lv_obj_t *slider_zoom, lv_obj_t *dropdown_tile_server,
                              lv_obj_t *slider_track_points, lv_obj_t *switch_vario_colors) {
  params.map_zoom = ui_save_slider(slider_zoom);
  params.map_tile_server = ui_save_dropdown(dropdown_tile_server);
  params.map_track_points = ui_save_slider(slider_track_points);
  params.map_vario_colors = ui_save_switch(switch_vario_colors);

  params_save_map();

#ifdef DEBUG_MODE
  Serial.println("Map settings saved to params");
#endif
}

// Callback modifié pour le slider zoom
static void slider_zoom_event_cb(lv_event_t *e) {
  lv_event_code_t code = lv_event_get_code(e);
  if (code == LV_EVENT_VALUE_CHANGED) {
    lv_obj_t *slider = (lv_obj_t *)lv_event_get_target(e);
    lv_obj_t *label = (lv_obj_t *)lv_event_get_user_data(e);
    int value = (int)lv_slider_get_value(slider);
    lv_label_set_text_fmt(label, "%d", value);

    // AJOUTE: Rafraîchir l'aperçu carte
    refresh_map_preview(value);
  }
}

static void slider_track_event_cb(lv_event_t *e) {
  lv_event_code_t code = lv_event_get_code(e);
  if (code == LV_EVENT_VALUE_CHANGED) {
    lv_obj_t *slider = (lv_obj_t *)lv_event_get_target(e);
    lv_obj_t *label = (lv_obj_t *)lv_event_get_user_data(e);
    int value = (int)lv_slider_get_value(slider);
    lv_label_set_text_fmt(label, "%d", value);
  }
}

static void dropdown_map_event_cb(lv_event_t *e) {
  lv_event_code_t code = lv_event_get_code(e);
  if (code == LV_EVENT_VALUE_CHANGED) {
    lv_obj_t *dropdown = (lv_obj_t *)lv_event_get_target(e);
    int selected = lv_dropdown_get_selected(dropdown);
  }
}

static void switch_vario_event_cb(lv_event_t *e) {
  lv_event_code_t code = lv_event_get_code(e);
  if (code == LV_EVENT_VALUE_CHANGED) {
    lv_obj_t *switch_obj = (lv_obj_t *)lv_event_get_target(e);
    lv_obj_t *indicator = (lv_obj_t *)lv_event_get_user_data(e);
    bool checked = lv_obj_has_state(switch_obj, LV_STATE_CHECKED);

    if (indicator) {
      if (checked) {
        lv_obj_align(indicator, LV_ALIGN_RIGHT_MID, -2, 0);
      } else {
        lv_obj_align(indicator, LV_ALIGN_LEFT_MID, 2, 0);
      }
    }
  }
}

typedef struct {
  lv_obj_t *slider_zoom;
  lv_obj_t *dropdown_tile_server;
  lv_obj_t *slider_track_points;
  lv_obj_t *switch_vario_colors;
} map_widgets_t;

static void btn_save_map_cb(lv_event_t *e) {
  map_widgets_t *widgets = (map_widgets_t *)lv_event_get_user_data(e);

#ifdef DEBUG_MODE
  Serial.println("Save map settings clicked");
#endif
  save_map_settings(widgets->slider_zoom, widgets->dropdown_tile_server,
                    widgets->slider_track_points, widgets->switch_vario_colors);
  ui_settings_show();
}

static void btn_cancel_map_cb(lv_event_t *e) {
  ui_settings_show();
}

void ui_settings_map_init(void) {
  const TextStrings *txt = get_text();

  lv_obj_t *main_frame = ui_create_black_screen_with_frame(3, ROUND_FRANE_RADUIS_BIG, &current_screen);

  ui_create_main_frame(main_frame, true, txt->map_settings);

  // Widgets structure pour les callbacks
  static map_widgets_t widgets;

  // 1. Niveau de zoom
  lv_obj_t *label_zoom = ui_create_label(main_left, "Niveau de zoom", INFO_FONT_BIG, lv_color_hex(TITLE_COLOR));

  widgets.slider_zoom = lv_slider_create(main_left);
  lv_slider_set_range(widgets.slider_zoom, MAP_ZOOM_MIN, MAP_ZOOM_MAX);
  lv_obj_set_width(widgets.slider_zoom, lv_pct(90));

  lv_obj_t *label_zoom_value = ui_create_label(main_left, "13", INFO_FONT_BIG, lv_color_hex(INFO_DATAS_COLOR));
  lv_obj_add_event_cb(widgets.slider_zoom, slider_zoom_event_cb, LV_EVENT_VALUE_CHANGED, label_zoom_value);

  // 2. Serveur de tuiles
  lv_obj_t *label_server = ui_create_label(main_left, "Serveur de tuiles", INFO_FONT_BIG, lv_color_hex(TITLE_COLOR));

  widgets.dropdown_tile_server = lv_dropdown_create(main_left);
  lv_obj_set_width(widgets.dropdown_tile_server, lv_pct(90));

  // Construire la liste des serveurs depuis PROGMEM
  char names_buffer[192] = "";
  char temp_name[48];
  for (int i = 0; i < tile_servers_count; i++) {
    get_tile_server_name(i, temp_name, sizeof(temp_name));
    if (i > 0) strcat(names_buffer, "\n");
    strcat(names_buffer, temp_name);
  }
  lv_dropdown_set_options(widgets.dropdown_tile_server, names_buffer);
  lv_obj_set_style_bg_color(widgets.dropdown_tile_server, lv_color_hex(CTL_BG_COLOR), 0);
  lv_obj_set_style_text_color(widgets.dropdown_tile_server, lv_color_white(), 0);
  lv_obj_set_style_text_font(widgets.dropdown_tile_server, INFO_FONT_BIG, 0);
  lv_obj_add_event_cb(widgets.dropdown_tile_server, dropdown_map_event_cb, LV_EVENT_VALUE_CHANGED, NULL);

  // 3. Points de trace
  lv_obj_t *label_track = ui_create_label(main_left, "Points de trace", INFO_FONT_BIG, lv_color_hex(TITLE_COLOR));

  widgets.slider_track_points = lv_slider_create(main_left);
  lv_slider_set_range(widgets.slider_track_points, 50, 500);
  lv_obj_set_width(widgets.slider_track_points, lv_pct(90));

  lv_obj_t *label_track_value = ui_create_label(main_left, "200", INFO_FONT_BIG, lv_color_hex(INFO_DATAS_COLOR));
  lv_obj_add_event_cb(widgets.slider_track_points, slider_track_event_cb, LV_EVENT_VALUE_CHANGED, label_track_value);

  // 4. Switch couleurs vario
  lv_obj_t *label_vario = ui_create_label(main_left, "Couleurs vario sur trace", INFO_FONT_BIG, lv_color_hex(TITLE_COLOR));

  widgets.switch_vario_colors = lv_obj_create(main_left);
  lv_obj_set_size(widgets.switch_vario_colors, 60, 30);
  lv_obj_set_style_radius(widgets.switch_vario_colors, ROUND_FRANE_RADUIS_BIG, 0);
  lv_obj_set_style_bg_color(widgets.switch_vario_colors, lv_color_hex(SW_ON), LV_STATE_CHECKED);
  lv_obj_set_style_bg_color(widgets.switch_vario_colors, lv_color_hex(SW_OFF), 0);
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

  // Titre aperçu
  lv_obj_t *label_preview = ui_create_label(main_right, "Apercu carte", INFO_FONT_BIG, lv_color_hex(TITLE_COLOR));
  lv_obj_align(label_preview, LV_ALIGN_TOP_MID, 0, 0);

  // Conteneur pour la carte (agrandi pour occuper tout l'espace)
  map_container_preview = lv_obj_create(main_right);
  lv_obj_set_size(map_container_preview, 450, 360);
  lv_obj_align(map_container_preview, LV_ALIGN_CENTER, 0, 20);
  lv_obj_set_style_bg_color(map_container_preview, lv_color_hex(0x1c1c1e), 0);
  lv_obj_set_style_border_width(map_container_preview, 2, 0);
  lv_obj_set_style_border_color(map_container_preview, lv_color_hex(TITLE_COLOR), 0);
  lv_obj_set_style_pad_all(map_container_preview, 5, 0);
  lv_obj_clear_flag(map_container_preview, LV_OBJ_FLAG_SCROLLABLE);

  // Afficher vue carte multi-tuiles (3x3)
#ifdef FLIGHT_TEST_MODE
  map_canvas_preview = create_map_view(map_container_preview, TEST_LAT, TEST_LON, params.map_zoom, 440, 350);
#else
  double display_lat = g_sensor_data.gps.valid ? g_sensor_data.gps.latitude : TEST_LAT;
  double display_lon = g_sensor_data.gps.valid ? g_sensor_data.gps.longitude : TEST_LON;
  map_canvas_preview = create_map_view(map_container_preview, display_lat, display_lon, params.map_zoom, 440, 350);
#endif

  if (map_canvas_preview) {
    lv_obj_center(map_canvas_preview);

    // Marqueur position rouge au centre
    lv_obj_t *marker = lv_obj_create(map_container_preview);
    lv_obj_set_size(marker, 16, 16);
    lv_obj_center(marker);
    lv_obj_set_style_radius(marker, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(marker, lv_color_hex(KO_COLOR), 0);
    lv_obj_set_style_border_width(marker, 3, 0);
    lv_obj_set_style_border_color(marker, lv_color_hex(BORDERS_COLOR), 0);
  }

  // Bouton Save
  lv_obj_t *btn_save_map = ui_create_button(btn_container, txt->save, LV_SYMBOL_SAVE, lv_color_hex(START_BTN_COLOR),
                                            PRE_BTN_W, PRE_BTN_H, INFO_FONT_S, INFO_FONT_BIG, btn_save_map_cb,
                                            &widgets, (lv_align_t)0, NULL, NULL);

  // Bouton Cancel
  lv_obj_t *btn_cancel_map = ui_create_button(btn_container, txt->cancel, LV_SYMBOL_BACKSPACE, lv_color_hex(CANCE_BTN_COLOR),
                                              PRE_BTN_W, PRE_BTN_H, INFO_FONT_S, INFO_FONT_BIG, btn_cancel_map_cb,
                                              &widgets, (lv_align_t)0, NULL, NULL);

  // Charger valeurs actuelles
  load_map_settings(widgets.slider_zoom, label_zoom_value,
                    widgets.dropdown_tile_server, widgets.slider_track_points,
                    label_track_value, widgets.switch_vario_colors, NULL);


#ifdef DEBUG_MODE
  Serial.println("Map settings screen initialized with map preview");
#endif
}

void ui_settings_map_show(void) {
  ui_switch_screen(ui_settings_map_init);
}

#endif