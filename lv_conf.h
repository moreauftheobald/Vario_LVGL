/**
 * LVGL Configuration for ESP32-S3 with RGB LCD 1024x600
 */

#ifndef LV_CONF_H
#define LV_CONF_H

#include <stdint.h>

/* ====================
   COLOR SETTINGS
   ==================== */
#define LV_COLOR_DEPTH 16
#define LV_COLOR_16_SWAP 0

/* ====================
   MEMORY SETTINGS
   ==================== */
#define LV_MEM_CUSTOM 0
#define LV_MEM_SIZE (128 * 1024U)
#define LV_MEM_ADR 0
#define LV_MEM_BUF_MAX_NUM 16

/* ====================
   HAL SETTINGS
   ==================== */
#define LV_TICK_CUSTOM 0
#define LV_DPI_DEF 130

/* ====================
   COMPILER SETTINGS
   ==================== */
#define LV_BIG_ENDIAN_SYSTEM 0
#define LV_ATTRIBUTE_FAST_MEM
#define LV_ATTRIBUTE_TASK_HANDLER
#define LV_USE_PERF_MONITOR 0
#define LV_USE_MEM_MONITOR 0

/* ====================
   FEATURE SETTINGS
   ==================== */
#define LV_USE_LOG 0
#define LV_USE_ASSERT_NULL 1
#define LV_USE_ASSERT_MALLOC 1
#define LV_USE_ASSERT_STYLE 0
#define LV_USE_PERF_MONITOR 0

/* ====================
   DRAWING SETTINGS (CRITICAL)
   ==================== */
#define LV_USE_DRAW_SW 1

/* DISABLE ARM/NEON optimizations for Xtensa (ESP32) */
#define LV_USE_DRAW_SW_ASM LV_DRAW_SW_ASM_NONE
#define LV_DRAW_SW_SUPPORT_ARGB8888 0
#define LV_DRAW_SW_SUPPORT_RGB888 0
#define LV_DRAW_SW_SUPPORT_RGB565 1

/* ====================
   GPU SETTINGS
   ==================== */
#define LV_USE_DRAW_VGLITE 0
#define LV_USE_DRAW_PXP 0
#define LV_USE_DRAW_OPENGLES 0

/* ====================
   OPERATING SYSTEM
   ==================== */
#define LV_USE_OS LV_OS_FREERTOS
#define LV_USE_STDLIB_MALLOC LV_STDLIB_BUILTIN
#define LV_USE_STDLIB_STRING LV_STDLIB_BUILTIN
#define LV_USE_STDLIB_SPRINTF LV_STDLIB_BUILTIN

/* ====================
   WIDGETS
   ==================== */
#define LV_USE_LABEL 1
#define LV_USE_BUTTON 1
#define LV_USE_IMAGE 1
#define LV_USE_SLIDER 1
#define LV_USE_ARC 1
#define LV_USE_CHART 1
#define LV_USE_LINE 1
#define LV_USE_CANVAS 1

/* Disable unused widgets to save memory */
#define LV_USE_BAR 1
#define LV_USE_BUTTONMATRIX 1
#define LV_USE_CALENDAR 0
#define LV_USE_CHECKBOX 0
#define LV_USE_DROPDOWN 1
#define LV_USE_KEYBOARD 1
#define LV_USE_LED 0
#define LV_USE_LIST 0
#define LV_USE_MENU 0
#define LV_USE_MSGBOX 0
#define LV_USE_ROLLER 0
#define LV_USE_SPAN 0
#define LV_USE_SPINBOX 0
#define LV_USE_SPINNER 0
#define LV_USE_SWITCH 0
#define LV_USE_TABLE 0
#define LV_USE_TABVIEW 0
#define LV_USE_TEXTAREA 1
#define LV_USE_TILEVIEW 0
#define LV_USE_WIN 0

/* ====================
   THEMES
   ==================== */
#define LV_USE_THEME_DEFAULT 1
#define LV_USE_THEME_MONO 0

/* ====================
   FONTS
   ==================== */
#define LV_FONT_MONTSERRAT_8 0
#define LV_FONT_MONTSERRAT_10 0
#define LV_FONT_MONTSERRAT_12 1
#define LV_FONT_MONTSERRAT_14 1
#define LV_FONT_MONTSERRAT_16 1
#define LV_FONT_MONTSERRAT_18 0
#define LV_FONT_MONTSERRAT_20 1
#define LV_FONT_MONTSERRAT_22 0
#define LV_FONT_MONTSERRAT_24 1
#define LV_FONT_MONTSERRAT_26 0
#define LV_FONT_MONTSERRAT_28 1
#define LV_FONT_MONTSERRAT_30 0
#define LV_FONT_MONTSERRAT_32 1
#define LV_FONT_MONTSERRAT_34 0
#define LV_FONT_MONTSERRAT_36 0
#define LV_FONT_MONTSERRAT_38 0
#define LV_FONT_MONTSERRAT_40 0
#define LV_FONT_MONTSERRAT_42 0
#define LV_FONT_MONTSERRAT_44 0
#define LV_FONT_MONTSERRAT_46 0
#define LV_FONT_MONTSERRAT_48 1

#define LV_FONT_DEFAULT &lv_font_montserrat_14

/* ====================
   EXAMPLES AND DEMOS
   ==================== */
#define LV_BUILD_EXAMPLES 0
#define LV_USE_DEMO_WIDGETS 0
#define LV_USE_DEMO_BENCHMARK 0

/* ====================
   FILE SYSTEM
   ==================== */
#define LV_USE_FS_STDIO 0
#define LV_USE_FS_POSIX 0
#define LV_USE_FS_WIN32 0
#define LV_USE_FS_FATFS 0

/* ====================
   ANIMATION
   ==================== */
#define LV_USE_ANIMATION 0

/* ====================
   SNAPSHOTS
   ==================== */
#define LV_USE_SNAPSHOT 0

/* ====================
   MONKEY TEST
   ==================== */
#define LV_USE_MONKEY 0

/* ====================
   GRID LAYOUT
   ==================== */
#define LV_USE_GRID 1
#define LV_USE_FLEX 1

#endif /* LV_CONF_H */