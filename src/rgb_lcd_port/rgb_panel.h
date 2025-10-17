#ifndef RGB_PANEL_H
#define RGB_PANEL_H

#include <ESP_Panel_Library.h>
#include "io_extension.h"

static ESP_Panel *panel = nullptr;

void RGB_Panel_Init() {
#ifdef DEBUG_MODE
  Serial.println("Init RGB Panel");
#endif

  panel = new ESP_Panel();

  panel->init();
  panel->begin();

  IOExt_Backlight(true);

#ifdef DEBUG_MODE
  Serial.println("RGB Panel OK");
#endif
}

#endif