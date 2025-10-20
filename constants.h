#ifndef CONSTANTS_H
#define CONSTANTS_H

// Project name
#define VARIO_NAME "Bip-Bip-Hourra"

// Version
#define VARIO_VERSION "1.0.0"

// Screen dimensions
#define SCREEN_WIDTH 1024
#define SCREEN_HEIGHT 600

// Button dimensions
#define BTN_WIDTH 360
#define BTN_HEIGHT 90

// Colors
#define COLOR_PRIMARY lv_palette_main(LV_PALETTE_BLUE)
#define COLOR_SECONDARY lv_palette_main(LV_PALETTE_GREEN)
#define COLOR_WARNING lv_palette_main(LV_PALETTE_ORANGE)
#define COLOR_TEXT lv_color_white()

//I2C Sensors
#define SENSORS_TASK_STACK_SIZE (2560)
#define SENSORS_TASK_PRIORITY (5)
#define BMP390_I2C_ADDR (0x77)
#define BMP390_SAMPLE_RATE_HZ (50)

#define BNO080_I2C_ADDR (0x4A)
#define BNO080_SAMPLE_RATE_HZ (100)
#define BNO080_RESET_PIN (-1)

#define GPS_I2C_ADDR (0x10)
#define GPS_SAMPLE_RATE_HZ (2)
// Commande PMTK personnalisee pour 2Hz
#define PMTK_SET_NMEA_UPDATE_2HZ "$PMTK220,500*2B"
#define PMTK_API_SET_FIX_CTL_2HZ "$PMTK300,500,0,0,0,0*28"
#endif
