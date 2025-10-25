#ifndef UI_HELPER_H
#define UI_HELPER_H

#include "lvgl.h"
#include "constants.h"
#include "lang.h"

// ============================================================================
// VARIABLES GLOBALES BARRE DE STATUT
// ============================================================================
static lv_obj_t *g_label_time = NULL;
static lv_obj_t *g_label_flight_time = NULL;
static lv_obj_t *g_label_wifi = NULL;
static lv_obj_t *g_label_internet = NULL;
static lv_obj_t *g_label_gps_sats = NULL;
static lv_obj_t *g_label_battery = NULL;

// ============================================================================
// CREATION ECRANS ET WIDGETS DE BASE
// ============================================================================

/**
 * @brief Cree un ecran noir avec un frame principal ayant une bordure blanche
 * @param border_width Epaisseur de la bordure en pixels
 * @param radius Rayon des coins arrondis
 * @param screen_ptr Pointeur vers la variable screen à utiliser/créer
 * @return Le frame principal (pas l'ecran)
 */
static inline lv_obj_t *ui_create_black_screen_with_frame(uint16_t border_width, uint16_t radius, lv_obj_t **screen_ptr) {
  // Ecran noir sans bordure
  if (*screen_ptr != NULL) {
    lv_obj_clean(*screen_ptr);
  } else {
    *screen_ptr = lv_obj_create(NULL);
  }

  lv_obj_set_style_bg_color(*screen_ptr, lv_color_hex(0x000000), 0);
  lv_obj_set_style_bg_opa(*screen_ptr, LV_OPA_COVER, 0);
  lv_obj_set_style_border_width(*screen_ptr, 0, 0);
  lv_obj_clear_flag(*screen_ptr, LV_OBJ_FLAG_SCROLLABLE);

  // Frame avec bordure blanche - fond NOIR uni
  lv_obj_t *frame = lv_obj_create(*screen_ptr);
  lv_obj_set_size(frame, LCD_H_RES, LCD_V_RES);
  lv_obj_center(frame);
  lv_obj_set_style_bg_color(frame, lv_color_hex(0x000000), 0);  // NOIR
  lv_obj_set_style_bg_opa(frame, LV_OPA_COVER, 0);              // Opaque à 100%
  lv_obj_set_style_border_width(frame, border_width, 0);
  lv_obj_set_style_border_color(frame, lv_color_hex(0xFFFFFF), 0);
  lv_obj_set_style_radius(frame, radius, 0);
  lv_obj_set_style_pad_all(frame, 20, 0);
  lv_obj_set_style_shadow_width(frame, 30, 0);
  lv_obj_set_style_shadow_color(frame, lv_color_black(), 0);
  lv_obj_set_style_shadow_opa(frame, LV_OPA_40, 0);

  lv_obj_clear_flag(frame, LV_OBJ_FLAG_SCROLLABLE);

  return frame;
}

/**
 * @brief Cree un ecran avec gradient background standard
 */
static inline lv_obj_t *ui_create_screen(void) {
  lv_obj_t *screen = lv_obj_create(NULL);
  lv_obj_set_style_bg_color(screen, lv_color_hex(0x000000), 0);
  return screen;
}


/**
 * @brief Cree un titre d'ecran
 */
static inline lv_obj_t *ui_create_title(lv_obj_t *parent, const char *text) {
  lv_obj_t *title = lv_label_create(parent);
  lv_label_set_text(title, text);
  lv_obj_set_style_text_font(title, &lv_font_montserrat_48, 0);
  lv_obj_set_style_text_color(title, lv_color_hex(0x00d4ff), 0);
  lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 5);
  return title;
}

/**
 * @brief Cree un panel d'information
 */
static inline lv_obj_t *ui_create_info_panel(lv_obj_t *parent, int width, int height) {
  lv_obj_t *panel = lv_obj_create(parent);
  lv_obj_set_size(panel, width, height);
  lv_obj_set_style_bg_color(panel, lv_color_hex(0x000000), 0);
  lv_obj_set_style_bg_opa(panel, LV_OPA_COVER, 0);
  lv_obj_set_style_border_width(panel, 0, 0);
  lv_obj_set_style_radius(panel, ROUND_FRANE_RADUIS_SMALL, 0);
  lv_obj_set_style_pad_all(panel, 20, 0);
  lv_obj_set_flex_flow(panel, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_flex_align(panel, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
  lv_obj_clear_flag(panel, LV_OBJ_FLAG_SCROLLABLE);
  return panel;
}

/**
 * @brief Cree un bouton moderne avec icone et texte
 */
static inline lv_obj_t *ui_create_button(lv_obj_t *parent, const char *text, const char *icon,
                                         lv_color_t color, int width, int height,
                                         const lv_font_t *text_font, const lv_font_t *icon_font,
                                         lv_event_cb_t callback, void *user_data,
                                         lv_align_t align, int32_t x_off, int32_t y_off) {
  lv_obj_t *btn = lv_button_create(parent);
  lv_obj_set_size(btn, width, height);
  lv_obj_set_style_bg_color(btn, color, 0);
  lv_obj_set_style_radius(btn, ROUND_FRANE_RADUIS_BIG, 0);
  lv_obj_set_style_shadow_width(btn, 5, 0);
  lv_obj_set_style_shadow_color(btn, lv_color_black(), 0);
  lv_obj_set_style_shadow_opa(btn, LV_OPA_20, 0);
  lv_obj_set_style_bg_color(btn, lv_color_darken(color, 20), LV_STATE_PRESSED);

  if (icon) {
    lv_obj_t *icon_label = lv_label_create(btn);
    lv_label_set_text(icon_label, icon);
    lv_obj_set_style_text_font(icon_label, icon_font, 0);
    lv_obj_set_style_text_color(icon_label, lv_color_white(), 0);
    lv_obj_align(icon_label, LV_ALIGN_LEFT_MID, 20, 0);
  }

  if (text) {
    lv_obj_t *text_label = lv_label_create(btn);
    lv_label_set_text(text_label, text);
    lv_obj_set_style_text_font(text_label, text_font, 0);
    lv_obj_set_style_text_color(text_label, lv_color_white(), 0);
    lv_obj_align(text_label, icon ? LV_ALIGN_LEFT_MID : LV_ALIGN_CENTER, icon ? 70 : 0, 0);
  }

  if (align) {
    Serial.println("OK");
    lv_obj_align(btn, align, x_off, y_off);
  }

  if (callback) {
    lv_obj_add_event_cb(btn, callback, LV_EVENT_CLICKED, user_data);
  }

  return btn;
}

/**
 * @brief Cree un conteneur flex transparent
 */
static inline lv_obj_t *ui_create_flex_container(lv_obj_t *parent, lv_flex_flow_t flow) {
  lv_obj_t *container = lv_obj_create(parent);
  lv_obj_set_size(container, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
  lv_obj_set_style_bg_opa(container, LV_OPA_TRANSP, 0);
  lv_obj_set_style_border_width(container, 0, 0);
  lv_obj_set_style_pad_all(container, 0, 0);
  lv_obj_set_flex_flow(container, flow);
  lv_obj_clear_flag(container, LV_OBJ_FLAG_SCROLLABLE);
  return container;
}

/**
 * @brief Cree un label avec style par defaut
 */
static inline lv_obj_t *ui_create_label(lv_obj_t *parent, const char *text,
                                        const lv_font_t *font, lv_color_t color) {
  lv_obj_t *label = lv_label_create(parent);
  lv_label_set_text(label, text);
  lv_obj_set_style_text_font(label, font, 0);
  lv_obj_set_style_text_color(label, color, 0);
  return label;
}

/**
 * @brief Cree un separateur horizontal
 */
static inline lv_obj_t *ui_create_separator(lv_obj_t *parent) {
  lv_obj_t *sep = lv_obj_create(parent);
  lv_obj_set_size(sep, lv_pct(100), 1);
  lv_obj_set_style_bg_color(sep, lv_color_hex(0x2a3f5f), 0);
  lv_obj_set_style_border_width(sep, 0, 0);
  return sep;
}

/**
 * @brief Cree un champ de saisie texte
 */
static inline lv_obj_t *ui_create_textarea(lv_obj_t *parent, const char *placeholder,
                                           int max_length, bool one_line) {
  lv_obj_t *ta = lv_textarea_create(parent);
  lv_obj_set_size(ta, lv_pct(100), one_line ? 50 : 150);
  lv_textarea_set_one_line(ta, one_line);
  lv_textarea_set_max_length(ta, max_length);
  lv_textarea_set_placeholder_text(ta, placeholder);
  lv_obj_set_style_bg_color(ta, lv_color_hex(0x0f1520), 0);
  lv_obj_set_style_border_color(ta, lv_color_hex(0x4080a0), 0);
  lv_obj_set_style_border_width(ta, 2, 0);
  lv_obj_set_style_radius(ta, ROUND_FRANE_RADUIS_SMALL, 0);
  return ta;
}

/**
 * @brief Cree un clavier
 */
static inline lv_obj_t *ui_create_keyboard(lv_obj_t *parent, lv_keyboard_mode_t mode) {
  lv_obj_t *kb = lv_keyboard_create(parent);
  lv_keyboard_set_mode(kb, mode);
  lv_obj_set_size(kb, lv_pct(100), lv_pct(40));
  lv_obj_align(kb, LV_ALIGN_BOTTOM_MID, 0, 0);
  lv_obj_add_flag(kb, LV_OBJ_FLAG_HIDDEN);
  return kb;
}

/**
 * @brief Cree un panel d'information avec bordure
 */
static inline lv_obj_t *ui_create_info_panel_bordered(lv_obj_t *parent, int width, int height) {
  lv_obj_t *panel = lv_obj_create(parent);
  lv_obj_set_size(panel, width, height);
  lv_obj_set_style_bg_color(panel, lv_color_hex(0x000000), 0);
  lv_obj_set_style_bg_opa(panel, LV_OPA_COVER, 0);
  lv_obj_set_style_border_width(panel, 3, 0);
  lv_obj_set_style_border_color(panel, lv_color_hex(0xFFFFFF), 0);
  lv_obj_set_style_radius(panel, ROUND_FRANE_RADUIS_SMALL, 0);
  lv_obj_set_style_pad_all(panel, 20, 0);
  lv_obj_set_flex_flow(panel, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_flex_align(panel, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
  return panel;
}

/**
 * @brief Cree un champ de saisie avec label
 */
static inline lv_obj_t *ui_create_input_field(lv_obj_t *parent, const char *label_text,
                                              const char *placeholder, int max_length) {
  lv_obj_t *label = ui_create_label(parent, label_text, &lv_font_montserrat_20, lv_color_hex(0x00d4ff));

  lv_obj_t *ta = lv_textarea_create(parent);
  lv_obj_set_size(ta, lv_pct(100), 50);
  lv_textarea_set_one_line(ta, true);
  lv_textarea_set_max_length(ta, max_length);
  lv_textarea_set_placeholder_text(ta, placeholder);
  lv_obj_set_style_bg_color(ta, lv_color_hex(0x0f1520), 0);
  lv_obj_set_style_border_color(ta, lv_color_hex(0x4080a0), 0);
  lv_obj_set_style_border_width(ta, 2, 0);
  lv_obj_set_style_radius(ta, ROUND_FRANE_RADUIS_SMALL, 0);
  lv_obj_set_style_text_color(ta, lv_color_white(), 0);
  lv_obj_set_style_text_font(ta, &lv_font_montserrat_20, 0);
  lv_obj_set_style_pad_all(ta, 8, 0);

  return ta;
}

/**
 * @brief Cree une ligne de formulaire avec label et widget
 */
static inline lv_obj_t *ui_create_form_row(lv_obj_t *parent, const char *label_text,
                                           int label_width, lv_color_t label_color) {
  lv_obj_t *row = ui_create_flex_container(parent, LV_FLEX_FLOW_ROW);
  lv_obj_set_size(row, lv_pct(100), LV_SIZE_CONTENT);
  lv_obj_set_flex_align(row, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
  lv_obj_set_style_pad_column(row, 20, 0);

  lv_obj_t *label = ui_create_label(row, label_text, &lv_font_montserrat_20, label_color);
  lv_obj_set_width(label, label_width);

  return row;
}

/**
 * @brief Cree une colonne de formulaire avec bordure
 */
static inline lv_obj_t *ui_create_form_column(lv_obj_t *parent, int width) {
  lv_obj_t *col = lv_obj_create(parent);
  lv_obj_set_size(col, width, LV_SIZE_CONTENT);
  lv_obj_set_style_bg_color(col, lv_color_hex(0x1a2035), 0);
  lv_obj_set_style_bg_opa(col, LV_OPA_80, 0);
  lv_obj_set_style_border_width(col, 2, 0);
  lv_obj_set_style_border_color(col, lv_color_hex(0x6080a0), 0);
  lv_obj_set_style_radius(col, ROUND_FRANE_RADUIS_SMALL, 0);
  lv_obj_set_style_pad_all(col, 15, 0);
  lv_obj_set_flex_flow(col, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_flex_align(col, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
  lv_obj_set_style_pad_row(col, 15, 0);
  lv_obj_clear_flag(col, LV_OBJ_FLAG_SCROLLABLE);
  return col;
}


/**
 * @brief Paire de boutons Save/Cancel
 */
typedef struct {
  lv_obj_t *save;
  lv_obj_t *cancel;
  lv_obj_t *reset;
} ui_button_pair_t;

static inline ui_button_pair_t ui_create_save_cancel_buttons(lv_obj_t *parent, 
                                                              const char *save_text,
                                                              const char *cancel_text,
                                                              const char *reset_text,
                                                              bool save, bool cancel, bool reset,
                                                              lv_event_cb_t save_cb,
                                                              lv_event_cb_t cancel_cb,
                                                              lv_event_cb_t reset_cb,
                                                              void *save_user_data,
                                                              void *cancel_user_data,
                                                              void *reset_user_data) {
  ui_button_pair_t buttons = {NULL, NULL, NULL};
  
  lv_obj_t *container = ui_create_flex_container(parent, LV_FLEX_FLOW_ROW);
  lv_obj_set_size(container, lv_pct(100), LV_SIZE_CONTENT);
  lv_obj_align(container, LV_ALIGN_BOTTOM_MID, 0, -5);
  lv_obj_set_flex_align(container, LV_FLEX_ALIGN_SPACE_EVENLY, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

  if (save) {
    buttons.save = ui_create_button(container, save_text, NULL, lv_color_hex(0x34c759),
                                    220, 50, &lv_font_montserrat_20, &lv_font_montserrat_24,
                                    save_cb, save_user_data, (lv_align_t)0, 0, 0);
  }
  
  if (cancel) {
    buttons.cancel = ui_create_button(container, cancel_text, NULL, lv_color_hex(0xff3b30),
                                      220, 50, &lv_font_montserrat_20, &lv_font_montserrat_24,
                                      cancel_cb, cancel_user_data, (lv_align_t)0, 0, 0);
  }
  
  if (reset) {
    buttons.reset = ui_create_button(container, reset_text, NULL, lv_color_hex(0xff9500),
                                     220, 50, &lv_font_montserrat_20, &lv_font_montserrat_24,
                                     reset_cb, reset_user_data, (lv_align_t)0, 0, 0);
  }

  return buttons;
}
// ============================================================================
// BARRE DE STATUT
// ============================================================================

/**
 * @brief Cree la barre de statut
 * @param parent Ecran parent
 * @return Pointeur vers la barre de statut
 */
static inline lv_obj_t *ui_create_status_bar(lv_obj_t *parent) {
  lv_obj_t *bar = lv_obj_create(parent);
  lv_obj_set_size(bar, LCD_H_RES, 55);
  lv_obj_align(bar, LV_ALIGN_TOP_MID, 0, 0);
  lv_obj_set_style_bg_color(bar, lv_color_hex(0x000000), 0);
  lv_obj_set_style_bg_opa(bar, LV_OPA_COVER, 0);
  lv_obj_set_style_border_side(bar, LV_BORDER_SIDE_BOTTOM, 0);
  lv_obj_set_style_border_color(bar, lv_color_hex(0xFFFFFF), 0);
  lv_obj_set_style_border_width(bar, 2, 0);
  lv_obj_set_style_radius(bar, 0, 0);
  lv_obj_set_style_pad_all(bar, 8, 0);
  lv_obj_clear_flag(bar, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_clear_flag(bar, LV_OBJ_FLAG_CLICKABLE);

  // Heure
  g_label_time = lv_label_create(bar);
  lv_label_set_text(g_label_time, "00:00:00");
  lv_obj_set_style_text_font(g_label_time, &lv_font_montserrat_28, 0);
  lv_obj_set_style_text_color(g_label_time, lv_color_hex(0x00d4ff), 0);
  lv_obj_align(g_label_time, LV_ALIGN_LEFT_MID, 10, 0);

  // Temps de vol
  g_label_flight_time = lv_label_create(bar);
  lv_label_set_text(g_label_flight_time, "");
  lv_obj_set_style_text_font(g_label_flight_time, &lv_font_montserrat_24, 0);
  lv_obj_set_style_text_color(g_label_flight_time, lv_color_hex(0x4cd964), 0);
  lv_obj_align(g_label_flight_time, LV_ALIGN_LEFT_MID, 150, 0);
  lv_obj_add_flag(g_label_flight_time, LV_OBJ_FLAG_HIDDEN);

  // Container droite
  lv_obj_t *right_container = lv_obj_create(bar);
  lv_obj_set_size(right_container, 550, 40);
  lv_obj_align(right_container, LV_ALIGN_RIGHT_MID, -10, 0);
  lv_obj_set_style_bg_opa(right_container, LV_OPA_TRANSP, 0);
  lv_obj_set_style_border_width(right_container, 0, 0);
  lv_obj_set_style_pad_all(right_container, 0, 0);
  lv_obj_set_flex_flow(right_container, LV_FLEX_FLOW_ROW);
  lv_obj_set_flex_align(right_container, LV_FLEX_ALIGN_END, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
  lv_obj_set_style_pad_column(right_container, 50, 0);
  lv_obj_clear_flag(right_container, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_clear_flag(right_container, LV_OBJ_FLAG_CLICKABLE);

  // WiFi
  g_label_wifi = lv_label_create(right_container);
  lv_label_set_text(g_label_wifi, LV_SYMBOL_WIFI);
  lv_obj_set_style_text_font(g_label_wifi, &lv_font_montserrat_24, 0);
  lv_obj_set_style_text_color(g_label_wifi, lv_color_hex(0xff3b30), 0);

  // Internet
  g_label_internet = lv_label_create(right_container);
  lv_label_set_text(g_label_internet, LV_SYMBOL_UPLOAD);
  lv_obj_set_style_text_font(g_label_internet, &lv_font_montserrat_24, 0);
  lv_obj_set_style_text_color(g_label_internet, lv_color_hex(0xff3b30), 0);

  // GPS
  g_label_gps_sats = lv_label_create(right_container);
  lv_label_set_text(g_label_gps_sats, LV_SYMBOL_GPS " 0");
  lv_obj_set_style_text_font(g_label_gps_sats, &lv_font_montserrat_24, 0);
  lv_obj_set_style_text_color(g_label_gps_sats, lv_color_hex(0xff3b30), 0);

  // Batterie
  g_label_battery = lv_label_create(right_container);
  lv_label_set_text(g_label_battery, LV_SYMBOL_BATTERY_FULL " 100%");
  lv_obj_set_style_text_font(g_label_battery, &lv_font_montserrat_24, 0);
  lv_obj_set_style_text_color(g_label_battery, lv_color_hex(0x4cd964), 0);

  return bar;
}

/**
 * @brief Met a jour l'heure
 */
static inline void ui_status_bar_update_time(uint8_t hour, uint8_t minute, uint8_t second) {
  if (g_label_time) {
    lv_label_set_text_fmt(g_label_time, "%02d:%02d:%02d", hour, minute, second);
  }
}

/**
 * @brief Met a jour le temps de vol
 */
static inline void ui_status_bar_update_flight_time(uint32_t seconds) {
  if (g_label_flight_time) {
    if (seconds > 0) {
      uint8_t hours = seconds / 3600;
      uint8_t minutes = (seconds % 3600) / 60;
      uint8_t secs = seconds % 60;

      lv_label_set_text_fmt(g_label_flight_time, "VOL %02d:%02d:%02d", hours, minutes, secs);
      lv_obj_clear_flag(g_label_flight_time, LV_OBJ_FLAG_HIDDEN);
    } else {
      lv_obj_add_flag(g_label_flight_time, LV_OBJ_FLAG_HIDDEN);
    }
  }
}

/**
 * @brief Met a jour l'etat WiFi
 */
static inline void ui_status_bar_update_wifi(bool connected) {
  if (g_label_wifi) {
    if (connected) {
      lv_obj_set_style_text_color(g_label_wifi, lv_color_hex(0x007aff), 0);
    } else {
      lv_obj_set_style_text_color(g_label_wifi, lv_color_hex(0xff3b30), 0);
    }
  }
}

/**
 * @brief Met a jour l'etat Internet
 */
static inline void ui_status_bar_update_internet(bool connected) {
  if (g_label_internet) {
    if (connected) {
      lv_obj_set_style_text_color(g_label_internet, lv_color_hex(0x4cd964), 0);
    } else {
      lv_obj_set_style_text_color(g_label_internet, lv_color_hex(0xff3b30), 0);
    }
  }
}

/**
 * @brief Met a jour le nombre de satellites GPS
 */
static inline void ui_status_bar_update_gps(uint8_t satellites, bool has_fix) {
  if (g_label_gps_sats) {
    lv_label_set_text_fmt(g_label_gps_sats, LV_SYMBOL_GPS " %d", satellites);

    if (satellites == 0) {
      lv_obj_set_style_text_color(g_label_gps_sats, lv_color_hex(0xff3b30), 0);
    } else if (!has_fix) {
      lv_obj_set_style_text_color(g_label_gps_sats, lv_color_hex(0xff9500), 0);
    } else {
      lv_obj_set_style_text_color(g_label_gps_sats, lv_color_hex(0x4cd964), 0);
    }
  }
}

/**
 * @brief Met a jour le niveau de batterie
 */
static inline void ui_status_bar_update_battery(uint8_t level) {
  if (g_label_battery) {
    const char *icon;
    if (level > 80) {
      icon = LV_SYMBOL_BATTERY_FULL;
    } else if (level > 60) {
      icon = LV_SYMBOL_BATTERY_3;
    } else if (level > 40) {
      icon = LV_SYMBOL_BATTERY_2;
    } else if (level > 20) {
      icon = LV_SYMBOL_BATTERY_1;
    } else {
      icon = LV_SYMBOL_BATTERY_EMPTY;
    }

    lv_label_set_text_fmt(g_label_battery, "%s %d%%", icon, level);

    if (level > 20) {
      lv_obj_set_style_text_color(g_label_battery, lv_color_hex(0x4cd964), 0);
    } else if (level > 10) {
      lv_obj_set_style_text_color(g_label_battery, lv_color_hex(0xff9500), 0);
    } else {
      lv_obj_set_style_text_color(g_label_battery, lv_color_hex(0xff3b30), 0);
    }
  }
}

#endif