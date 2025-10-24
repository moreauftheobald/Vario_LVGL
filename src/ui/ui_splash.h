#ifndef UI_SPLASH_H
#define UI_SPLASH_H

#include "lvgl.h"
#include "constants.h"

// Forward declaration
void ui_prestart_show(void);

// Declare the logo data
LV_IMG_DECLARE(logo_bipbiphourra);

static lv_obj_t *screen_splash = NULL;

// Callback pour fermer le splash screen
static void splash_timer_cb(lv_timer_t *timer) {
  if (screen_splash) {
    ui_prestart_show();
    lv_obj_del(screen_splash);
    screen_splash = NULL;
  }
}

void ui_splash_init(void) {
  screen_splash = lv_obj_create(NULL);
  lv_obj_set_style_bg_color(screen_splash, lv_color_hex(0xFFF7E6), 0);  // Couleur beige clair

  // Logo image
  lv_obj_t *logo = lv_image_create(screen_splash);
  lv_image_set_src(logo, &logo_bipbiphourra);
  lv_obj_center(logo);

  // Timer pour fermer apres 10 secondes
  lv_timer_create(splash_timer_cb, 3000, NULL);

#ifdef DEBUG_MODE
  Serial.println("Splash screen initialized");
#endif
}

void ui_splash_show(void) {
  if (screen_splash == NULL) {
    ui_splash_init();
  }
  lv_screen_load(screen_splash);
}

#endif