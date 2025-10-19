#ifndef WIFI_TASK_H
#define WIFI_TASK_H

#include <WiFi.h>
#include "esp_heap_caps.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "src/params/params.h"

// Event bits
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_DISCONNECTED_BIT BIT1
#define WIFI_START_BIT BIT2
#define WIFI_STOP_BIT BIT3

// Variables globales
static TaskHandle_t wifi_task_handle = NULL;
static EventGroupHandle_t wifi_event_group = NULL;
static bool wifi_is_connected = false;
static String wifi_current_ssid = "";
static String wifi_current_ip = "";

// Getters pour l'UI
bool wifi_get_connected_status(void) {
  return wifi_is_connected;
}

String wifi_get_current_ssid(void) {
  return wifi_current_ssid;
}

String wifi_get_current_ip(void) {
  return wifi_current_ip;
}

// Fonction de connexion WiFi avec priorite
static bool wifi_connect_with_priority(void) {
#ifdef DEBUG_MODE
  Serial.printf("Free PSRAM before WiFi init: %u bytes\n", ESP.getFreePsram());
#endif

  // Essayer chaque reseau par priorite
  for (int priority = 0; priority < 4; priority++) {
    if (params.wifi_ssid[priority].length() == 0) {
      continue;
    }

#ifdef DEBUG_MODE
    Serial.printf("Trying WiFi priority %d: %s\n", priority + 1,
                  params.wifi_ssid[priority].c_str());
    Serial.printf("Connecting to SSID: '%s'\n", params.wifi_ssid[priority].c_str());
    Serial.printf("Password length: %d\n", params.wifi_password[priority].length());
    Serial.printf("Password (masked): %s\n",
                  String("*").c_str());  // Ne jamais logger le vrai mot de passe
#endif
    WiFi.begin(params.wifi_ssid[priority].c_str(),
               params.wifi_password[priority].c_str());

    // Attendre connexion (max 10 secondes)
    int retry = 0;
    while (WiFi.status() != WL_CONNECTED && retry < 20) {
      vTaskDelay(pdMS_TO_TICKS(500));
      retry++;
    }

    if (WiFi.status() == WL_CONNECTED) {
      wifi_current_ssid = params.wifi_ssid[priority];
      wifi_current_ip = WiFi.localIP().toString();
      wifi_is_connected = true;

#ifdef DEBUG_MODE
      Serial.printf("WiFi connected to: %s\n", wifi_current_ssid.c_str());
      Serial.printf("IP address: %s\n", wifi_current_ip.c_str());
      Serial.printf("Free PSRAM after WiFi connect: %u bytes\n", ESP.getFreePsram());
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

  // Remettre PSRAM comme prioritaire meme en cas d'echec

  return false;
}

// Tache WiFi
static void wifi_task(void *pvParameters) {
#ifdef DEBUG_MODE
  Serial.println("WiFi task started");
#endif

  WiFi.mode(WIFI_STA);
  WiFi.setAutoReconnect(true);

  while (1) {
    EventBits_t bits = xEventGroupWaitBits(
      wifi_event_group,
      WIFI_START_BIT | WIFI_STOP_BIT,
      pdTRUE,
      pdFALSE,
      portMAX_DELAY);

    if (bits & WIFI_START_BIT) {
#ifdef DEBUG_MODE
      Serial.println("WiFi start requested");
#endif

      if (!wifi_is_connected) {
        wifi_connect_with_priority();
      }
    }

    if (bits & WIFI_STOP_BIT) {
#ifdef DEBUG_MODE
      Serial.println("WiFi stop requested");
#endif

      if (wifi_is_connected) {
        WiFi.disconnect(true);
        wifi_is_connected = false;
        wifi_current_ssid = "";
        wifi_current_ip = "";
        xEventGroupSetBits(wifi_event_group, WIFI_DISCONNECTED_BIT);
      }
    }

    // Verifier periodiquement la connexion
    vTaskDelay(pdMS_TO_TICKS(5000));

    if (wifi_is_connected && WiFi.status() != WL_CONNECTED) {
#ifdef DEBUG_MODE
      Serial.println("WiFi connection lost, trying to reconnect...");
#endif
      wifi_is_connected = false;
      xEventGroupSetBits(wifi_event_group, WIFI_DISCONNECTED_BIT);
      wifi_connect_with_priority();
    }
  }
}

// Fonctions publiques
void wifi_task_start(void) {
  if (wifi_event_group == NULL) {
    wifi_event_group = xEventGroupCreate();
  }

  xEventGroupSetBits(wifi_event_group, WIFI_START_BIT);
}

void wifi_task_stop(void) {
  xEventGroupSetBits(wifi_event_group, WIFI_STOP_BIT);
}

void wifi_task_init(void) {
  if (wifi_task_handle != NULL) {
    return;
  }

  wifi_event_group = xEventGroupCreate();

  xTaskCreate(
    wifi_task,
    "wifi_task",
    2560,
    NULL,
    5,
    &wifi_task_handle);

#ifdef DEBUG_MODE
  Serial.println("WiFi task initialized");
#endif
}

#endif