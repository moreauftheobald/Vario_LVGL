#ifndef TEST_LOGGER_TASK_H
#define TEST_LOGGER_TASK_H

#include <Arduino.h>
#include "constants.h"
#include "globals.h"
#include "kalman_task.h"
#include "FS.h"
#include "SD_MMC.h"
#include "src/sd_card.h"

#ifdef TEST_MODE

static TaskHandle_t test_logger_task_handle = NULL;
static File test_log_file;
static bool test_logging_active = false;
static SemaphoreHandle_t sd_mutex = NULL;

// Variables pour calcul vario brut simplifie (derivee simple)
static float last_alt = 0.0f;
static uint32_t last_time = 0;
static bool first_sample = true;

// SUPPRIME: calculate_bno_accel_z_world() redondant avec Kalman

// Calcul vario brut simplifie (derivee discrete simple)
static float calculate_raw_vario_simple(float current_alt, uint32_t current_time) {
  if (first_sample) {
    last_alt = current_alt;
    last_time = current_time;
    first_sample = false;
    return 0.0f;
  }
  
  float dt = (current_time - last_time) / 1000.0f;
  if (dt < 0.1f) return 0.0f;
  
  float vario = (current_alt - last_alt) / dt;
  
  last_alt = current_alt;
  last_time = current_time;
  
  return vario;
}

// Creation fichier
static bool test_logger_create_file() {
  if (sd_mutex == NULL) {
    sd_mutex = xSemaphoreCreateMutex();
  }
  
  if (!xSemaphoreTake(sd_mutex, pdMS_TO_TICKS(1000))) {
    return false;
  }
  
  char filename[64];
  if (!sd_is_ready()) {
#ifdef DEBUG_MODE
    Serial.println("[TEST_LOG] SD not ready");
#endif
    xSemaphoreGive(sd_mutex);
    return false;
  }
  
  File dir = SD_MMC.open(FLIGHTS_DIR);
  if (!dir || !dir.isDirectory()) {
    SD_MMC.mkdir(FLIGHTS_DIR);
  }
  if (dir) dir.close();
  
  char file_only[40];
  snprintf(file_only, sizeof(file_only), "test_%lu.csv", millis());
  snprintf(filename, sizeof(filename), "%s/%s", FLIGHTS_DIR, file_only);
  
#ifdef DEBUG_MODE
  Serial.printf("[TEST_LOG] Creating file: %s\n", filename);
#endif
  
  test_log_file = SD_MMC.open(filename, FILE_WRITE);
  if (!test_log_file) {
#ifdef DEBUG_MODE
    Serial.printf("[TEST_LOG] Cannot create file: %s\n", filename);
#endif
    xSemaphoreGive(sd_mutex);
    return false;
  }
  
  // Header CSV
  test_log_file.print("Timestamp_ms,Date,Time,");
  test_log_file.print("Pressure_hPa,Temp_C,Pressure_Alt_m,Vario_Raw_Baro_ms,Alt_QNE_m,");
  test_log_file.print("BNO_Quat_W,BNO_Quat_X,BNO_Quat_Y,BNO_Quat_Z,");
  test_log_file.print("BNO_Accel_X_ms2,BNO_Accel_Y_ms2,BNO_Accel_Z_ms2,");
  test_log_file.print("GPS_Longitude,GPS_Latitude,GPS_Alt_m,GPS_Speed_knots,GPS_Course_deg,GPS_Satellites,GPS_FixQuality,");
  test_log_file.print("Kalman_Alt_m,Kalman_Vario_ms,Kalman_Alt_QNE_m,Kalman_Alt_QNH_m,Kalman_Alt_QFE_m,");
  test_log_file.print("Kalman_P00,Kalman_P11,Kalman_P22,");
  test_log_file.println("Valid_BMP,Valid_BNO,Valid_GPS");
  
  test_log_file.flush();
  
#ifdef DEBUG_MODE
  Serial.printf("[TEST_LOG] File created: %s\n", filename);
#endif
  
  xSemaphoreGive(sd_mutex);
  return true;
}

// Ecriture ligne OPTIMISEE
static void test_logger_write_line() {
  if (!test_log_file) return;
  
  if (!xSemaphoreTake(sd_mutex, pdMS_TO_TICKS(100))) {
    return;
  }
  
  uint32_t now = millis();
  
  // Kalman
  kalman_data_t kdata;
  kalman_get_data(&kdata);
  
  // BMP390
  float pressure_hpa = g_sensor_data.bmp390.valid ? 
                       g_sensor_data.bmp390.pressure / 100.0f : 0.0f;
  float temp_c = g_sensor_data.bmp390.valid ? 
                 g_sensor_data.bmp390.temperature : 0.0f;
  float pressure_alt = pressure_hpa > 0 ? 
                       44330.0f * (1.0f - pow(pressure_hpa / 1013.25f, 0.1903f)) : 0.0f;
  float alt_qne = pressure_alt;
  
  // Vario brut simplifie
  float vario_raw = calculate_raw_vario_simple(pressure_alt, now);
  
  // BNO080 - OPTIMISE: lecture directe sans recalcul
  float qw = g_sensor_data.bno080.valid ? g_sensor_data.bno080.quat_real : 0.0f;
  float qx = g_sensor_data.bno080.valid ? g_sensor_data.bno080.quat_i : 0.0f;
  float qy = g_sensor_data.bno080.valid ? g_sensor_data.bno080.quat_j : 0.0f;
  float qz = g_sensor_data.bno080.valid ? g_sensor_data.bno080.quat_k : 0.0f;
  float ax = g_sensor_data.bno080.valid ? g_sensor_data.bno080.accel_x : 0.0f;
  float ay = g_sensor_data.bno080.valid ? g_sensor_data.bno080.accel_y : 0.0f;
  float az = g_sensor_data.bno080.valid ? g_sensor_data.bno080.accel_z : 0.0f;
  
  // OPTIMISE: Pas de calcul az_world - on log juste les accelerations brutes
  // Le Kalman fait deja ce calcul, inutile de le refaire ici
  
  // GPS
  char date_str[16] = "00/00/0000";
  char time_str[16] = "00:00:00";
  
  if (g_sensor_data.gps.valid) {
    snprintf(date_str, sizeof(date_str), "%02d/%02d/%04d",
             g_sensor_data.gps.day,
             g_sensor_data.gps.month,
             g_sensor_data.gps.year + 2000);
    snprintf(time_str, sizeof(time_str), "%02d:%02d:%02d",
             g_sensor_data.gps.hour,
             g_sensor_data.gps.minute,
             g_sensor_data.gps.seconds);
  }
  
  float gps_lon = g_sensor_data.gps.valid ? g_sensor_data.gps.longitude : 0.0f;
  float gps_lat = g_sensor_data.gps.valid ? g_sensor_data.gps.latitude : 0.0f;
  float gps_alt = g_sensor_data.gps.valid ? g_sensor_data.gps.altitude : 0.0f;
  float gps_speed = g_sensor_data.gps.valid ? g_sensor_data.gps.speed : 0.0f;
  float gps_course = g_sensor_data.gps.valid ? g_sensor_data.gps.angle : 0.0f;
  int gps_sat = g_sensor_data.gps.valid ? g_sensor_data.gps.satellites : 0;
  int gps_fix = g_sensor_data.gps.valid ? g_sensor_data.gps.fixquality : 0;
  
  // Covariances Kalman
  extern KalmanFilter_t kf;
  float p00 = kf.P[0][0];
  float p11 = kf.P[1][1];
  float p22 = kf.P[2][2];
  
  // Ecriture CSV
  test_log_file.printf("%lu,%s,%s,", now, date_str, time_str);
  test_log_file.printf("%.2f,%.2f,%.2f,%.3f,%.2f,",
                       pressure_hpa, temp_c, pressure_alt, vario_raw, alt_qne);
  test_log_file.printf("%.4f,%.4f,%.4f,%.4f,", qw, qx, qy, qz);
  test_log_file.printf("%.4f,%.4f,%.4f,", ax, ay, az);  // OPTIMISE: 3 valeurs au lieu de 4
  test_log_file.printf("%.6f,%.6f,%.2f,%.2f,%.2f,%d,%d,",
                       gps_lon, gps_lat, gps_alt, gps_speed, gps_course, gps_sat, gps_fix);
  test_log_file.printf("%.2f,%.3f,%.2f,%.2f,%.2f,", 
                       kdata.altitude, kdata.vario, kdata.altitude_qne, 
                       kdata.altitude_qnh, kdata.altitude_qfe);
  test_log_file.printf("%.4f,%.4f,%.4f,", p00, p11, p22);
  test_log_file.printf("%d,%d,%d\n",
                       g_sensor_data.bmp390.valid ? 1 : 0,
                       g_sensor_data.bno080.valid ? 1 : 0,
                       g_sensor_data.gps.valid ? 1 : 0);
  
  xSemaphoreGive(sd_mutex);
}

// Tache logger
static void test_logger_task(void *pvParameters) {
#ifdef DEBUG_MODE
  Serial.println("[TEST_LOG] Task started");
#endif
  
  if (!test_logger_create_file()) {
#ifdef DEBUG_MODE
    Serial.println("[TEST_LOG] Failed to create file");
#endif
    vTaskDelete(NULL);
    return;
  }
  
  test_logging_active = true;
  
  TickType_t last_wake = xTaskGetTickCount();
  const TickType_t log_period = pdMS_TO_TICKS(100);  // 10Hz
  
  while (test_logging_active) {
    test_logger_write_line();
    vTaskDelayUntil(&last_wake, log_period);
  }
  
  if (test_log_file) {
    test_log_file.close();
  }
  
#ifdef DEBUG_MODE
  Serial.println("[TEST_LOG] Task stopped");
#endif
  
  vTaskDelete(NULL);
}

// Fonctions publiques
static bool test_logger_start(void) {
  if (test_logger_task_handle != NULL) {
    return false;
  }
  
  BaseType_t ret = xTaskCreatePinnedToCore(
    test_logger_task,
    "test_logger",
    4096,
    NULL,
    3,
    &test_logger_task_handle,
    0
  );
  
  return (ret == pdPASS);
}

static void test_logger_stop(void) {
  test_logging_active = false;
  
  if (test_logger_task_handle != NULL) {
    vTaskDelay(pdMS_TO_TICKS(500));
    test_logger_task_handle = NULL;
  }
}

#endif  // TEST_MODE
#endif  // TEST_LOGGER_TASK_H