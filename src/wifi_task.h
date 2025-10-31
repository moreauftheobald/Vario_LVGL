#ifndef WIFI_TASK_H
#define WIFI_TASK_H

#include <WiFi.h>
#include "esp_heap_caps.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "src/params/params.h"
#include "constants.h"

// Variables globales
static TaskHandle_t wifi_task_handle = NULL;
static EventGroupHandle_t wifi_event_group = NULL;
static bool wifi_is_connected = false;

// Strings allouees en PSRAM
static char* wifi_current_ssid = NULL;
static char* wifi_current_ip = NULL;

// ============================================================================
// GETTERS POUR L'UI ET AUTRES TACHES
// ============================================================================

bool wifi_get_connected_status(void) {
  return wifi_is_connected;
}

const char* wifi_get_current_ssid(void) {
  return wifi_current_ssid ? wifi_current_ssid : "";
}

const char* wifi_get_current_ip(void) {
  return wifi_current_ip ? wifi_current_ip : "";
}

/**
 * @brief Obtient le handle de l'event group WiFi pour attente externe
 * @return Handle de l'event group WiFi (peut être NULL si pas initialisé)
 */
EventGroupHandle_t wifi_get_event_group(void) {
  return wifi_event_group;
}

/**
 * @brief Attend que le WiFi soit connecté (bloquant avec timeout)
 * @param timeout_ms Timeout en millisecondes (0 = infini)
 * @return true si connecté, false si timeout
 */
bool wifi_wait_connected(uint32_t timeout_ms) {
  if (!wifi_event_group) {
    return wifi_is_connected;
  }
  
  if (wifi_is_connected) {
    return true;
  }
  
  TickType_t timeout_ticks = (timeout_ms == 0) ? portMAX_DELAY : pdMS_TO_TICKS(timeout_ms);
  
  EventBits_t bits = xEventGroupWaitBits(
    wifi_event_group,
    WIFI_CONNECTED_BIT,
    pdFALSE,  // Ne pas effacer le bit
    pdFALSE,  // Attendre un seul bit
    timeout_ticks
  );
  
  return (bits & WIFI_CONNECTED_BIT) != 0;
}

// ============================================================================
// FONCTION DE CONNEXION WIFI
// ============================================================================

static bool wifi_connect_with_priority(void) {
#ifdef DEBUG_MODE
  Serial.printf("Free PSRAM before WiFi init: %u bytes\n", ESP.getFreePsram());
#endif

  for (int priority = 0; priority < 4; priority++) {
    const char* ssid = psram_str_get(params.wifi_ssid[priority]);
    const char* password = psram_str_get(params.wifi_password[priority]);
    
    if (ssid[0] == '\0') {
      continue;
    }

#ifdef DEBUG_MODE
    Serial.printf("Trying WiFi priority %d: %s\n", priority + 1, ssid);
    Serial.printf("Password length: %d\n", strlen(password));
#endif

    // CRITIQUE: Disconnect complet avant chaque essai
    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);
    vTaskDelay(pdMS_TO_TICKS(500));
    
    WiFi.mode(WIFI_STA);
    WiFi.setAutoReconnect(false);
    vTaskDelay(pdMS_TO_TICKS(100));

    WiFi.begin(ssid, password);

    int retry = 0;
    while (WiFi.status() != WL_CONNECTED && retry < 20) {
      vTaskDelay(pdMS_TO_TICKS(500));
      retry++;
    }

    if (WiFi.status() == WL_CONNECTED) {
      if (wifi_current_ssid) {
        heap_caps_free(wifi_current_ssid);
      }
      if (wifi_current_ip) {
        heap_caps_free(wifi_current_ip);
      }
      
      wifi_current_ssid = psram_strdup(ssid);
      String ip_str = WiFi.localIP().toString();
      wifi_current_ip = psram_strdup(ip_str.c_str());
      wifi_is_connected = true;

#ifdef DEBUG_MODE
      Serial.printf("WiFi connected to: %s\n", wifi_current_ssid);
      Serial.printf("IP address: %s\n", wifi_current_ip);
#endif

      xEventGroupSetBits(wifi_event_group, WIFI_CONNECTED_BIT);
      return true;
    }

#ifdef DEBUG_MODE
    Serial.printf("Failed to connect to priority %d\n", priority + 1);
#endif
  }

#ifdef DEBUG_MODE
  Serial.println("No WiFi network available");
#endif
  return false;
}

// ============================================================================
// TACHE WIFI
// ============================================================================

static void wifi_task(void *pvParameters) {
#ifdef DEBUG_MODE
  Serial.println("WiFi task started");
#endif

  WiFi.mode(WIFI_OFF);

  while (1) {
    EventBits_t bits = xEventGroupWaitBits(
      wifi_event_group,
      WIFI_START_BIT | WIFI_STOP_BIT,
      pdTRUE,
      pdFALSE,
      portMAX_DELAY
    );

    if (bits & WIFI_START_BIT) {
#ifdef DEBUG_MODE
      Serial.println("Starting WiFi connection...");
#endif
      
      if (wifi_connect_with_priority()) {
        // Connexion reussie
        while (WiFi.status() == WL_CONNECTED) {
          vTaskDelay(pdMS_TO_TICKS(5000));
        }
        
        // Deconnecte
        wifi_is_connected = false;
        xEventGroupClearBits(wifi_event_group, WIFI_CONNECTED_BIT);
        xEventGroupSetBits(wifi_event_group, WIFI_DISCONNECTED_BIT);
        
#ifdef DEBUG_MODE
        Serial.println("WiFi disconnected");
#endif
      }
    }

    if (bits & WIFI_STOP_BIT) {
#ifdef DEBUG_MODE
      Serial.println("Stopping WiFi...");
#endif
      WiFi.disconnect(true);
      WiFi.mode(WIFI_OFF);
      wifi_is_connected = false;
      
      // Effacer bit connected, set bit disconnected
      xEventGroupClearBits(wifi_event_group, WIFI_CONNECTED_BIT);
      xEventGroupSetBits(wifi_event_group, WIFI_DISCONNECTED_BIT);
      
      // Liberer la memoire
      if (wifi_current_ssid) {
        heap_caps_free(wifi_current_ssid);
        wifi_current_ssid = NULL;
      }
      if (wifi_current_ip) {
        heap_caps_free(wifi_current_ip);
        wifi_current_ip = NULL;
      }
      
#ifdef DEBUG_MODE
      Serial.println("WiFi stopped");
#endif
    }

    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}

// ============================================================================
// FONCTIONS PUBLIQUES
// ============================================================================

static void wifi_task_start(void) {
  // Vérifier si déjà démarré
  if (wifi_task_handle != NULL) {
#ifdef DEBUG_MODE
    Serial.println("WiFi task already running");
#endif
    return;
  }

  if (wifi_event_group == NULL) {
    wifi_event_group = xEventGroupCreate();
  }

  xTaskCreatePinnedToCore(
    wifi_task,
    "wifi_task",
    4096,
    NULL,
    5,
    &wifi_task_handle,
    0
  );

  xEventGroupSetBits(wifi_event_group, WIFI_START_BIT);

#ifdef DEBUG_MODE
  Serial.println("WiFi task started");
#endif
}

static void wifi_task_stop(void) {
  if (wifi_event_group) {
    xEventGroupSetBits(wifi_event_group, WIFI_STOP_BIT);
  }

#ifdef DEBUG_MODE
  Serial.println("WiFi task stop requested");
#endif
}

#endif