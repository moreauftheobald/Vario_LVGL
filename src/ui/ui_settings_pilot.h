#ifndef UI_SETTINGS_PILOT_H
#define UI_SETTINGS_PILOT_H

#include "lvgl.h"
#include "constants.h"
#include "UI_helper.h"
#include "lang.h"
#include "globals.h"
#include "src/params/params.h"

void ui_settings_show(void);

// Structure pour les widgets
typedef struct {
  lv_obj_t *ta_name;
  lv_obj_t *ta_firstname;
  lv_obj_t *ta_wing;
  lv_obj_t *ta_phone;
} pilot_widgets_t;

// Fonctions de sauvegarde/chargement
static void load_pilot_data(pilot_widgets_t *widgets) {
  if (widgets->ta_name)
    lv_textarea_set_text(widgets->ta_name, psram_str_get(params.pilot_name));
  if (widgets->ta_firstname)
    lv_textarea_set_text(widgets->ta_firstname, psram_str_get(params.pilot_firstname));
  if (widgets->ta_wing)
    lv_textarea_set_text(widgets->ta_wing, psram_str_get(params.pilot_wing));
  if (widgets->ta_phone)
    lv_textarea_set_text(widgets->ta_phone, psram_str_get(params.pilot_phone));

#ifdef DEBUG_MODE
  Serial.println("Pilot data loaded from params");
#endif
}

static void save_pilot_data(pilot_widgets_t *widgets) {
  // Sauvegarder depuis les textarea vers PSRAM
  psram_str_set(&params.pilot_name, lv_textarea_get_text(widgets->ta_name));
  psram_str_set(&params.pilot_firstname, lv_textarea_get_text(widgets->ta_firstname));
  psram_str_set(&params.pilot_wing, lv_textarea_get_text(widgets->ta_wing));
  psram_str_set(&params.pilot_phone, lv_textarea_get_text(widgets->ta_phone));

  params_save_pilot();

#ifdef DEBUG_MODE
  Serial.println("Pilot data saved to params");
#endif
}

// Callbacks clavier
static void ta_pilot_event_cb(lv_event_t *e) {
  lv_obj_t *ta = (lv_obj_t *)lv_event_get_target(e);

  if (ta_active != ta) {
    ta_active = ta;
    lv_keyboard_set_textarea(keyboard, ta);
    lv_obj_clear_flag(keyboard, LV_OBJ_FLAG_HIDDEN);
    lv_obj_move_foreground(keyboard);
    force_full_refresh();
  }
}

static void btn_save_pilot_cb(lv_event_t *e) {
  pilot_widgets_t *widgets = (pilot_widgets_t *)lv_event_get_user_data(e);
  if (widgets == NULL) {
    return;
  }

  if (widgets->ta_name == NULL || widgets->ta_firstname == NULL || widgets->ta_wing == NULL || widgets->ta_phone == NULL) {
    return;
  }

  save_pilot_data(widgets);
  ui_settings_show();
}

static void btn_cancel_pilot_cb(lv_event_t *e) {
  ui_settings_show();
}

void ui_settings_pilot_init(void) {
  const TextStrings *txt = get_text();

  lv_obj_t *main_frame = ui_create_black_screen_with_frame(UI_BORDER_MEDIUM, UI_RADIUS_LARGE, &current_screen);
  ui_create_main_frame(main_frame, false, txt->pilot_settings);

  // Widgets statiques
  static pilot_widgets_t widgets;

  // Nom
  lv_obj_t *name_row = ui_create_form_row(main_left, txt->pilot_name, UI_HEADER_LINE_W, lv_color_hex(UI_COLOR_PRIMARY));
  widgets.ta_name = ui_create_textarea(name_row, UI_TEXTAREA_W, UI_TEXTAREA_H, 32, true);
  lv_obj_add_event_cb(widgets.ta_name, ta_pilot_event_cb, LV_EVENT_FOCUSED, NULL);
  
  // Prenom
  lv_obj_t *firstname_row = ui_create_form_row(main_left, txt->pilot_firstname, UI_HEADER_LINE_W, lv_color_hex(UI_COLOR_PRIMARY));
  widgets.ta_firstname = ui_create_textarea(firstname_row, UI_TEXTAREA_W, UI_TEXTAREA_H, 32, true);
  lv_obj_add_event_cb(widgets.ta_firstname, ta_pilot_event_cb, LV_EVENT_FOCUSED, NULL);

  // Voile
  lv_obj_t *wing_row = ui_create_form_row(main_left, txt->pilot_wing, UI_HEADER_LINE_W, lv_color_hex(UI_COLOR_PRIMARY));
  widgets.ta_wing = ui_create_textarea(wing_row, UI_TEXTAREA_W, UI_TEXTAREA_H, 32, true);
  lv_obj_add_event_cb(widgets.ta_wing, ta_pilot_event_cb, LV_EVENT_FOCUSED, NULL);

  // Telephone
  lv_obj_t *phone_row = ui_create_form_row(main_left, txt->pilot_phone, UI_HEADER_LINE_W, lv_color_hex(UI_COLOR_PRIMARY));
  widgets.ta_phone = ui_create_textarea(phone_row, UI_TEXTAREA_W, UI_TEXTAREA_H, 32, true);
  lv_obj_add_event_cb(widgets.ta_phone, ta_pilot_event_cb, LV_EVENT_FOCUSED, NULL);

  // Clavier
  if (!keyboard) {
    keyboard = ui_create_keyboard(main_frame);
    lv_obj_add_event_cb(keyboard, ui_keyboard_common_event_cb, LV_EVENT_ALL, NULL);
  }

  // Bouton Save
  lv_obj_t *btn_save_pilot = ui_create_button(btn_container, txt->save, LV_SYMBOL_SAVE, lv_color_hex(UI_COLOR_BTN_START),
                                              UI_BTN_PRESTART_W, UI_BTN_PRESTART_H, UI_FONT_SMALL, UI_FONT_NORMAL, btn_save_pilot_cb,
                                              &widgets, (lv_align_t)0, NULL, NULL);

  // Bouton Cancel
  lv_obj_t *btn_cancel_pilot = ui_create_button(btn_container, txt->cancel, LV_SYMBOL_BACKSPACE, lv_color_hex(UI_COLOR_BTN_CANCEL),
                                                UI_BTN_PRESTART_W, UI_BTN_PRESTART_H, UI_FONT_SMALL, UI_FONT_NORMAL, btn_cancel_pilot_cb,
                                                NULL, (lv_align_t)0, NULL, NULL);

  // Charger les donnees
  load_pilot_data(&widgets);

#ifdef DEBUG_MODE
  Serial.println("Pilot settings screen initialized");
#endif
}

void ui_settings_pilot_show(void) {
  ui_switch_screen(ui_settings_pilot_init);
}

#endif