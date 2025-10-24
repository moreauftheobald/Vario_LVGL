#ifndef FILE_SERVER_TASK_H
#define FILE_SERVER_TASK_H

#include <WiFi.h>
#include <WebServer.h>
#include <SD_MMC.h>  // CHANGÉ: SD_MMC au lieu de SD
#include <FS.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static TaskHandle_t file_server_task_handle = NULL;
static WebServer *web_server = NULL;
static bool server_is_running = false;

// Handler pour la page d'accueil
static void handle_root() {
  static const char PAGE_HEADER[] PROGMEM = R"rawliteral(
<!DOCTYPE html><html><head>
<meta charset="UTF-8">
<title>Vario File Server</title>
<style>
body{font-family:Arial;background:#1a1a2e;color:#eee;padding:20px}
h1{color:#00d4ff}
a{color:#00d4ff;text-decoration:none;padding:5px 10px;background:#2a2a4e;border-radius:5px;display:inline-block;margin:5px}
a:hover{background:#3a3a6e}
.file{margin:10px 0}
.size{color:#888;font-size:0.9em;margin-left:10px}
</style></head><body>
<h1>Vario File Server</h1>
<h2>Flight Files</h2>
)rawliteral";

  static const char PAGE_FOOTER[] PROGMEM = R"rawliteral(</body></html>)rawliteral";
  
  if(!web_server) {
    #ifdef DEBUG_MODE
    Serial.println("[FILE_SERVER] web_server is NULL!");
    #endif
    return;
  }
  
  web_server->setContentLength(CONTENT_LENGTH_UNKNOWN);
  web_server->send(200, "text/html", "");
  web_server->sendContent_P(PAGE_HEADER);

  // Attendre un peu que SD soit dispo
  vTaskDelay(pdMS_TO_TICKS(100));

  File root = SD_MMC.open("/flights");
  if (!root) {
    web_server->sendContent("<p>No flights directory found</p>");
    #ifdef DEBUG_MODE
    Serial.println("[FILE_SERVER] Cannot open /flights");
    #endif
  } else {
    File file = root.openNextFile();
    bool has_files = false;

    while (file) {
      if (!file.isDirectory()) {
        has_files = true;
        web_server->sendContent("<div class='file'>");
        web_server->sendContent("<a href='/download?file=");
        web_server->sendContent(String(file.name()));
        web_server->sendContent("'>");
        web_server->sendContent(String(file.name()));
        web_server->sendContent("</a>");
        web_server->sendContent("<span class='size'>");
        web_server->sendContent(String(file.size() / 1024));
        web_server->sendContent(" KB");
        web_server->sendContent("</span>");
        web_server->sendContent("</div>");
      }
      file.close();
      file = root.openNextFile();
    }

    if (!has_files) {
      web_server->sendContent("<p>No flight files available</p>");
    }

    root.close();
  }
  web_server->sendContent_P(PAGE_FOOTER);
}

// Handler pour le telechargement de fichiers
static void handle_download() {
  if (!web_server->hasArg("file")) {
    web_server->send(400, "text/plain", "Missing file parameter");
    return;
  }

  String filename = web_server->arg("file");
  String filepath = "/flights/" + filename;

  if (!SD_MMC.exists(filepath)) {  // CHANGÉ: SD_MMC
    web_server->send(404, "text/plain", "File not found");
    return;
  }

  File file = SD_MMC.open(filepath, FILE_READ);  // CHANGÉ: SD_MMC
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
    return;  // Deja lance
  }

  xTaskCreatePinnedToCore(
    file_server_task,
    "file_server",
    4096,
    NULL,
    3,
    &file_server_task_handle,
    0);

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