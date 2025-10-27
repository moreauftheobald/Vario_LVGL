#ifndef UI_SPLASH_H
#define UI_SPLASH_H

#include "lvgl.h"
#include "constants.h"
#include "src/lvgl_port/lvgl_port.h"

// Forward declaration
void ui_prestart_show(void);

// Declare the logo data
LV_IMG_DECLARE(logo_bipbiphourra);

static lv_obj_t *screen_splash = NULL;
static lv_timer_t *splash_timer = NULL;

// Callback pour fermer le splash screen
static void splash_timer_cb(lv_timer_t *timer) {
#ifdef DEBUG_MODE
  Serial.println("[SPLASH] Timer callback - closing splash");
#endif

  if (screen_splash) {
    // Nettoyer le splash
    lv_obj_del(screen_splash);
    screen_splash = NULL;
    
    // Appeler DIRECTEMENT init (pas show) car on est déjà dans un contexte LVGL locké
    ui_prestart_init();
  }
  
  // Supprimer le timer
  if (splash_timer) {
    lv_timer_del(splash_timer);
    splash_timer = NULL;
  }
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

  screen_splash = lv_obj_create(NULL);
  lv_obj_set_style_bg_color(screen_splash, lv_color_hex(0xFFF7E6), 0);

  // Logo image
  lv_obj_t *logo = lv_image_create(screen_splash);
  lv_image_set_src(logo, &logo_bipbiphourra);
  lv_obj_center(logo);

  // Charger l'ecran
  
  lv_screen_load(screen_splash);

  lvgl_port_unlock();

  splash_timer = lv_timer_create(splash_timer_cb, 3000, NULL);
  lv_timer_set_repeat_count(splash_timer, 1);  // S'exécute une seule fois

#ifdef DEBUG_MODE
  Serial.println("[SPLASH] Screen loaded, timer started");
#endif
}

void ui_splash_show(void) {
  if (screen_splash == NULL) {
    ui_splash_init();
  } else {
    // Si deja initialise, juste afficher
    if (lvgl_port_lock(-1)) {
      lv_screen_load(screen_splash);
      lvgl_port_unlock();
    }
  }
}

#endif