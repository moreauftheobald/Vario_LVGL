#ifndef GLOBALS_H
#define GLOBALS_H

#include <Preferences.h>
#include "lvgl.h"

// Variables globales partagees entre ecrans de parametres
extern Preferences prefs;
extern lv_obj_t *ta_active;
extern lv_obj_t *keyboard;

// Fonction utilitaire partagee
void force_full_refresh(void);

#endif