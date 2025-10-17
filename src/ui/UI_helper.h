#ifndef UI_HELPER_H
#define UI_HELPER_H

#include "lvgl.h"
#include "constants.h"

/**
 * @brief Cree un ecran avec gradient background standard
 */
static inline lv_obj_t* ui_create_screen(void) {
  lv_obj_t *screen = lv_obj_create(NULL);
  lv_obj_set_style_bg_color(screen, lv_color_hex(0x0a0e27), 0);
  lv_obj_set_style_bg_grad_color(screen, lv_color_hex(0x1a1f3a), 0);
  lv_obj_set_style_bg_grad_dir(screen, LV_GRAD_DIR_VER, 0);
  return screen;
}

/**
 * @brief Cree un main frame avec bordure et ombre
 */
static inline lv_obj_t* ui_create_main_frame(lv_obj_t *parent) {
  lv_obj_t *frame = lv_obj_create(parent);
  lv_obj_set_size(frame, SCREEN_WIDTH - 10, SCREEN_HEIGHT - 10);
  lv_obj_center(frame);
  lv_obj_set_style_bg_color(frame, lv_color_hex(0x151932), 0);
  lv_obj_set_style_bg_opa(frame, LV_OPA_90, 0);
  lv_obj_set_style_border_width(frame, 3, 0);
  lv_obj_set_style_border_color(frame, lv_color_hex(0x6080a0), 0);
  lv_obj_set_style_radius(frame, 20, 0);
  lv_obj_set_style_pad_all(frame, 20, 0);
  lv_obj_set_style_shadow_width(frame, 30, 0);
  lv_obj_set_style_shadow_color(frame, lv_color_black(), 0);
  lv_obj_set_style_shadow_opa(frame, LV_OPA_40, 0);
  return frame;
}

/**
 * @brief Cree un titre d'ecran
 */
static inline lv_obj_t* ui_create_title(lv_obj_t *parent, const char *text) {
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
static inline lv_obj_t* ui_create_info_panel(lv_obj_t *parent, int width, int height) {
  lv_obj_t *panel = lv_obj_create(parent);
  lv_obj_set_size(panel, width, height);
  lv_obj_set_style_bg_color(panel, lv_color_hex(0x1a2035), 0);
  lv_obj_set_style_bg_opa(panel, LV_OPA_80, 0);
  lv_obj_set_style_border_width(panel, 0, 0);
  lv_obj_set_style_radius(panel, 15, 0);
  lv_obj_set_style_pad_all(panel, 20, 0);
  lv_obj_set_flex_flow(panel, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_flex_align(panel, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
  lv_obj_clear_flag(panel, LV_OBJ_FLAG_SCROLLABLE);
  return panel;
}

/**
 * @brief Cree un bouton moderne avec icone et texte
 */
static inline lv_obj_t* ui_create_button(lv_obj_t *parent, const char *text, const char *icon, 
                                         lv_color_t color, int width, int height) {
  lv_obj_t *btn = lv_button_create(parent);
  lv_obj_set_size(btn, width, height);
  lv_obj_set_style_bg_color(btn, color, 0);
  lv_obj_set_style_radius(btn, 15, 0);
  lv_obj_set_style_shadow_width(btn, 5, 0);
  lv_obj_set_style_shadow_color(btn, lv_color_black(), 0);
  lv_obj_set_style_shadow_opa(btn, LV_OPA_20, 0);
  lv_obj_set_style_bg_color(btn, lv_color_darken(color, 20), LV_STATE_PRESSED);

  if (icon) {
    lv_obj_t *icon_label = lv_label_create(btn);
    lv_label_set_text(icon_label, icon);
    lv_obj_set_style_text_font(icon_label, &lv_font_montserrat_24, 0);
    lv_obj_set_style_text_color(icon_label, lv_color_white(), 0);
    lv_obj_align(icon_label, LV_ALIGN_LEFT_MID, 20, 0);
  }

  if (text) {
    lv_obj_t *text_label = lv_label_create(btn);
    lv_label_set_text(text_label, text);
    lv_obj_set_style_text_font(text_label, &lv_font_montserrat_20, 0);
    lv_obj_set_style_text_color(text_label, lv_color_white(), 0);
    lv_obj_align(text_label, icon ? LV_ALIGN_LEFT_MID : LV_ALIGN_CENTER, icon ? 70 : 0, 0);
  }

  return btn;
}

/**
 * @brief Cree un bouton simple (sans icone)
 */
static inline lv_obj_t* ui_create_simple_button(lv_obj_t *parent, const char *text, 
                                                 lv_color_t color, int width, int height) {
  lv_obj_t *btn = lv_button_create(parent);
  lv_obj_set_size(btn, width, height);
  lv_obj_set_style_bg_color(btn, color, 0);
  lv_obj_set_style_radius(btn, 12, 0);
  lv_obj_set_style_shadow_width(btn, 5, 0);
  lv_obj_set_style_shadow_color(btn, lv_color_black(), 0);
  lv_obj_set_style_shadow_opa(btn, LV_OPA_20, 0);
  lv_obj_set_style_bg_color(btn, lv_color_darken(color, 20), LV_STATE_PRESSED);

  lv_obj_t *label = lv_label_create(btn);
  lv_label_set_text(label, text);
  lv_obj_set_style_text_font(label, &lv_font_montserrat_24, 0);
  lv_obj_set_style_text_color(label, lv_color_white(), 0);
  lv_obj_center(label);

  return btn;
}

/**
 * @brief Cree un conteneur flex transparent
 */
static inline lv_obj_t* ui_create_flex_container(lv_obj_t *parent, lv_flex_flow_t flow) {
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
static inline lv_obj_t* ui_create_label(lv_obj_t *parent, const char *text, 
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
static inline lv_obj_t* ui_create_separator(lv_obj_t *parent) {
  lv_obj_t *sep = lv_obj_create(parent);
  lv_obj_set_size(sep, lv_pct(100), 1);
  lv_obj_set_style_bg_color(sep, lv_color_hex(0x2a3f5f), 0);
  lv_obj_set_style_border_width(sep, 0, 0);
  return sep;
}

/**
 * @brief Cree un champ de saisie texte
 */
static inline lv_obj_t* ui_create_textarea(lv_obj_t *parent, const char *placeholder, 
                                           int max_length, bool one_line) {
  lv_obj_t *ta = lv_textarea_create(parent);
  lv_obj_set_size(ta, lv_pct(100), one_line ? 55 : 120);
  if (one_line) {
    lv_textarea_set_one_line(ta, true);
  }
  lv_textarea_set_max_length(ta, max_length);
  lv_textarea_set_placeholder_text(ta, placeholder);
  lv_obj_set_style_bg_color(ta, lv_color_hex(0x1a2035), 0);
  lv_obj_set_style_border_color(ta, lv_color_hex(0x6080a0), 0);
  lv_obj_set_style_border_width(ta, 2, 0);
  lv_obj_set_style_radius(ta, 8, 0);
  lv_obj_set_style_text_color(ta, lv_color_white(), 0);
  lv_obj_set_style_text_font(ta, &lv_font_montserrat_20, 0);
  lv_obj_set_style_pad_all(ta, 8, 0);
  return ta;
}

/**
 * @brief Cree un input field avec label
 */
static inline lv_obj_t* ui_create_input_field(lv_obj_t *parent, const char *label_text, 
                                               const char *placeholder, int max_length) {
  lv_obj_t *container = ui_create_flex_container(parent, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_width(container, lv_pct(100));
  lv_obj_set_style_pad_row(container, 5, 0);

  lv_obj_t *label = ui_create_label(container, label_text, 
                                     &lv_font_montserrat_20, lv_color_hex(0x00d4ff));
  
  lv_obj_t *ta = ui_create_textarea(container, placeholder, max_length, true);
  
  return ta;
}

/**
 * @brief Cree un clavier LVGL style
 */
static inline lv_obj_t* ui_create_keyboard(lv_obj_t *parent, lv_keyboard_mode_t mode) {
  lv_obj_t *kb = lv_keyboard_create(parent);
  lv_obj_set_size(kb, lv_pct(98), 220);
  lv_obj_align(kb, LV_ALIGN_BOTTOM_MID, 0, -70);
  lv_obj_set_style_bg_color(kb, lv_color_hex(0x1a2035), 0);
  lv_obj_set_style_bg_opa(kb, LV_OPA_90, 0);
  lv_obj_set_style_border_width(kb, 2, 0);
  lv_obj_set_style_border_color(kb, lv_color_hex(0x6080a0), 0);
  lv_obj_set_style_radius(kb, 10, 0);
  lv_keyboard_set_mode(kb, mode);
  lv_obj_add_flag(kb, LV_OBJ_FLAG_HIDDEN);
  lv_obj_clear_flag(kb, LV_OBJ_FLAG_SCROLLABLE);
  return kb;
}

/**
 * @brief Cree une ligne de formulaire horizontale avec label et widget
 */
static inline lv_obj_t* ui_create_form_row(lv_obj_t *parent, const char *label_text, 
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
 * @brief Cree un panel de colonne avec bordure (pour formulaires)
 */
static inline lv_obj_t* ui_create_form_column(lv_obj_t *parent, int width) {
  lv_obj_t *col = lv_obj_create(parent);
  lv_obj_set_size(col, width, LV_SIZE_CONTENT);
  lv_obj_set_style_bg_color(col, lv_color_hex(0x1a2035), 0);
  lv_obj_set_style_bg_opa(col, LV_OPA_80, 0);
  lv_obj_set_style_border_width(col, 2, 0);
  lv_obj_set_style_border_color(col, lv_color_hex(0x6080a0), 0);
  lv_obj_set_style_radius(col, 15, 0);
  lv_obj_set_style_pad_all(col, 15, 0);
  lv_obj_set_flex_flow(col, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_flex_align(col, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
  lv_obj_set_style_pad_row(col, 15, 0);
  lv_obj_clear_flag(col, LV_OBJ_FLAG_SCROLLABLE);
  return col;
}

/**
 * @brief Style commun pour bouton retour
 */
static inline lv_obj_t* ui_create_back_button(lv_obj_t *parent, const char *text) {
  lv_obj_t *btn = lv_button_create(parent);
  lv_obj_set_size(btn, 300, 70);
  lv_obj_align(btn, LV_ALIGN_BOTTOM_MID, 0, -15);
  lv_obj_set_style_bg_color(btn, lv_color_hex(0xff3b30), 0);
  lv_obj_set_style_radius(btn, 15, 0);
  lv_obj_set_style_shadow_width(btn, 5, 0);
  lv_obj_set_style_shadow_color(btn, lv_color_black(), 0);
  lv_obj_set_style_shadow_opa(btn, LV_OPA_20, 0);
  lv_obj_set_style_bg_color(btn, lv_color_darken(lv_color_hex(0xff3b30), 20), LV_STATE_PRESSED);

  lv_obj_t *icon = lv_label_create(btn);
  lv_label_set_text(icon, LV_SYMBOL_LEFT);
  lv_obj_set_style_text_font(icon, &lv_font_montserrat_24, 0);
  lv_obj_set_style_text_color(icon, lv_color_white(), 0);
  lv_obj_align(icon, LV_ALIGN_LEFT_MID, 25, 0);

  lv_obj_t *label = lv_label_create(btn);
  lv_label_set_text(label, text);
  lv_obj_set_style_text_font(label, &lv_font_montserrat_24, 0);
  lv_obj_set_style_text_color(label, lv_color_white(), 0);
  lv_obj_align(label, LV_ALIGN_CENTER, 10, 0);

  return btn;
}

/**
 * @brief Cree un bouton moderne avec grande icone et texte (style prestart)
 */
static inline lv_obj_t* ui_create_modern_button(lv_obj_t *parent, const char *text, 
                                                 const char *icon, lv_color_t color) {
  lv_obj_t *btn = lv_button_create(parent);
  lv_obj_set_size(btn, BTN_WIDTH, BTN_HEIGHT);
  lv_obj_set_style_bg_color(btn, color, 0);
  lv_obj_set_style_radius(btn, 15, 0);
  lv_obj_set_style_shadow_width(btn, 5, 0);
  lv_obj_set_style_shadow_color(btn, lv_color_black(), 0);
  lv_obj_set_style_shadow_opa(btn, LV_OPA_20, 0);
  lv_obj_set_style_bg_color(btn, lv_color_darken(color, 20), LV_STATE_PRESSED);

  // Icon label
  lv_obj_t *icon_label = lv_label_create(btn);
  lv_label_set_text(icon_label, icon);
  lv_obj_set_style_text_font(icon_label, &lv_font_montserrat_32, 0);
  lv_obj_set_style_text_color(icon_label, lv_color_white(), 0);
  lv_obj_align(icon_label, LV_ALIGN_LEFT_MID, 40, 0);

  // Text label
  lv_obj_t *text_label = lv_label_create(btn);
  lv_label_set_text(text_label, text);
  lv_obj_set_style_text_font(text_label, &lv_font_montserrat_24, 0);
  lv_obj_set_style_text_color(text_label, lv_color_white(), 0);
  lv_obj_align(text_label, LV_ALIGN_CENTER, 20, 0);

  return btn;
}

/**
 * @brief Cree un bouton de menu settings (icone petite a gauche)
 */
static inline lv_obj_t* ui_create_settings_button(lv_obj_t *parent, const char *text, 
                                                   const char *icon, lv_color_t color) {
  lv_obj_t *btn = lv_button_create(parent);
  lv_obj_set_size(btn, 440, 80);
  lv_obj_set_style_bg_color(btn, color, 0);
  lv_obj_set_style_radius(btn, 12, 0);
  lv_obj_set_style_shadow_width(btn, 5, 0);
  lv_obj_set_style_shadow_color(btn, lv_color_black(), 0);
  lv_obj_set_style_shadow_opa(btn, LV_OPA_20, 0);
  lv_obj_set_style_bg_color(btn, lv_color_darken(color, 20), LV_STATE_PRESSED);

  // Icon
  lv_obj_t *icon_label = lv_label_create(btn);
  lv_label_set_text(icon_label, icon);
  lv_obj_set_style_text_font(icon_label, &lv_font_montserrat_24, 0);
  lv_obj_set_style_text_color(icon_label, lv_color_white(), 0);
  lv_obj_align(icon_label, LV_ALIGN_LEFT_MID, 20, 0);

  // Text
  lv_obj_t *text_label = lv_label_create(btn);
  lv_label_set_text(text_label, text);
  lv_obj_set_style_text_font(text_label, &lv_font_montserrat_20, 0);
  lv_obj_set_style_text_color(text_label, lv_color_white(), 0);
  lv_obj_align(text_label, LV_ALIGN_LEFT_MID, 70, 0);

  return btn;
}

/**
 * @brief Cree un bouton exit/power avec icone et texte
 */
static inline lv_obj_t* ui_create_exit_button(lv_obj_t *parent, const char *text) {
  lv_obj_t *btn = lv_button_create(parent);
  lv_obj_set_size(btn, 300, 80);
  lv_obj_align(btn, LV_ALIGN_BOTTOM_MID, 0, -20);
  lv_obj_set_style_bg_color(btn, lv_color_hex(0xff3b30), 0);
  lv_obj_set_style_radius(btn, 15, 0);
  lv_obj_set_style_shadow_width(btn, 5, 0);
  lv_obj_set_style_shadow_color(btn, lv_color_black(), 0);
  lv_obj_set_style_shadow_opa(btn, LV_OPA_20, 0);
  lv_obj_set_style_bg_color(btn, lv_color_darken(lv_color_hex(0xff3b30), 20), LV_STATE_PRESSED);

  lv_obj_t *icon = lv_label_create(btn);
  lv_label_set_text(icon, LV_SYMBOL_POWER);
  lv_obj_set_style_text_font(icon, &lv_font_montserrat_32, 0);
  lv_obj_set_style_text_color(icon, lv_color_white(), 0);
  lv_obj_align(icon, LV_ALIGN_LEFT_MID, 30, 0);

  lv_obj_t *label = lv_label_create(btn);
  lv_label_set_text(label, text);
  lv_obj_set_style_text_font(label, &lv_font_montserrat_24, 0);
  lv_obj_set_style_text_color(label, lv_color_white(), 0);
  lv_obj_align(label, LV_ALIGN_CENTER, 15, 0);

  return btn;
}

/**
 * @brief Cree un panel d'information avec bordure (style prestart)
 */
static inline lv_obj_t* ui_create_info_panel_bordered(lv_obj_t *parent, int width, int height) {
  lv_obj_t *panel = lv_obj_create(parent);
  lv_obj_set_size(panel, width, height);
  lv_obj_set_style_bg_color(panel, lv_color_hex(0x1a2035), 0);
  lv_obj_set_style_bg_opa(panel, LV_OPA_80, 0);
  lv_obj_set_style_border_width(panel, 3, 0);
  lv_obj_set_style_border_color(panel, lv_color_hex(0x6080a0), 0);
  lv_obj_set_style_radius(panel, 15, 0);
  lv_obj_set_style_pad_all(panel, 20, 0);
  lv_obj_set_flex_flow(panel, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_flex_align(panel, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
  return panel;
}

/**
 * @brief Cree une paire de boutons Save/Cancel
 */
typedef struct {
  lv_obj_t *save;
  lv_obj_t *cancel;
} ui_button_pair_t;

static inline ui_button_pair_t ui_create_save_cancel_buttons(lv_obj_t *parent, const char *save_text, 
                                                              const char *cancel_text) {
  ui_button_pair_t buttons;
  
  lv_obj_t *container = ui_create_flex_container(parent, LV_FLEX_FLOW_ROW);
  lv_obj_set_flex_align(container, LV_FLEX_ALIGN_SPACE_EVENLY, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
  
  buttons.save = ui_create_simple_button(container, save_text, lv_color_hex(0x34c759), 220, 50);
  buttons.cancel = ui_create_simple_button(container, cancel_text, lv_color_hex(0xff3b30), 220, 50);
  
  return buttons;
}

#endif