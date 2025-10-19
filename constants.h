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

//ESP32 S3 PINS
#define LCD_IO_RGB_DISP (-1)
#define LCD_IO_RGB_VSYNC (GPIO_NUM_3)
#define LCD_IO_RGB_HSYNC (GPIO_NUM_46)
#define LCD_IO_RGB_DE (GPIO_NUM_5)
#define LCD_IO_RGB_PCLK (GPIO_NUM_7)

#define LCD_IO_RGB_DATA0 (GPIO_NUM_14)
#define LCD_IO_RGB_DATA1 (GPIO_NUM_38)
#define LCD_IO_RGB_DATA2 (GPIO_NUM_18)
#define LCD_IO_RGB_DATA3 (GPIO_NUM_17)
#define LCD_IO_RGB_DATA4 (GPIO_NUM_10)
#define LCD_IO_RGB_DATA5 (GPIO_NUM_39)
#define LCD_IO_RGB_DATA6 (GPIO_NUM_0)
#define LCD_IO_RGB_DATA7 (GPIO_NUM_45)
#define LCD_IO_RGB_DATA8 (GPIO_NUM_48)
#define LCD_IO_RGB_DATA9 (GPIO_NUM_47)
#define LCD_IO_RGB_DATA10 (GPIO_NUM_21)
#define LCD_IO_RGB_DATA11 (GPIO_NUM_1)
#define LCD_IO_RGB_DATA12 (GPIO_NUM_2)
#define LCD_IO_RGB_DATA13 (GPIO_NUM_42)
#define LCD_IO_RGB_DATA14 (GPIO_NUM_41)
#define LCD_IO_RGB_DATA15 (GPIO_NUM_40)
#define LCD_IO_RST (-1)

#endif
