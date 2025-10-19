#ifndef FILE_SERVER_TASK_H
#define FILE_SERVER_TASK_H

#include <WiFi.h>
#include <WebServer.h>
#include <SD.h>
#include <FS.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static TaskHandle_t file_server_task_handle = NULL;
static WebServer *web_server = NULL;
static bool server_is_running = false;

// Handler pour la page d'accueil
static void handle_root() {
  String html = "<!DOCTYPE html><html><head>";
  html += "<meta charset='UTF-8'>";
  html += "<title>Vario File Server</title>";
  html += "<style>";
  html += "body{font-family:Arial;background:#1a1a2e;color:#eee;padding:20px}";
  html += "h1{color:#00d4ff}";
  html += "a{color:#00d4ff;text-decoration:none;padding:5px 10px;";
  html += "background:#2a2a4e;border-radius:5px;display:inline-block;margin:5px}";
  html += "a:hover{background:#3a3a6e}";
  html += ".file{margin:10px 0}";
  html += ".size{color:#888;font-size:0.9em;margin-left:10px}";
  html += "</style></head><body>";
  html += "<h1>Vario File Server</h1>";
  html += "<h2>Flight Files</h2>";
  
  File root = SD.open("/flights");
  if (!root) {
    html += "<p>No flights directory found</p>";
  } else {
    File file = root.openNextFile();
    bool has_files = false;
    
    while (file) {
      if (!file.isDirectory()) {
        has_files = true;
        html += "<div class='file'>";
        html += "<a href='/download?file=" + String(file.name()) + "'>";
        html += String(file.name());
        html += "</a>";
        html += "<span class='size'>";
        html += String(file.size() / 1024) + " KB";
        html += "</span>";
        html += "</div>";
      }
      file.close();
      file = root.openNextFile();
    }
    
    if (!has_files) {
      html += "<p>No flight files available</p>";
    }
    
    root.close();
  }
  
  html += "</body></html>";
  
  web_server->send(200, "text/html", html);
}

// Handler pour le telechargement de fichiers
static void handle_download() {
  if (!web_server->hasArg("file")) {
    web_server->send(400, "text/plain", "Missing file parameter");
    return;
  }
  
  String filename = web_server->arg("file");
  String filepath = "/flights/" + filename;
  
  if (!SD.exists(filepath)) {
    web_server->send(404, "text/plain", "File not found");
    return;
  }
  
  File file = SD.open(filepath, FILE_READ);
  if (!file) {
    web_server->send(500, "text/plain", "Failed to open file");
    return;
  }
  
#ifdef DEBUG_MODE
  Serial.printf("Sending file: %s (%d bytes)\n", filename.c_str(), file.size());
#endif
  
  web_server->streamFile(file, "application/octet-stream");
  file.close();
}

// Handler 404
static void handle_not_found() {
  web_server->send(404, "text/plain", "404 Not Found");
}

// Tache serveur de fichiers
static void file_server_task(void *pvParameters) {
#ifdef DEBUG_MODE
  Serial.println("File server task started");
#endif
  
  web_server = new WebServer(80);
  
  web_server->on("/", handle_root);
  web_server->on("/download", handle_download);
  web_server->onNotFound(handle_not_found);
  
  web_server->begin();
  server_is_running = true;
  
#ifdef DEBUG_MODE
  Serial.println("HTTP server started on port 80");
#endif
  
  while (1) {
    web_server->handleClient();
    vTaskDelay(pdMS_TO_TICKS(10));
  }
}

// Fonctions publiques
bool file_server_is_running(void) {
  return server_is_running;
}

void file_server_start(void) {
  if (file_server_task_handle != NULL) {
    return; // Deja lance
  }
  
  xTaskCreatePinnedToCore(
    file_server_task,
    "file_server",
    4096,
    NULL,
    3,
    &file_server_task_handle,
    0
  );
  
#ifdef DEBUG_MODE
  Serial.println("File server task created");
#endif
}

void file_server_stop(void) {
  if (file_server_task_handle != NULL) {
    if (web_server) {
      web_server->stop();
      delete web_server;
      web_server = NULL;
    }
    
    vTaskDelete(file_server_task_handle);
    file_server_task_handle = NULL;
    server_is_running = false;
    
#ifdef DEBUG_MODE
    Serial.println("File server stopped");
#endif
  }
}

#endif