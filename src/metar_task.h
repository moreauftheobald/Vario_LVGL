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

// Structure donnees METAR
typedef struct {
  char station[5];      // Code ICAO
  float qnh;           // hPa
  float temperature;   // Celsius
  float dewpoint;      // Celsius
  int wind_dir;        // Degres
  float wind_speed;    // m/s
  int visibility;      // metres
  char conditions[32]; // Description
  uint32_t timestamp;  // millis()
  bool valid;
} metar_data_t;

// Variables globales
static metar_data_t metar_data = {0};
static SemaphoreHandle_t metar_mutex = NULL;
static TaskHandle_t metar_task_handle = NULL;
static EventGroupHandle_t metar_event_group = NULL;

// Event bits
#define METAR_FETCH_BIT BIT0
#define METAR_STOP_BIT BIT1

// API config
#define METAR_API_URL "https://aviationweather.gov/api/data/metar"
#define METAR_RADIUS_KM 100

// Stations METAR proches (Luxembourg, Metz, etc)
static const char* nearby_stations[] = {
  "ELLX",  // Luxembourg
  "LFJL",  // Metz-Nancy
  "EBLG",  // Liege
  "EDRS",  // Saarbrucken
  NULL
};

// Calcul distance entre 2 coordonnees
static float calc_distance(float lat1, float lon1, float lat2, float lon2) {
  const float R = 6371.0f; // km
  float dlat = (lat2 - lat1) * M_PI / 180.0f;
  float dlon = (lon2 - lon1) * M_PI / 180.0f;
  
  float a = sin(dlat/2) * sin(dlat/2) + 
            cos(lat1 * M_PI / 180.0f) * cos(lat2 * M_PI / 180.0f) * 
            sin(dlon/2) * sin(dlon/2);
  float c = 2 * atan2(sqrt(a), sqrt(1-a));
  
  return R * c;
}

// Parser METAR pour extraire QNH
static bool parse_metar_qnh(const String& metar, float* qnh) {
  // Chercher pattern QNH: Q1234 ou A2992
  int idx = metar.indexOf(" Q");
  if (idx > 0) {
    String qnh_str = metar.substring(idx + 2, idx + 6);
    *qnh = qnh_str.toFloat();
    return true;
  }
  
  // Format US: A2992 (inches Hg)
  idx = metar.indexOf(" A");
  if (idx > 0) {
    String alt_str = metar.substring(idx + 2, idx + 6);
    float inches = alt_str.toFloat() / 100.0f;
    *qnh = inches * 33.8639f; // Conversion en hPa
    return true;
  }
  
  return false;
}

// Recuperer METAR depuis API
static bool fetch_metar_from_api(const char* station) {
  if (!wifi_get_connected_status()) {
    return false;
  }
  
  HTTPClient http;
  String url = String(METAR_API_URL) + "?ids=" + station + "&format=raw";
  
#ifdef DEBUG_MODE
  Serial.printf("[METAR] Fetching: %s\n", url.c_str());
#endif
  
  http.begin(url);
  http.setTimeout(5000);
  
  int httpCode = http.GET();
  if (httpCode == HTTP_CODE_OK) {
    String payload = http.getString();
    
#ifdef DEBUG_MODE
    Serial.printf("[METAR] Response: %s\n", payload.c_str());
#endif
    
    float qnh = 0;
    if (parse_metar_qnh(payload, &qnh)) {
      if (xSemaphoreTake(metar_mutex, pdMS_TO_TICKS(100))) {
        strcpy(metar_data.station, station);
        metar_data.qnh = qnh;
        metar_data.timestamp = millis();
        metar_data.valid = true;
        xSemaphoreGive(metar_mutex);
        
#ifdef DEBUG_MODE
        Serial.printf("[METAR] QNH: %.1f hPa from %s\n", qnh, station);
#endif
        
        // Appliquer au filtre de Kalman
        kalman_set_qnh(qnh);
        
        http.end();
        return true;
      }
    }
  }
  
  http.end();
  return false;
}

// Chercher station la plus proche
static bool find_nearest_station() {
  // Si pas de GPS, utiliser stations par defaut
  if (!g_sensor_data.gps.valid || !g_sensor_data.gps.fix) {
#ifdef DEBUG_MODE
    Serial.println("[METAR] No GPS, trying default stations");
#endif
    
    for (int i = 0; nearby_stations[i]; i++) {
      if (fetch_metar_from_api(nearby_stations[i])) {
        return true;
      }
    }
    return false;
  }
  
  // Avec GPS, chercher la plus proche
  float lat = g_sensor_data.gps.latitude;
  float lon = g_sensor_data.gps.longitude;
  
  // Positions approximatives des stations
  struct {
    const char* code;
    float lat;
    float lon;
  } stations[] = {
    {"ELLX", 49.6233, 6.2044},   // Luxembourg
    {"LFJL", 48.9821, 6.2513},   // Metz
    {"EBLG", 50.6374, 5.4433},   // Liege
    {"EDRS", 49.2147, 7.1095},   // Saarbrucken
    {NULL, 0, 0}
  };
  
  const char* nearest = NULL;
  float min_dist = 999999;
  
  for (int i = 0; stations[i].code; i++) {
    float dist = calc_distance(lat, lon, stations[i].lat, stations[i].lon);
    if (dist < min_dist && dist < METAR_RADIUS_KM) {
      min_dist = dist;
      nearest = stations[i].code;
    }
  }
  
  if (nearest) {
#ifdef DEBUG_MODE
    Serial.printf("[METAR] Nearest station: %s (%.1f km)\n", nearest, min_dist);
#endif
    return fetch_metar_from_api(nearest);
  }
  
  return false;
}

// Tache METAR
static void metar_task(void* parameter) {
  TickType_t last_fetch = 0;
  const TickType_t fetch_interval = pdMS_TO_TICKS(1800000); // 30 min
  
#ifdef DEBUG_MODE
  Serial.println("[METAR] Task started");
#endif
  
  while (1) {
    EventBits_t bits = xEventGroupWaitBits(
      metar_event_group,
      METAR_FETCH_BIT | METAR_STOP_BIT,
      pdTRUE,
      pdFALSE,
      portMAX_DELAY
    );
    
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
        if (find_nearest_station()) {
          last_fetch = xTaskGetTickCount();
        }
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
    metar_mutex = xSemaphoreCreateMutex();
  }
  
  if (!metar_event_group) {
    metar_event_group = xEventGroupCreate();
  }
  
  if (!metar_task_handle) {
    xTaskCreate(
      metar_task,
      "metar_task",
      4096,
      NULL,
      3,
      &metar_task_handle
    );
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
  if (xSemaphoreTake(metar_mutex, pdMS_TO_TICKS(10))) {
    float qnh = metar_data.valid ? metar_data.qnh : 1013.25f;
    xSemaphoreGive(metar_mutex);
    return qnh;
  }
  return 1013.25f;
}

#endif