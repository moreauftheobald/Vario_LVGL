#ifndef CONSTANTS_H
#define CONSTANTS_H
/*=========================================================================
Project Global constants
/*=========================================================================*/
// Project name
#define VARIO_NAME "Bip-Bip-Hourra"

// Version
#define VARIO_VERSION "1.0.0"

//Debug Mode
#define DEBUG_MODE

// Mode test
//#define TEST_MODE  // Decommente pour activer

// Mode test
#define FLIGHT_TEST_MODE  // Decommente pour activer

//LVGL Include Mode
#define LV_CONF_INCLUDE_SIMPLE

/*=========================================================================
I2C GPIO AND CONSTANTS
/*=========================================================================*/
//I2C GPIO AND CONSTANTS
#define I2C_MASTER_SDA GPIO_NUM_8
#define I2C_MASTER_SCL GPIO_NUM_9
#define PIN_NUM_TOUCH_RST (GPIO_NUM_NC)
#define PIN_NUM_TOUCH_INT (GPIO_NUM_4)

//I2C Adress
#define BMP390_I2C_ADDR (0x77)
#define BNO080_I2C_ADDR (0x4A)
#define GPS_I2C_ADDR (0x10)
#define ESP_LCD_TOUCH_IO_I2C_GT911_ADDRESS (0x5D)
#define ESP_LCD_TOUCH_IO_I2C_GT911_ADDRESS_BACKUP (0x14)
#define IO_EXTENSION_ADDR 0x24

//I2C Constants
#define I2C_MASTER_FREQUENCY (400 * 1000)
#define I2C_MASTER_NUM I2C_NUM_0


/*=========================================================================
I2C PERIPHERALS CONSTANTS
/*=========================================================================*/
//Task
#define SENSORS_TASK_STACK_SIZE (2560)
#define SENSORS_TASK_PRIORITY (5)

//BNO080
#define BNO080_SAMPLE_RATE_HZ (100)
#define BNO080_RESET_PIN (-1)

//BMP390
#define BMP390_SAMPLE_RATE_HZ (50)

//GPS
#define GPS_SAMPLE_RATE_HZ (2)
#define PMTK_SET_NMEA_UPDATE_2HZ "$PMTK220,500*2B"
#define PMTK_API_SET_FIX_CTL_2HZ "$PMTK300,500,0,0,0,0*28"

//IO EXTANDER CONSTANTS
#define IO_EXTENSION_Mode 0x02
#define IO_EXTENSION_IO_OUTPUT_ADDR 0x03
#define IO_EXTENSION_IO_INPUT_ADDR 0x04
#define IO_EXTENSION_PWM_ADDR 0x05
#define IO_EXTENSION_ADC_ADDR 0x06
#define IO_EXTENSION_IO_0 0x00
#define IO_EXTENSION_IO_1 0x01
#define IO_EXTENSION_IO_2 0x02
#define IO_EXTENSION_IO_3 0x03
#define IO_EXTENSION_IO_4 0x04
#define IO_EXTENSION_IO_5 0x05
#define IO_EXTENSION_IO_6 0x06
#define IO_EXTENSION_IO_7 0x07


/*=========================================================================
SDMMC GPIO AND CONSTANTS
/*=========================================================================*/
// Pins SDMMC (mode 1-bit)
#define SD_PIN_CLK GPIO_NUM_12
#define SD_PIN_CMD GPIO_NUM_11
#define SD_PIN_D0  GPIO_NUM_13
#define SD_CS_PIN_EXTENDER IO_EXTENSION_IO_4

// Point de montage
#define SD_MOUNT_POINT "/sdcard"

// Repertoires
#define OSM_TILES_DIR "/osm_tiles"
#define LOGS_DIR "/logs"
#define FLIGHTS_DIR "/flights"
#define CONFIG_DIR "/config"

// Option de formatage automatique (mettre true pour forcer FAT32 si echec)
#define SD_FORMAT_IF_MOUNT_FAILED false


/*=========================================================================
LCD GPIO AND CONSTANTS
/*=========================================================================*/
//LCD GPIO
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
#define PIN_NUM_BK_LIGHT (-1)

//LCD Constants
#define LCD_H_RES (1024)
#define LCD_V_RES (600)
#define LCD_PIXEL_CLOCK_HZ (16 * 1000 * 1000)

#define LCD_BIT_PER_PIXEL (16)
#define RGB_BIT_PER_PIXEL (16)
#define RGB_DATA_WIDTH (16)
#define LCD_RGB_BUFFER_NUMS (2)
#define RGB_BOUNCE_BUFFER_SIZE (LCD_H_RES * 15)


#define LCD_BK_LIGHT_ON_LEVEL (1)
#define LCD_BK_LIGHT_OFF_LEVEL (!LCD_BK_LIGHT_ON_LEVEL)


/*=========================================================================
LVGL AND GRAPHICS CONSTANTS
/*=========================================================================*/
// Colors
#define COLOR_PRIMARY lv_palette_main(LV_PALETTE_BLUE)
#define COLOR_SECONDARY lv_palette_main(LV_PALETTE_GREEN)
#define COLOR_WARNING lv_palette_main(LV_PALETTE_ORANGE)
#define COLOR_TEXT lv_color_white()

//LVGL PORT CONSTANTS
#define LVGL_PORT_H_RES (LCD_H_RES)
#define LVGL_PORT_V_RES (LCD_V_RES)
#define LVGL_PORT_TICK_PERIOD_MS (2)
#define LVGL_PORT_TASK_MAX_DELAY_MS (500)
#define LVGL_PORT_TASK_MIN_DELAY_MS (10)
#define LVGL_PORT_TASK_STACK_SIZE (10 * 1024)
#define LVGL_PORT_TASK_PRIORITY (2)
#define LVGL_PORT_TASK_CORE (1)
#define CONFIG_LVGL_PORT_BUF_PSRAM 1
#define CONFIG_LVGL_PORT_BUF_INTERNAL 0
#define LVGL_PORT_BUFFER_MALLOC_CAPS (MALLOC_CAP_SPIRAM)
#define LVGL_PORT_BUFFER_HEIGHT (150)
#define LVGL_PORT_AVOID_TEAR_ENABLE (1)
#define LVGL_PORT_AVOID_TEAR_MODE (3)
#define LVGL_PORT_ROTATION_0 (1)
#define LVGL_PORT_LCD_RGB_BUFFER_NUMS (2)
#define LVGL_PORT_DIRECT_MODE (1)

#define UI_TASK_PRIORITY 2


/*=========================================================================
WIFI CONSTANTS
/*=========================================================================*/
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_DISCONNECTED_BIT BIT1
#define WIFI_START_BIT BIT2
#define WIFI_STOP_BIT BIT3

/*=========================================================================
TILES CONSTANTS
/*=========================================================================*/
// Taille standard tuile OSM
#define OSM_TILE_SIZE 256

// Position test: Décollage parapente Volmerange-les-Mines
//#define TEST_LAT 49.446845
//#define TEST_LON 6.099846

// Position test: 62 rue de verdun 57700 HAYANGE
#define TEST_LAT 49.327626
#define TEST_LON 6.057100

// Nom du serveur de cartes (peut être paramétrable plus tard)
#define OSM_SERVER_NAME "osm"

#define MAP_ZOOM_MIN  6
#define MAP_ZOOM_MAX 16

/*=========================================================================
METAR CONSTANTS
/*=========================================================================*/
// Event bits
#define METAR_FETCH_BIT BIT0
#define METAR_STOP_BIT BIT1

// API config
#define METAR_API_URL "https://aviationweather.gov/api/data/metar"
#define METAR_BBOX_RADIUS 1.0f  // Degres autour de la position (~100km)

/*=========================================================================
OTHER CONSTANTS
/*=========================================================================*/
//KALMAN CONSTANTS
#define INIT_SAMPLES 20

//VARIO INTEGRATION CONSTANTS
#define INT_MIN_PER  1
#define INT_MAX_PER 30

//VARIO AUDIO FREQUENCY RANGE
#define MIN_FREQ   600
#define MAX_FREQ  1400

#endif