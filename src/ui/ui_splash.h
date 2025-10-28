#ifndef UI_SPLASH_H
#define UI_SPLASH_H

#include "lvgl.h"
#include "constants.h"
#include "src/lvgl_port/lvgl_port.h"

// Forward declaration
void ui_prestart_show(void);

// Declare the logo data
LV_IMG_DECLARE(logo_bipbiphourra);

static lv_timer_t *splash_timer = NULL;

// Callback pour fermer le splash screen
static void splash_timer_cb(lv_timer_t *timer) {
#ifdef DEBUG_MODE
  Serial.println("[SPLASH] Timer callback - closing splash");
#endif

  // Supprimer le timer d'abord
  if (splash_timer) {
    lv_timer_del(splash_timer);
    splash_timer = NULL;
  }

  // Appeler show() qui gère tout proprement
  ui_prestart_show();
}

void ui_splash_init(void) {
#ifdef DEBUG_MODE
  Serial.println("[SPLASH] Init");
#endif

  if (!lvgl_port_lock(-1)) {
#ifdef DEBUG_MODE
    Serial.println("[SPLASH] Failed to acquire LVGL lock!");
#endif
    return;
  }

  // CRÉER l'écran AVANT de l'utiliser
  current_screen = lv_obj_create(NULL);
  lv_obj_set_style_bg_color(current_screen, lv_color_hex(0xFFF7E6), 0);

  // Logo image
  lv_obj_t *logo = lv_image_create(current_screen);
  lv_image_set_src(logo, &logo_bipbiphourra);
  lv_obj_center(logo);

  // Charger l'ecran
  lv_screen_load(current_screen);

  lvgl_port_unlock();

  splash_timer = lv_timer_create(splash_timer_cb, 3000, NULL);
  lv_timer_set_repeat_count(splash_timer, 1);

#ifdef DEBUG_MODE
  Serial.println("[SPLASH] Screen loaded, timer started");
#endif
}

void ui_splash_show(void) {
  if (current_screen == NULL) {
    ui_splash_init();
  } else {
    // Si deja initialise, juste afficher
    if (lvgl_port_lock(-1)) {
      lv_screen_load(current_screen);
      lvgl_port_unlock();
    }
  }
}

#endif