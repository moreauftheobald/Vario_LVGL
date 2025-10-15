#ifndef UI_PRESTART_H
#define UI_PRESTART_H

#include "lvgl.h"
#include "constants.h"

// Screen objects
static lv_obj_t *screen_prestart = NULL;
static lv_obj_t *btn_file_transfer = NULL;
static lv_obj_t *btn_settings = NULL;
static lv_obj_t *btn_start = NULL;
static lv_obj_t *label_title = NULL;
static lv_obj_t *label_version = NULL;
static lv_obj_t *info_panel = NULL;

// Callback declarations
static void btn_file_transfer_cb(lv_event_t *e);
static void btn_settings_cb(lv_event_t *e);
static void btn_start_cb(lv_event_t *e);
void ui_file_transfer_show(void);
void ui_settings_show(void);

/**
 * @brief Create a modern styled button with icon and label
 */
static lv_obj_t *create_modern_button(lv_obj_t *parent, const char *text, const char *icon, lv_color_t color, int y_offset) {
  lv_obj_t *btn = lv_button_create(parent);
  lv_obj_set_size(btn, BTN_WIDTH, BTN_HEIGHT);
  lv_obj_align(btn, LV_ALIGN_CENTER, 0, y_offset);

  // Button styling simplifie
  lv_obj_set_style_bg_color(btn, color, 0);
  lv_obj_set_style_radius(btn, 15, 0);
  lv_obj_set_style_shadow_width(btn, 5, 0);
  lv_obj_set_style_shadow_color(btn, lv_color_black(), 0);
  lv_obj_set_style_shadow_opa(btn, LV_OPA_20, 0);

  // Pressed state
  lv_obj_set_style_bg_color(btn, lv_color_darken(color, 20), LV_STATE_PRESSED);

  // Creer les labels directement sur le bouton (sans container intermediaire)
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
 * @brief Initialize prestart screen
 */
void ui_prestart_init(void) {
  const TextStrings *txt = get_text();

  // Create screen with gradient background
  screen_prestart = lv_obj_create(NULL);
  lv_obj_set_style_bg_color(screen_prestart, lv_color_hex(0x0a0e27), 0);
  lv_obj_set_style_bg_grad_color(screen_prestart, lv_color_hex(0x1a1f3a), 0);
  lv_obj_set_style_bg_grad_dir(screen_prestart, LV_GRAD_DIR_VER, 0);

  // Main container with rounded border frame
  lv_obj_t *main_frame = lv_obj_create(screen_prestart);
  lv_obj_set_size(main_frame, SCREEN_WIDTH - 10, SCREEN_HEIGHT - 10);
  lv_obj_center(main_frame);
  lv_obj_set_style_bg_color(main_frame, lv_color_hex(0x151932), 0);
  lv_obj_set_style_bg_opa(main_frame, LV_OPA_90, 0);
  lv_obj_set_style_border_width(main_frame, 3, 0);
  lv_obj_set_style_border_color(main_frame, lv_color_hex(0x6080a0), 0);
  lv_obj_set_style_radius(main_frame, 20, 0);
  lv_obj_set_style_pad_all(main_frame, 20, 0);
  lv_obj_set_style_shadow_width(main_frame, 30, 0);
  lv_obj_set_style_shadow_color(main_frame, lv_color_black(), 0);
  lv_obj_set_style_shadow_opa(main_frame, LV_OPA_40, 0);

  // Title (no container, directly on main_frame)
  label_title = lv_label_create(main_frame);
  lv_label_set_text(label_title, VARIO_NAME);
  lv_obj_set_style_text_font(label_title, &lv_font_montserrat_48, 0);
  lv_obj_set_style_text_color(label_title, lv_color_hex(0x00d4ff), 0);
  lv_obj_set_style_text_align(label_title, LV_TEXT_ALIGN_CENTER, 0);
  lv_obj_align(label_title, LV_ALIGN_TOP_MID, 0, 5);
  lv_obj_set_style_bg_opa(label_title, LV_OPA_TRANSP, 0);
  lv_obj_set_style_pad_all(label_title, 0, 0);

  // Main content container (2 columns layout)
  lv_obj_t *content_container = lv_obj_create(main_frame);
  lv_obj_set_size(content_container, lv_pct(100), LV_SIZE_CONTENT);
  lv_obj_align(content_container, LV_ALIGN_CENTER, 0, 40);
  lv_obj_set_style_bg_opa(content_container, LV_OPA_TRANSP, 0);
  lv_obj_set_style_border_width(content_container, 0, 0);
  lv_obj_set_style_pad_all(content_container, 0, 0);
  lv_obj_set_flex_flow(content_container, LV_FLEX_FLOW_ROW);
  lv_obj_set_flex_align(content_container, LV_FLEX_ALIGN_SPACE_EVENLY, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
  lv_obj_set_style_pad_column(content_container, 30, 0);

  // Left column: Buttons container
  lv_obj_t *btn_container = lv_obj_create(content_container);
  lv_obj_set_size(btn_container, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
  lv_obj_set_style_bg_opa(btn_container, LV_OPA_TRANSP, 0);
  lv_obj_set_style_border_width(btn_container, 0, 0);
  lv_obj_set_style_pad_all(btn_container, 0, 0);
  lv_obj_set_flex_flow(btn_container, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_flex_align(btn_container, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
  lv_obj_set_style_pad_row(btn_container, 30, 0);

  // Button 1: File Transfer (Orange/Yellow)
  btn_file_transfer = create_modern_button(btn_container, txt->file_transfer, LV_SYMBOL_USB,
                                           lv_color_hex(0xff9500), 0);
  lv_obj_add_event_cb(btn_file_transfer, btn_file_transfer_cb, LV_EVENT_CLICKED, NULL);

  // Button 2: Settings (Blue)
  btn_settings = create_modern_button(btn_container, txt->settings, LV_SYMBOL_SETTINGS,
                                      lv_color_hex(0x007aff), 0);
  lv_obj_add_event_cb(btn_settings, btn_settings_cb, LV_EVENT_CLICKED, NULL);

  // Button 3: Start (Green)
  btn_start = create_modern_button(btn_container, txt->start, LV_SYMBOL_PLAY,
                                   lv_color_hex(0x34c759), 0);
  lv_obj_add_event_cb(btn_start, btn_start_cb, LV_EVENT_CLICKED, NULL);

  // Right column: Information panel
  info_panel = lv_obj_create(content_container);
  lv_obj_set_size(info_panel, 550, 450);
  lv_obj_set_style_bg_color(info_panel, lv_color_hex(0x1a2035), 0);
  lv_obj_set_style_bg_opa(info_panel, LV_OPA_80, 0);
  lv_obj_set_style_border_width(info_panel, 3, 0);
  lv_obj_set_style_border_color(info_panel, lv_color_hex(0x6080a0), 0);
  lv_obj_set_style_radius(info_panel, 15, 0);
  lv_obj_set_style_pad_all(info_panel, 20, 0);
  lv_obj_set_flex_flow(info_panel, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_flex_align(info_panel, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
  lv_obj_set_style_pad_row(info_panel, 12, 0);

  // Info panel title
  lv_obj_t *info_title = lv_label_create(info_panel);
  lv_label_set_text(info_title, "Informations");
  lv_obj_set_style_text_font(info_title, &lv_font_montserrat_24, 0);
  lv_obj_set_style_text_color(info_title, lv_color_hex(0x00d4ff), 0);
  lv_obj_set_width(info_title, lv_pct(100));

  // Separator line
  lv_obj_t *separator = lv_obj_create(info_panel);
  lv_obj_set_size(separator, lv_pct(100), 1);
  lv_obj_set_style_bg_color(separator, lv_color_hex(0x2a3f5f), 0);
  lv_obj_set_style_border_width(separator, 0, 0);

  // Version info as first item
  label_version = lv_label_create(info_panel);
  lv_label_set_text(label_version, LV_SYMBOL_SETTINGS " Version: " VARIO_VERSION);
  lv_obj_set_style_text_font(label_version, &lv_font_montserrat_20, 0);
  lv_obj_set_style_text_color(label_version, lv_color_hex(0xaabbcc), 0);
  lv_obj_set_width(label_version, lv_pct(100));

  // Placeholder info items (will be populated later)
  const char *info_items[] = {
    LV_SYMBOL_SD_CARD " Carte SD: --",
    LV_SYMBOL_DIRECTORY " Espace libre: --",
    LV_SYMBOL_IMAGE " Cartes: --",
    LV_SYMBOL_LIST " Vols: --",
    LV_SYMBOL_HOME " Pilote: --"
  };

  for (int i = 0; i < 5; i++) {
    lv_obj_t *info_item = lv_label_create(info_panel);
    lv_label_set_text(info_item, info_items[i]);
    lv_obj_set_style_text_font(info_item, &lv_font_montserrat_20, 0);
    lv_obj_set_style_text_color(info_item, lv_color_hex(0xaabbcc), 0);
    lv_obj_set_width(info_item, lv_pct(100));
  }

#ifdef DEBUG_MODE
  Serial.println("Prestart screen initialized");
#endif
}

/**
 * @brief Show prestart screen
 */
void ui_prestart_show(void) {
  if (screen_prestart == NULL) {
    ui_prestart_init();
  }
  lv_screen_load(screen_prestart);

#ifdef DEBUG_MODE
  Serial.println("Prestart screen shown");
#endif
}

/**
 * @brief File transfer button callback
 */
static void btn_file_transfer_cb(lv_event_t *e) {
#ifdef DEBUG_MODE
  Serial.println("File transfer button clicked");
#endif
  ui_file_transfer_show();
}

/**
 * @brief Settings button callback
 */
static void btn_settings_cb(lv_event_t *e) {
#ifdef DEBUG_MODE
  Serial.println("Settings button clicked");
#endif
  ui_settings_show();
}

/**
 * @brief Start button callback
 */
static void btn_start_cb(lv_event_t *e) {
#ifdef DEBUG_MODE
  Serial.println("Start button clicked");
#endif
  // TODO: Implement main vario screen
}

#endif