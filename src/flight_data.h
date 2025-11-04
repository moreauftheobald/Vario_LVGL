#ifndef FLIGHT_DATA_H
#define FLIGHT_DATA_H

#include <Arduino.h>
#include "globals.h"
#include "kalman_task.h"
#include "params/params.h"

// Configuration tache
#define FLIGHT_DATA_STACK_SIZE 4096
#define FLIGHT_DATA_PRIORITY 2
#define FLIGHT_DATA_UPDATE_RATE_MS 1000

// Buffer circulaire pour integration vario
#define VARIO_BUFFER_MAX 30
static float vario_buffer[VARIO_BUFFER_MAX];
static int vario_buffer_index = 0;
static int vario_buffer_count = 0;

static TaskHandle_t flight_data_task_handle = NULL;
extern TerrainElevation terrain;
float terrain_alt = NAN;

// Fonction pour arrondir a 0.1
static inline float round_to_tenth(float value) {
  return roundf(value * 10.0f) / 10.0f;
}

// Calcul vario integre
static inline float calculate_integrated_vario(int period_seconds) {
  if (vario_buffer_count == 0) return 0.0f;

  int samples = (vario_buffer_count < period_seconds) ? vario_buffer_count : period_seconds;

  float sum = 0.0f;
  for (int i = 0; i < samples; i++) {
    int idx = (vario_buffer_index - 1 - i + VARIO_BUFFER_MAX) % VARIO_BUFFER_MAX;
    sum += vario_buffer[idx];
  }

  return sum / samples;
}

// Mise a jour des donnees de vol
static inline void update_flight_data(int integration_period) {
  extern sensor_raw_data_t g_sensor_data;
  extern kalman_data_t kalman_data;
  extern SemaphoreHandle_t kalman_mutex;
  extern flight_data_t g_flight_data;
  extern SemaphoreHandle_t flight_data_mutex;

  if (!kalman_mutex || !flight_data_mutex) return;

  // Recuperer donnees Kalman
  float alt_qne = 0, alt_qnh = 0, alt_qfe = 0, vario = 0;
  bool kalman_valid = false;

  if (xSemaphoreTake(kalman_mutex, pdMS_TO_TICKS(5))) {
    alt_qne = kalman_data.altitude_qne;
    alt_qnh = kalman_data.altitude_qnh;
    alt_qfe = kalman_data.altitude_qfe;
    vario = kalman_data.vario;
    kalman_valid = kalman_data.valid;
    xSemaphoreGive(kalman_mutex);
  }

  // Recuperer donnees GPS
  float gps_alt = 0, gps_speed = 0;
  bool gps_valid = false;

  if (g_sensor_data.gps.valid && g_sensor_data.gps.fix) {
    gps_alt = g_sensor_data.gps.altitude;
    gps_speed = g_sensor_data.gps.speed * 1.852f;  // noeuds vers km/h
    gps_valid = true;
  }

  // Arrondir vario brut
  float vario_rounded = round_to_tenth(vario);

  // Ajouter au buffer circulaire
  vario_buffer[vario_buffer_index] = vario_rounded;
  vario_buffer_index = (vario_buffer_index + 1) % VARIO_BUFFER_MAX;
  if (vario_buffer_count < VARIO_BUFFER_MAX) {
    vario_buffer_count++;
  }

  // Calculer vario integre
  float vario_int = calculate_integrated_vario(integration_period);


#ifdef FLIGHT_TEST_MODE
  terrain_alt = terrain.getElevation(TEST_LAT, TEST_LON);
#else
  if (g_sensor_data.gps.valid && g_sensor_data.gps.fix) {
    terrain_alt = terrain.getElevation(
      g_sensor_data.gps.latitude,
      g_sensor_data.gps.longitude);
  }
#endif

  float agl = NAN;
  if (!isnan(terrain_alt)) {
    agl = alt_qnh - terrain_alt;
  }

  // Mise a jour structure
  if (xSemaphoreTake(flight_data_mutex, pdMS_TO_TICKS(5))) {
    g_flight_data.altitude_qne = alt_qne;
    g_flight_data.altitude_qnh = alt_qnh;
    g_flight_data.altitude_qfe = alt_qfe;
    g_flight_data.altitude_gps = gps_alt;
    g_flight_data.altitude_agl = agl;
    g_flight_data.vario_raw = vario_rounded;
    g_flight_data.vario_integrated = round_to_tenth(vario_int);
    g_flight_data.speed_gps = gps_speed;
    g_flight_data.valid = kalman_valid;  // Seulement Kalman requis
    g_flight_data.timestamp = millis();
    xSemaphoreGive(flight_data_mutex);
  }
}

// Tache FreeRTOS
static void flight_data_task(void *parameter) {
  TickType_t last_wake_time = xTaskGetTickCount();

#ifdef DEBUG_MODE
  Serial.println("[FLIGHT_DATA] Task started");
#endif

  while (true) {
    int integration_period = params.vario_integration_period;
    update_flight_data(integration_period);

#ifdef DEBUG_MODE
    static uint32_t last_debug = 0;
    uint32_t now = millis();
    if (now - last_debug >= 5000) {
      extern flight_data_t g_flight_data;
      extern SemaphoreHandle_t flight_data_mutex;

      if (xSemaphoreTake(flight_data_mutex, pdMS_TO_TICKS(5))) {
        Serial.printf("[FLIGHT_DATA] QNE:%.1fm QNH:%.1fm QFE:%.1fm GPS:%.1fm | Vario:%.1fm/s Int:%.1fm/s | Speed:%.1fkm/h | Valid:%d\n",
                      g_flight_data.altitude_qne,
                      g_flight_data.altitude_qnh,
                      g_flight_data.altitude_qfe,
                      g_flight_data.altitude_gps,
                      g_flight_data.vario_raw,
                      g_flight_data.vario_integrated,
                      g_flight_data.speed_gps,
                      g_flight_data.valid);
        xSemaphoreGive(flight_data_mutex);
      }

      last_debug = now;
    }
#endif

    vTaskDelayUntil(&last_wake_time, pdMS_TO_TICKS(FLIGHT_DATA_UPDATE_RATE_MS));
  }
}

// Demarrage tache
static inline void flight_data_task_start() {
  extern SemaphoreHandle_t flight_data_mutex;

  if (flight_data_mutex == NULL) {
    flight_data_mutex = xSemaphoreCreateMutex();
  }

  memset(vario_buffer, 0, sizeof(vario_buffer));
  vario_buffer_index = 0;
  vario_buffer_count = 0;

  xTaskCreate(
    flight_data_task,
    "FlightData",
    FLIGHT_DATA_STACK_SIZE,
    NULL,
    FLIGHT_DATA_PRIORITY,
    &flight_data_task_handle);

#ifdef DEBUG_MODE
  Serial.println("[FLIGHT_DATA] Task created");
#endif
}

#endif  // FLIGHT_DATA_H