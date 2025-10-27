#ifndef METAR_TASK_H
#define METAR_TASK_H

#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <math.h>
#include "constants.h"
#include "globals.h"
#include "src/wifi_task.h"
#include "src/kalman_task.h"

// Variables globales
static metar_data_t metar_data = { 0 };
static SemaphoreHandle_t metar_mutex = NULL;
static TaskHandle_t metar_task_handle = NULL;
static EventGroupHandle_t metar_event_group = NULL;

static float haversine_km(float lat1, float lon1, float lat2, float lon2) {
  const float R = 6371.0f;
  float dLat = radians(lat2 - lat1);
  float dLon = radians(lon2 - lon1);
  float a = sin(dLat / 2) * sin(dLat / 2) + cos(radians(lat1)) * cos(radians(lat2)) * sin(dLon / 2) * sin(dLon / 2);
  float c = 2 * atan2(sqrt(a), sqrt(1 - a));
  return R * c;
}

// Chercher METAR dans une zone geographique
static bool fetch_metar_by_bbox(float lat, float lon) {
  if (!wifi_get_connected_status()) return false;

  float min_lat = lat - METAR_BBOX_RADIUS;
  float min_lon = lon - METAR_BBOX_RADIUS;
  float max_lat = lat + METAR_BBOX_RADIUS;
  float max_lon = lon + METAR_BBOX_RADIUS;

  HTTPClient http;
  char bbox_params[128];
  snprintf(bbox_params, sizeof(bbox_params),
           "?bbox=%.2f%%2C%.2f%%2C%.2f%%2C%.2f&format=json&taf=false&hours=1.5",
           min_lat, min_lon, max_lat, max_lon);

  String url = String(METAR_API_URL) + String(bbox_params);

#ifdef DEBUG_MODE
  Serial.printf("[METAR] URL: %s\n", url.c_str());
#endif

  http.begin(url);
  http.setTimeout(10000);
  int httpCode = http.GET();

  if (httpCode != HTTP_CODE_OK) {
    http.end();
    return false;
  }

  String payload = http.getString();
  http.end();

  // --- Parser JSON ---
  JsonDocument doc;
  DeserializationError err = deserializeJson(doc, payload);
  if (err) {
#ifdef DEBUG_MODE
    Serial.printf("[METAR] JSON parse error: %s\n", err.c_str());
#endif
    return false;
  }

  JsonArray data = doc.as<JsonArray>();
  if (data.isNull() || data.size() == 0) {
#ifdef DEBUG_MODE
    Serial.println("[METAR] No METAR data in JSON");
#endif
    return false;
  }

  float sum_weight = 0.0f;
  float weighted_qnh = 0.0f;
  int count_valid = 0;

  for (JsonObject metar : doc.as<JsonArray>()) {
    if (!metar.containsKey("altim") || !metar.containsKey("lat") || !metar.containsKey("lon")) {
      Serial.println("[METAR] skipping entry: missing altim/lat/lon");
      continue;
    }

    float slat = metar["lat"];
    float slon = metar["lon"];
    float altim = metar["altim"];  // QNH en hPa
    if (altim <= 0.0f) {
#ifdef DEBUG_MODE
      Serial.println("[METAR] skipping entry: altim <= 0");
#endif
      continue;
    }

    const char* sid = "----";
    if (metar.containsKey("station_id")) {
      // safe extraction: get<const char*> returns nullptr if not string
      if (!metar["station_id"].isNull()) {
        sid = metar["station_id"];
        if (sid == nullptr) sid = "----";
      }
    }

    float qnh = altim;
    float dist = haversine_km(lat, lon, slat, slon);
    if (dist < 0.1f) dist = 0.1f;
    float w = 1.0f / (dist * dist);

    weighted_qnh += qnh * w;
    sum_weight += w;
    count_valid++;

#ifdef DEBUG_MODE
    // utiliser %s uniquement avec sid garanti non-NULL, et types corrects
    Serial.printf("[METAR] %s altim=%.2f inHg, QNH=%.1f hPa, dist=%.1f km\n",
                  sid, (double)altim, (double)qnh, (double)dist);
#endif
  }

  if (count_valid == 0 || sum_weight == 0) {
#ifdef DEBUG_MODE
    Serial.println("[METAR] No valid QNH found");
#endif
    return false;
  }

  float qnh_est = weighted_qnh / sum_weight;

  // Sauvegarde dans la structure globale
  if (xSemaphoreTake(metar_mutex, pdMS_TO_TICKS(100))) {
    strcpy(metar_data.station, "AVG");
    metar_data.qnh = qnh_est;
    metar_data.timestamp = millis();
    metar_data.valid = true;
    g_sensor_data.qnh_metar = qnh_est;
    xSemaphoreGive(metar_mutex);
  }

#ifdef DEBUG_MODE
  Serial.printf("[METAR] QNH local estimé = %.1f hPa (%d stations)\n", qnh_est, count_valid);
#endif

  kalman_set_qnh(qnh_est);
  return true;
}

// Chercher station la plus proche
static bool find_nearest_station() {
  float lat, lon;
  bool has_position = false;

#ifdef FLIGHT_TEST_MODE
  // Mode test: utiliser position fixe
  lat = TEST_LAT;
  lon = TEST_LON;
  has_position = true;

#ifdef DEBUG_MODE
  Serial.printf("[METAR] Using test position: %.6f, %.6f\n", lat, lon);
#endif

#else
  // Mode normal: attendre fix GPS
  if (!g_sensor_data.gps.valid || !g_sensor_data.gps.fix) {
#ifdef DEBUG_MODE
    Serial.println("[METAR] Waiting for GPS fix...");
#endif
    return false;
  }

  lat = g_sensor_data.gps.latitude;
  lon = g_sensor_data.gps.longitude;
  has_position = true;

#ifdef DEBUG_MODE
  Serial.printf("[METAR] Using GPS position: %.6f, %.6f\n", lat, lon);
#endif
#endif

  if (!has_position) {
    return false;
  }

  // Chercher METAR dans la zone via API
  return fetch_metar_by_bbox(lat, lon);
}

// Tache METAR
static void metar_task(void* parameter) {
  TickType_t last_fetch = 0;
  const TickType_t fetch_interval = pdMS_TO_TICKS(1800000);  // 30 min

#ifdef DEBUG_MODE
  Serial.println("[METAR] Task started");
#endif

  while (1) {
    EventBits_t bits = xEventGroupWaitBits(
      metar_event_group,
      METAR_FETCH_BIT | METAR_STOP_BIT,
      pdTRUE,
      pdFALSE,
      portMAX_DELAY);

    if (bits & METAR_STOP_BIT) {
      break;
    }

    if (bits & METAR_FETCH_BIT) {
      // Attendre connexion WiFi (max 30s)
      int retry = 0;
      while (!wifi_get_connected_status() && retry < 60) {
        vTaskDelay(pdMS_TO_TICKS(500));
        retry++;
      }

      if (wifi_get_connected_status()) {
#ifdef FLIGHT_TEST_MODE
        // En mode test, on peut fetch immediatement
        if (find_nearest_station()) {
          last_fetch = xTaskGetTickCount();
        }
#else
        // En mode normal, attendre GPS fix
        int gps_retry = 0;
        while ((!g_sensor_data.gps.valid || !g_sensor_data.gps.fix) && gps_retry < 120) {
          vTaskDelay(pdMS_TO_TICKS(500));
          gps_retry++;
        }

        if (find_nearest_station()) {
          last_fetch = xTaskGetTickCount();
        }
#endif
      }

      // Auto-refresh toutes les 30 min
      if ((xTaskGetTickCount() - last_fetch) > fetch_interval) {
        if (wifi_get_connected_status()) {
          find_nearest_station();
          last_fetch = xTaskGetTickCount();
        }
      }
    }

    vTaskDelay(pdMS_TO_TICKS(1000));
  }

  vTaskDelete(NULL);
}

// API publique
void metar_start(void) {
  if (!metar_mutex) {
    Serial.println("mutex created");
    metar_mutex = xSemaphoreCreateMutex();
  }

  if (!metar_event_group) {
    metar_event_group = xEventGroupCreate();
  }

  if (!metar_task_handle) {
    xTaskCreatePinnedToCore(
      metar_task,
      "metar_task",
      5120,
      NULL,
      3,
      &metar_task_handle,
      0);
  }
}

void metar_fetch(void) {
  if (metar_event_group) {
    xEventGroupSetBits(metar_event_group, METAR_FETCH_BIT);
  }
}

void metar_stop(void) {
  if (metar_event_group) {
    xEventGroupSetBits(metar_event_group, METAR_STOP_BIT);
  }

  if (metar_task_handle) {
    vTaskDelay(pdMS_TO_TICKS(100));
    metar_task_handle = NULL;
  }
}

bool metar_get_data(metar_data_t* out) {
  if (xSemaphoreTake(metar_mutex, pdMS_TO_TICKS(10))) {
    memcpy(out, &metar_data, sizeof(metar_data_t));
    xSemaphoreGive(metar_mutex);
    return metar_data.valid;
  }
  return false;
}

float metar_get_qnh(void) {
  if (xSemaphoreTake(metar_mutex, pdMS_TO_TICKS(100))) {
    float qnh = metar_data.valid ? metar_data.qnh : 1013.25f;
    xSemaphoreGive(metar_mutex);
    return qnh;
  }
  return 1013.25f;
}

#endif