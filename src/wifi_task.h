#ifndef WIFI_TASK_H
#define WIFI_TASK_H

#include <WiFi.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "constants.h"
#include "src/params/params.h"
#include "globals.h"

// ============================================================================
// VARIABLES GLOBALES
// ============================================================================
static TaskHandle_t wifi_task_handle = NULL;
static EventGroupHandle_t wifi_event_group = NULL;
static bool wifi_is_connected = false;
static char *wifi_current_ssid = NULL;
static char *wifi_current_ip = NULL;

// ============================================================================
// FONCTIONS PUBLIQUES GETTER
// ============================================================================
static bool wifi_get_connected_status(void) {
  return wifi_is_connected;
}

static const char* wifi_get_current_ssid(void) {
  return wifi_current_ssid ? wifi_current_ssid : "";
}

static const char* wifi_get_current_ip(void) {
  return wifi_current_ip ? wifi_current_ip : "0.0.0.0";
}

static bool wifi_wait_connected(uint32_t timeout_ms) {
  if (!wifi_event_group) return false;
  
  TickType_t timeout_ticks = (timeout_ms == 0) ? 
    portMAX_DELAY : pdMS_TO_TICKS(timeout_ms);
  
  EventBits_t bits = xEventGroupWaitBits(
    wifi_event_group,
    WIFI_CONNECTED_BIT,
    pdFALSE,
    pdFALSE,
    timeout_ticks
  );
  
  return (bits & WIFI_CONNECTED_BIT) != 0;
}

// ============================================================================
// FONCTION DE CONNEXION WIFI OPTIMISEE
// ============================================================================
static bool wifi_connect_with_priority(void) {
#ifdef DEBUG_MODE
  Serial.printf("[WIFI] Free PSRAM: %u bytes\n", ESP.getFreePsram());
#endif

  for (int priority = 0; priority < 4; priority++) {
    const char* ssid = psram_str_get(params.wifi_ssid[priority]);
    const char* password = psram_str_get(params.wifi_password[priority]);
    
    // Ignorer entree vide
    if (ssid[0] == '\0') continue;

#ifdef DEBUG_MODE
    Serial.printf("[WIFI] Try priority %d: %s\n", priority + 1, ssid);
#endif

    // Deconnexion propre avant nouvel essai
    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);
    vTaskDelay(pdMS_TO_TICKS(500));
    
    WiFi.mode(WIFI_STA);
    WiFi.setAutoReconnect(false);
    vTaskDelay(pdMS_TO_TICKS(100));

    WiFi.begin(ssid, password);

    // Attente connexion (10 secondes max)
    int retry = 0;
    while (WiFi.status() != WL_CONNECTED && retry < 20) {
      vTaskDelay(pdMS_TO_TICKS(500));
      retry++;
    }

    if (WiFi.status() == WL_CONNECTED) {
      // Liberer anciennes allocations
      if (wifi_current_ssid) {
        heap_caps_free(wifi_current_ssid);
        wifi_current_ssid = NULL;
      }
      if (wifi_current_ip) {
        heap_caps_free(wifi_current_ip);
        wifi_current_ip = NULL;
      }
      
      // OPTIMISATION: Allocation directe sans String temporaire
      wifi_current_ssid = psram_strdup(ssid);
      
      IPAddress ip = WiFi.localIP();
      wifi_current_ip = (char*)heap_caps_malloc(16, MALLOC_CAP_SPIRAM);
      if (wifi_current_ip) {
        snprintf(wifi_current_ip, 16, "%d.%d.%d.%d", 
                 ip[0], ip[1], ip[2], ip[3]);
      }
      
      wifi_is_connected = true;

#ifdef DEBUG_MODE
      Serial.printf("[WIFI] Connected to: %s\n", wifi_current_ssid);
      Serial.printf("[WIFI] IP: %s\n", wifi_current_ip);
#endif

      xEventGroupSetBits(wifi_event_group, WIFI_CONNECTED_BIT);
      return true;
    }

#ifdef DEBUG_MODE
    Serial.printf("[WIFI] Failed priority %d\n", priority + 1);
#endif
  }

#ifdef DEBUG_MODE
  Serial.println("[WIFI] No network available");
#endif
  return false;
}

// ============================================================================
// TACHE WIFI
// ============================================================================
static void wifi_task(void *pvParameters) {
#ifdef DEBUG_MODE
  Serial.println("[WIFI] Task started");
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
      Serial.println("[WIFI] Starting connection...");
#endif
      
      if (wifi_connect_with_priority()) {
        // Connexion reussie - surveiller etat
        while (WiFi.status() == WL_CONNECTED) {
          vTaskDelay(pdMS_TO_TICKS(5000));
        }
        
        // Deconnexion detectee
        wifi_is_connected = false;
        xEventGroupClearBits(wifi_event_group, WIFI_CONNECTED_BIT);
        xEventGroupSetBits(wifi_event_group, WIFI_DISCONNECTED_BIT);
        
#ifdef DEBUG_MODE
        Serial.println("[WIFI] Connection lost");
#endif
      } else {
#ifdef DEBUG_MODE
        Serial.println("[WIFI] Connection failed");
#endif
      }
    }

    if (bits & WIFI_STOP_BIT) {
#ifdef DEBUG_MODE
      Serial.println("[WIFI] Stopping...");
#endif
      WiFi.disconnect(true);
      WiFi.mode(WIFI_OFF);
      wifi_is_connected = false;
      
      xEventGroupClearBits(wifi_event_group, WIFI_CONNECTED_BIT);
      xEventGroupSetBits(wifi_event_group, WIFI_DISCONNECTED_BIT);
      
      // Liberer memoire
      if (wifi_current_ssid) {
        heap_caps_free(wifi_current_ssid);
        wifi_current_ssid = NULL;
      }
      if (wifi_current_ip) {
        heap_caps_free(wifi_current_ip);
        wifi_current_ip = NULL;
      }
      
#ifdef DEBUG_MODE
      Serial.println("[WIFI] Stopped");
#endif
    }

    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}

// ============================================================================
// FONCTIONS PUBLIQUES CONTROLE
// ============================================================================
static void wifi_task_start(void) {
  if (wifi_task_handle != NULL) {
#ifdef DEBUG_MODE
    Serial.println("[WIFI] Task already running");
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
  Serial.println("[WIFI] Task started");
#endif
}

static void wifi_task_stop(void) {
  if (wifi_event_group) {
    xEventGroupSetBits(wifi_event_group, WIFI_STOP_BIT);
  }

#ifdef DEBUG_MODE
  Serial.println("[WIFI] Stop requested");
#endif
}

#endif