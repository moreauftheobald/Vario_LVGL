#ifndef GLOBALS_H
#define GLOBALS_H

#include <Preferences.h>
#include "lvgl.h"

// Variables globales partagees entre tous les ecrans
/*extern Preferences prefs;
extern lv_obj_t *ta_active;
extern lv_obj_t *keyboard;
extern lv_obj_t *main_screen;  // Ecran principal reutilisable*/

// Structure pour donnees brutes BMP390
typedef struct {
  float temperature;      // Celsius
  float pressure;         // Pascals
  uint32_t timestamp;     // millis()
  bool valid;
} bmp390_data_t;

// Structure pour donnees brutes BNO080
typedef struct {
  float quat_i;           // Quaternion i
  float quat_j;           // Quaternion j
  float quat_k;           // Quaternion k
  float quat_real;        // Quaternion real
  float accel_x;          // Acceleration X (m/s²)
  float accel_y;          // Acceleration Y (m/s²)
  float accel_z;          // Acceleration Z (m/s²)
  float gyro_x;           // Gyroscope X (rad/s)
  float gyro_y;           // Gyroscope Y (rad/s)
  float gyro_z;           // Gyroscope Z (rad/s)
  uint32_t timestamp;     // millis()
  bool valid;
} bno080_data_t;

// Structure pour donnees brutes GPS
typedef struct {
  char lastline[120];      // Derniere trame NMEA complete
  bool fix;                // Fix GPS valide
  uint8_t fixquality;      // Qualite du fix (0-2)
  uint8_t satellites;      // Nombre de satellites
  float latitude;          // Latitude degres decimaux
  float longitude;         // Longitude degres decimaux
  float altitude;          // Altitude metres
  float speed;             // Vitesse noeuds
  float angle;             // Cap degres
  uint8_t hour;            // Heure UTC
  uint8_t minute;          // Minute UTC
  uint8_t seconds;         // Seconde UTC
  uint8_t year;            // Annee
  uint8_t month;           // Mois
  uint8_t day;             // Jour
  uint32_t timestamp;      // millis()
  bool valid;
} gps_data_t;

// Structure globale accessible par toutes les taches
typedef struct {
  bmp390_data_t bmp390;
  bno080_data_t bno080;
  gps_data_t gps;          // Ajouter cette ligne
} sensor_raw_data_t;

// Definition des variables globales
Preferences prefs;
lv_obj_t *ta_active = NULL;
lv_obj_t *keyboard = NULL;
lv_obj_t *main_screen = NULL;
sensor_raw_data_t g_sensor_data = {0};


// Instance globale
//extern sensor_raw_data_t g_sensor_data;

void force_full_refresh(void) {
  lv_obj_invalidate(lv_screen_active());
  lv_refr_now(NULL);
  vTaskDelay(pdMS_TO_TICKS(10));
}

#ifdef DEBUG_MODE
void print_all_tasks_stack_usage() {
  TaskStatus_t *pxTaskStatusArray;
  volatile UBaseType_t uxArraySize, x;
  uint32_t ulTotalRunTime;

  uxArraySize = uxTaskGetNumberOfTasks();
  pxTaskStatusArray = (TaskStatus_t *)malloc(uxArraySize * sizeof(TaskStatus_t));

  if (pxTaskStatusArray != NULL) {
    uxArraySize = uxTaskGetSystemState(pxTaskStatusArray, uxArraySize, &ulTotalRunTime);

    Serial.println("=== Task Stack HighWaterMarks ===");
    for (x = 0; x < uxArraySize; x++) {
      Serial.printf(
        "%-20s  Stack min free: %5u bytes | Priority: %2u | State: %d\n",
        pxTaskStatusArray[x].pcTaskName,
        pxTaskStatusArray[x].usStackHighWaterMark * 4,
        pxTaskStatusArray[x].uxCurrentPriority,
        pxTaskStatusArray[x].eCurrentState);
    }
    Serial.println("==============================");
    free(pxTaskStatusArray);
  }
}
#endif

#endif