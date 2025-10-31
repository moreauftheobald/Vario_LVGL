#ifndef METAR_TASK_H
#define METAR_TASK_H

#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "constants.h"
#include "globals.h"
#include "src/wifi_task.h"
#include "src/kalman_task.h"

// =============================
// Données globales
// =============================
static metar_data_t metar_data = { 0 };
static SemaphoreHandle_t metar_mutex = NULL;
static TaskHandle_t metar_task_handle = NULL;
static EventGroupHandle_t metar_event_group = NULL;
static bool qnh_retrieved = false;

// =============================
// METHOD 1: Progressive smooth transition
// =============================
#if QNH_ADJUST_METHOD == 1
static void updateQNH_method1(float newQNH) {
  float startQNH = metar_data.qnh;

  if (startQNH < 900.0f || startQNH > 1100.0f) {
    startQNH = 1013.25f;
  }

  float deltaQNH = newQNH - startQNH;

  if (fabs(deltaQNH) < 0.1f) {
    if (xSemaphoreTake(metar_mutex, pdMS_TO_TICKS(100))) {
      metar_data.qnh = newQNH;
      g_sensor_data.qnh_metar = newQNH;
      xSemaphoreGive(metar_mutex);
    }
    kalman_set_qnh_ready(newQNH);
    return;
  }

#ifdef DEBUG_MODE
  Serial.printf("[QNH-M1] Smooth transition: %.1f -> %.1f hPa\n", startQNH, newQNH);
#endif

  const int STEPS = 50;
  const int DELAY_MS = 100;

  for (int i = 1; i <= STEPS; i++) {
    float currentQNH = startQNH + (deltaQNH * i / STEPS);

    if (xSemaphoreTake(metar_mutex, pdMS_TO_TICKS(100))) {
      metar_data.qnh = currentQNH;
      g_sensor_data.qnh_metar = currentQNH;
      xSemaphoreGive(metar_mutex);
    }

    kalman_set_qnh_ready(currentQNH);
    vTaskDelay(pdMS_TO_TICKS(DELAY_MS));
  }

  if (xSemaphoreTake(metar_mutex, pdMS_TO_TICKS(100))) {
    metar_data.qnh = newQNH;
    g_sensor_data.qnh_metar = newQNH;
    xSemaphoreGive(metar_mutex);
  }
  kalman_set_qnh_ready(newQNH);

#ifdef DEBUG_MODE
  Serial.printf("[QNH-M1] Transition completed: %.1f hPa\n", newQNH);
#endif
}
#endif

// =============================
// METHOD 2: Smooth transition + IMU confidence boost
// =============================
#if QNH_ADJUST_METHOD == 2
static void updateQNH_method2(float newQNH) {
  float startQNH = metar_data.qnh;

  if (startQNH < 900.0f || startQNH > 1100.0f) {
    startQNH = 1013.25f;
  }

  float deltaQNH = newQNH - startQNH;

  if (fabs(deltaQNH) < 0.1f) {
    if (xSemaphoreTake(metar_mutex, pdMS_TO_TICKS(100))) {
      metar_data.qnh = newQNH;
      g_sensor_data.qnh_metar = newQNH;
      g_sensor_data.qnh_transition = false;
      xSemaphoreGive(metar_mutex);
    }
    kalman_set_qnh_ready(newQNH);
    return;
  }

#ifdef DEBUG_MODE
  Serial.printf("[QNH-M2] Smooth transition + IMU boost: %.1f -> %.1f hPa\n", startQNH, newQNH);
#endif

  // Activer le flag de transition (pour kalman_task)
  if (xSemaphoreTake(metar_mutex, pdMS_TO_TICKS(100))) {
    g_sensor_data.qnh_transition = true;
    xSemaphoreGive(metar_mutex);
  }

  const int STEPS = 50;
  const int DELAY_MS = 100;

  for (int i = 1; i <= STEPS; i++) {
    float currentQNH = startQNH + (deltaQNH * i / STEPS);

    if (xSemaphoreTake(metar_mutex, pdMS_TO_TICKS(100))) {
      metar_data.qnh = currentQNH;
      g_sensor_data.qnh_metar = currentQNH;
      xSemaphoreGive(metar_mutex);
    }

    kalman_set_qnh_ready(currentQNH);
    vTaskDelay(pdMS_TO_TICKS(DELAY_MS));
  }

  if (xSemaphoreTake(metar_mutex, pdMS_TO_TICKS(100))) {
    metar_data.qnh = newQNH;
    g_sensor_data.qnh_metar = newQNH;
    g_sensor_data.qnh_transition = false;
    xSemaphoreGive(metar_mutex);
  }
  kalman_set_qnh_ready(newQNH);

#ifdef DEBUG_MODE
  Serial.printf("[QNH-M2] Transition completed: %.1f hPa\n", newQNH);
#endif
}
#endif

// =============================
// METHOD 3: Direct altitude offset (instant)
// =============================
#if QNH_ADJUST_METHOD == 3
static void updateQNH_method3(float newQNH) {
  float startQNH = metar_data.qnh;

  if (startQNH < 900.0f || startQNH > 1100.0f) {
    startQNH = 1013.25f;
  }

  float deltaQNH = newQNH - startQNH;

  if (fabs(deltaQNH) < 0.1f) {
    if (xSemaphoreTake(metar_mutex, pdMS_TO_TICKS(100))) {
      metar_data.qnh = newQNH;
      g_sensor_data.qnh_metar = newQNH;
      xSemaphoreGive(metar_mutex);
    }
    kalman_set_qnh_ready(newQNH);
    return;
  }

#ifdef DEBUG_MODE
  Serial.printf("[QNH-M3] Direct offset: %.1f -> %.1f hPa (delta: %.1f hPa)\n",
                startQNH, newQNH, deltaQNH);
#endif

  // Calculer offset d'altitude (approximation: ~8.3m par hPa à altitude moyenne)
  float alt_offset = 8.3f * deltaQNH;

#ifdef DEBUG_MODE
  Serial.printf("[QNH-M3] Applying altitude offset: %.1f m\n", alt_offset);
#endif

  // Appliquer l'offset directement au Kalman
  kalman_apply_altitude_offset(alt_offset);

  // Mettre à jour le QNH instantanément
  if (xSemaphoreTake(metar_mutex, pdMS_TO_TICKS(100))) {
    metar_data.qnh = newQNH;
    g_sensor_data.qnh_metar = newQNH;
    xSemaphoreGive(metar_mutex);
  }
  kalman_set_qnh_ready(newQNH);

#ifdef DEBUG_MODE
  Serial.printf("[QNH-M3] QNH updated instantly to %.1f hPa\n", newQNH);
#endif
}
#endif

// =============================
// Fonction wrapper qui appelle la bonne méthode
// =============================
static void updateQNH_smooth(float newQNH) {
#if QNH_ADJUST_METHOD == 1
  updateQNH_method1(newQNH);
#elif QNH_ADJUST_METHOD == 2
  updateQNH_method2(newQNH);
#elif QNH_ADJUST_METHOD == 3
  updateQNH_method3(newQNH);
#else
#error "QNH_ADJUST_METHOD must be 1, 2, or 3"
#endif
}

// =============================
// Fonction utilitaire API Open-Meteo
// =============================
static bool fetch_qnh_openmeteo(float lat, float lon) {
  if (!wifi_get_connected_status()) return false;

  char url[256];
  snprintf(url, sizeof(url),
           "https://api.open-meteo.com/v1/forecast?latitude=%.6f&longitude=%.6f&current=pressure_msl",
           lat, lon);

#ifdef DEBUG_MODE
  Serial.printf("[QNH] Fetching from Open-Meteo:\n%s\n", url);
#endif

  HTTPClient http;
  http.begin(url);
  http.setTimeout(10000);
  int code = http.GET();

  if (code != HTTP_CODE_OK) {
#ifdef DEBUG_MODE
    Serial.printf("[QNH] HTTP error: %d\n", code);
#endif
    http.end();
    return false;
  }

  String payload = http.getString();
  http.end();

  DynamicJsonDocument doc(2048);
  DeserializationError err = deserializeJson(doc, payload);

  if (err) {
#ifdef DEBUG_MODE
    Serial.printf("[QNH] JSON error: %s\n", err.c_str());
#endif
    return false;
  }

  if (!doc.containsKey("current") || !doc["current"].containsKey("pressure_msl")) {
#ifdef DEBUG_MODE
    Serial.println("[QNH] Missing pressure_msl field");
#endif
    return false;
  }

  float qnh = doc["current"]["pressure_msl"];

#ifdef DEBUG_MODE
  Serial.printf("[QNH] Retrieved pressure_msl = %.1f hPa\n", qnh);
#endif

  // Sauvegarde du nom de station thread-safe
  if (xSemaphoreTake(metar_mutex, pdMS_TO_TICKS(100))) {
    strcpy(metar_data.station, "OpenMeteo");
    metar_data.timestamp = millis();
    metar_data.valid = true;
    xSemaphoreGive(metar_mutex);
  }

  // Appliquer la méthode d'ajustement sélectionnée
  updateQNH_smooth(qnh);

  qnh_retrieved = true;

  return true;
}

// Fonction calcul distance (formule Haversine)
static float calculate_distance(float lat1, float lon1, float lat2, float lon2) {
  const float R = 6371.0;

  float dlat = (lat2 - lat1) * M_PI / 180.0;
  float dlon = (lon2 - lon1) * M_PI / 180.0;

  float a = sin(dlat / 2) * sin(dlat / 2) + cos(lat1 * M_PI / 180.0) * cos(lat2 * M_PI / 180.0) * sin(dlon / 2) * sin(dlon / 2);

  float c = 2 * atan2(sqrt(a), sqrt(1 - a));
  return R * c;
}

static void metar_task(void* parameter) {
#ifdef DEBUG_MODE
  Serial.println("[QNH] Task started");
#endif

  const TickType_t fetch_interval = pdMS_TO_TICKS(3600000);
  const TickType_t qnh_timeout = pdMS_TO_TICKS(30000);
  const float distance_threshold = 30.0;

  TickType_t last_fetch = 0;
  TickType_t start_time = xTaskGetTickCount();
  bool timeout_applied = false;

  float last_qnh_lat = 0.0;
  float last_qnh_lon = 0.0;
  bool first_qnh_update = true;

  while (1) {
    if (!qnh_retrieved && !timeout_applied) {
      if ((xTaskGetTickCount() - start_time) >= qnh_timeout) {
#ifdef DEBUG_MODE
        Serial.println("[QNH] Timeout - forcing standard QNH (1013.25 hPa)");
#endif
        kalman_force_start_standard_qnh();
        timeout_applied = true;
        qnh_retrieved = true;
      }
    }

    bool auto_update_needed = false;

    if (qnh_retrieved && !first_qnh_update) {
      if ((xTaskGetTickCount() - last_fetch) > fetch_interval) {
        auto_update_needed = true;
#ifdef DEBUG_MODE
        Serial.println("[QNH] Auto-update: 1 hour elapsed");
#endif
      }

#ifdef FLIGHT_TEST_MODE
#else
      if (!auto_update_needed && g_sensor_data.gps.valid && g_sensor_data.gps.fix) {
        float distance = calculate_distance(
          last_qnh_lat, last_qnh_lon,
          g_sensor_data.gps.latitude, g_sensor_data.gps.longitude);

        if (distance >= distance_threshold) {
          auto_update_needed = true;
#ifdef DEBUG_MODE
          Serial.printf("[QNH] Auto-update: %.1f km traveled\n", distance);
#endif
        }
      }
#endif
    }

    if (auto_update_needed && wifi_get_connected_status()) {
      xEventGroupSetBits(metar_event_group, METAR_FETCH_BIT);
    }

    EventBits_t bits = xEventGroupWaitBits(
      metar_event_group,
      METAR_FETCH_BIT | METAR_STOP_BIT,
      pdTRUE,
      pdFALSE,
      pdMS_TO_TICKS(60000));

    if (bits & METAR_STOP_BIT) break;

    if (bits & METAR_FETCH_BIT) {
      // Attendre connexion WiFi avec timeout 30s (bloquant sans CPU)
      if (!wifi_wait_connected(30000)) {
#ifdef DEBUG_MODE
        Serial.println("[QNH] WiFi not available after 30s timeout");
#endif
        continue;
      }

#ifdef DEBUG_MODE
      Serial.println("[QNH] WiFi connected, preparing to fetch...");
#endif

#ifdef FLIGHT_TEST_MODE
      float lat = TEST_LAT;
      float lon = TEST_LON;
#else
      if (!g_sensor_data.gps.valid || !g_sensor_data.gps.fix) {
#ifdef DEBUG_MODE
        Serial.println("[QNH] Waiting for GPS fix...");
#endif
        continue;
      }
      float lat = g_sensor_data.gps.latitude;
      float lon = g_sensor_data.gps.longitude;
#endif

      if (fetch_qnh_openmeteo(lat, lon)) {
        last_fetch = xTaskGetTickCount();
        last_qnh_lat = lat;
        last_qnh_lon = lon;
        first_qnh_update = false;

#ifdef DEBUG_MODE
        Serial.printf("[QNH] Successfully fetched at position: %.6f, %.6f\n", lat, lon);
#endif
      }
    }
  }

  vTaskDelete(NULL);
}

// =============================
// Fonctions publiques
// =============================
void metar_start(void) {
  if (!metar_mutex) metar_mutex = xSemaphoreCreateMutex();
  if (!metar_event_group) metar_event_group = xEventGroupCreate();

  if (!metar_task_handle) {
    xTaskCreatePinnedToCore(
      metar_task,
      "metar_task",
      4608,
      NULL,
      3,
      &metar_task_handle,
      0);
  }

#ifdef DEBUG_MODE
  Serial.println("[QNH] Task initialized");
#endif
}

void metar_fetch(void) {
  if (metar_event_group)
    xEventGroupSetBits(metar_event_group, METAR_FETCH_BIT);
}

void metar_stop(void) {
  if (metar_event_group)
    xEventGroupSetBits(metar_event_group, METAR_STOP_BIT);
}

bool metar_get_data(metar_data_t* out) {
  if (!metar_mutex) return false;

  if (xSemaphoreTake(metar_mutex, pdMS_TO_TICKS(50))) {
    memcpy(out, &metar_data, sizeof(metar_data_t));
    xSemaphoreGive(metar_mutex);
    return metar_data.valid;
  }
  return false;
}

float metar_get_qnh(void) {
  if (!metar_mutex) return 1013.25f;

  if (xSemaphoreTake(metar_mutex, pdMS_TO_TICKS(100))) {
    float qnh = metar_data.valid ? metar_data.qnh : 1013.25f;
    xSemaphoreGive(metar_mutex);
    return qnh;
  }
  return 1013.25f;
}

#endif