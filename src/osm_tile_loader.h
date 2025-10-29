#ifndef OSM_TILE_LOADER_H
#define OSM_TILE_LOADER_H

#include <Arduino.h>
#include <SD_MMC.h>
#include <math.h>
#include "lvgl.h"
#include "constants.h"

// ===== SYSTEME DE CACHE MULTI-ZOOM ASYNCHRONE =====

#define CACHE_GRID_SIZE 3    // 3x3 tuiles par cache (9 tuiles)
#define CACHE_ZOOM_LEVELS 3  // 3 niveaux de zoom (current-1, current, current+1)

typedef struct {
  int zoom;
  int tile_x;
  int tile_y;
  uint16_t* data;
  bool valid;
  bool loading;  // En cours de chargement
} CachedTile;

typedef struct {
  int center_tile_x;
  int center_tile_y;
  int zoom_level;
  CachedTile tiles[CACHE_GRID_SIZE * CACHE_GRID_SIZE];
} ZoomCache;

static ZoomCache tile_caches[CACHE_ZOOM_LEVELS];  // -1, 0, +1
static int current_cache_zoom = 0;
static double cache_gps_lat = 0.0;
static double cache_gps_lon = 0.0;
static bool cache_initialized = false;
static SemaphoreHandle_t cache_mutex = NULL;
static TaskHandle_t cache_task_handle = NULL;
static bool cache_task_running = false;
static int previous_cache_zoom = -1;
static double previous_gps_lat = 0.0;
static double previous_gps_lon = 0.0;

// ===== FONCTIONS DE CONVERSION =====

// Conversion lat/lon vers tuile OSM et position pixel dans la tuile
static void lat_lon_to_tile_pixel(double lat, double lon, int zoom,
                                  int* tile_x, int* tile_y,
                                  double* pixel_x, double* pixel_y) {
  double n = pow(2.0, zoom);

  // Position en tuiles (avec décimales)
  double tile_x_f = (lon + 180.0) / 360.0 * n;
  double lat_rad = lat * M_PI / 180.0;
  double tile_y_f = (1.0 - asinh(tan(lat_rad)) / M_PI) / 2.0 * n;

  // Index de tuile (partie entière)
  *tile_x = (int)floor(tile_x_f);
  *tile_y = (int)floor(tile_y_f);

  // Position pixel dans la tuile (partie décimale * 256)
  *pixel_x = (tile_x_f - *tile_x) * OSM_TILE_SIZE;
  *pixel_y = (tile_y_f - *tile_y) * OSM_TILE_SIZE;
}

// ===== GESTION DU CACHE =====

// Initialiser le systeme de cache
static void init_tile_cache(void) {
  if (cache_initialized) return;

  cache_mutex = xSemaphoreCreateMutex();

  // Initialiser les 3 caches
  for (int c = 0; c < CACHE_ZOOM_LEVELS; c++) {
    tile_caches[c].zoom_level = 0;
    tile_caches[c].center_tile_x = 0;
    tile_caches[c].center_tile_y = 0;

    for (int i = 0; i < CACHE_GRID_SIZE * CACHE_GRID_SIZE; i++) {
      tile_caches[c].tiles[i].data = NULL;
      tile_caches[c].tiles[i].valid = false;
      tile_caches[c].tiles[i].loading = false;
    }
  }

  cache_initialized = true;

#ifdef DEBUG_MODE
  Serial.println("[CACHE] Tile cache system initialized");
#endif
}

// Trouver une tuile dans le cache
static uint16_t* get_tile_from_cache(int zoom, int tile_x, int tile_y) {
  if (!cache_initialized) return NULL;

  if (xSemaphoreTake(cache_mutex, pdMS_TO_TICKS(10)) == pdTRUE) {
    // Chercher dans les 3 niveaux de cache
    for (int c = 0; c < CACHE_ZOOM_LEVELS; c++) {
      if (tile_caches[c].zoom_level != zoom) continue;

      for (int i = 0; i < CACHE_GRID_SIZE * CACHE_GRID_SIZE; i++) {
        CachedTile* tile = &tile_caches[c].tiles[i];
        if (tile->valid && tile->zoom == zoom && tile->tile_x == tile_x && tile->tile_y == tile_y) {

          uint16_t* result = tile->data;
          xSemaphoreGive(cache_mutex);

#ifdef DEBUG_MODE
          Serial.printf("[CACHE] HIT: %d/%d/%d\n", zoom, tile_x, tile_y);
#endif
          return result;
        }
      }
    }
    xSemaphoreGive(cache_mutex);
  }

#ifdef DEBUG_MODE
  Serial.printf("[CACHE] MISS: %d/%d/%d\n", zoom, tile_x, tile_y);
#endif
  return NULL;
}

// Task de gestion du cache (tourne sur core 0) - avec timing
static void tile_cache_task(void* parameter) {
#ifdef DEBUG_MODE
  Serial.println("[CACHE] Task started on core 0");
#endif

  while (cache_task_running) {
    // Attendre 1 seconde avant de traiter
    vTaskDelay(pdMS_TO_TICKS(1000));

    if (!cache_initialized) continue;

#ifdef DEBUG_MODE
    unsigned long task_start = millis();
#endif

    int tiles_loaded = 0;
    int tiles_failed = 0;

    // Détecter si le zoom a changé
    bool zoom_changed = (previous_cache_zoom != current_cache_zoom && previous_cache_zoom != -1);

    // Détecter si la position GPS a changé (seuil de 0.0001° ~ 11m)
    bool position_changed = (fabs(previous_gps_lat - cache_gps_lat) > 0.0001 || fabs(previous_gps_lon - cache_gps_lon) > 0.0001);

    // Si rien n'a changé, skip
    if (!zoom_changed && !position_changed && previous_cache_zoom != -1) {
      continue;
    }

    int zoom_direction = 0;
    int zoom_start = -1;
    int zoom_end = 1;

    if (zoom_changed) {
      zoom_direction = (current_cache_zoom > previous_cache_zoom) ? 1 : -1;
      // Charger seulement le nouveau niveau nécessaire
      zoom_start = zoom_direction;
      zoom_end = zoom_direction;

#ifdef DEBUG_MODE
      Serial.printf("[CACHE] Zoom changed: %d -> %d (loading zoom %d)\n",
                    previous_cache_zoom, current_cache_zoom,
                    current_cache_zoom + zoom_direction);
#endif
    } else if (position_changed) {
      // Position GPS a changé, charger tous les niveaux
      zoom_start = -1;
      zoom_end = 1;

#ifdef DEBUG_MODE
      Serial.printf("[CACHE] Position changed: (%.6f,%.6f) -> (%.6f,%.6f)\n",
                    previous_gps_lat, previous_gps_lon,
                    cache_gps_lat, cache_gps_lon);
#endif
    } else {
      // Premier chargement, charger tous les niveaux
      zoom_start = -1;
      zoom_end = 1;

#ifdef DEBUG_MODE
      Serial.println("[CACHE] Initial load");
#endif
    }

    previous_cache_zoom = current_cache_zoom;
    previous_gps_lat = cache_gps_lat;
    previous_gps_lon = cache_gps_lon;

    // Recalculer les caches
    for (int zoom_offset = zoom_start; zoom_offset <= zoom_end; zoom_offset++) {
      int target_zoom = current_cache_zoom + zoom_offset;

      // Vérifier limites zoom
      if (target_zoom < MAP_ZOOM_MIN || target_zoom > MAP_ZOOM_MAX) {
        continue;
      }

      // IMPORTANT: Recalculer les coordonnées de tuiles pour ce zoom
      int target_tile_x, target_tile_y;
      double dummy_x, dummy_y;
      lat_lon_to_tile_pixel(cache_gps_lat, cache_gps_lon, target_zoom,
                            &target_tile_x, &target_tile_y, &dummy_x, &dummy_y);

      int cache_idx = zoom_offset + 1;  // -1->0, 0->1, +1->2

      if (xSemaphoreTake(cache_mutex, pdMS_TO_TICKS(50)) == pdTRUE) {
        tile_caches[cache_idx].zoom_level = target_zoom;
        tile_caches[cache_idx].center_tile_x = target_tile_x;
        tile_caches[cache_idx].center_tile_y = target_tile_y;

        // Charger les 9 tuiles (3x3) autour du centre
        for (int dy = -1; dy <= 1; dy++) {
          for (int dx = -1; dx <= 1; dx++) {
            int tile_idx = (dy + 1) * CACHE_GRID_SIZE + (dx + 1);
            int tile_x = target_tile_x + dx;
            int tile_y = target_tile_y + dy;

            CachedTile* cached = &tile_caches[cache_idx].tiles[tile_idx];

            // Si déjà valide et bonne tuile, skip
            if (cached->valid && cached->zoom == target_zoom && cached->tile_x == tile_x && cached->tile_y == tile_y) {
              continue;
            }

            // Si en cours de chargement, skip
            if (cached->loading) {
              continue;
            }

            // Libérer ancienne donnée si existe
            if (cached->data) {
              heap_caps_free(cached->data);
              cached->data = NULL;
            }

            cached->loading = true;
            cached->valid = false;

            xSemaphoreGive(cache_mutex);

            // Allouer buffer
            uint16_t* tile_data = (uint16_t*)heap_caps_malloc(
              OSM_TILE_SIZE * OSM_TILE_SIZE * sizeof(uint16_t),
              MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);

            if (!tile_data) {
              tiles_failed++;
              if (xSemaphoreTake(cache_mutex, pdMS_TO_TICKS(50)) == pdTRUE) {
                cached->loading = false;
                xSemaphoreGive(cache_mutex);
              }
              continue;
            }

            // Charger depuis SD EN CHUNKS pour ne pas bloquer
            char tile_path[128];
            snprintf(tile_path, sizeof(tile_path), "%s/%s/%d/%d/%d.bin",
                     OSM_TILES_DIR, OSM_SERVER_NAME, target_zoom, tile_x, tile_y);

            bool loaded = false;
            if (SD_MMC.exists(tile_path)) {
              File tile_file = SD_MMC.open(tile_path, FILE_READ);
              if (tile_file) {
                size_t expected = OSM_TILE_SIZE * OSM_TILE_SIZE * 2;  // 131072 bytes
                if (tile_file.size() == expected) {
                  // Lire par chunks de 16KB avec pauses pour ne pas bloquer
                  const size_t chunk_size = 8192;  // 16KB par chunk
                  size_t bytes_read = 0;
                  loaded = true;

                  while (bytes_read < expected) {
                    size_t to_read = min(chunk_size, expected - bytes_read);
                    size_t chunk_bytes = tile_file.read(
                      ((uint8_t*)tile_data) + bytes_read,
                      to_read);

                    if (chunk_bytes != to_read) {
                      loaded = false;
                      break;
                    }

                    bytes_read += chunk_bytes;

                    // Pause de 12ms après chaque chunk pour laisser place aux capteurs
                    // Lecture 8KB prend ~8-10ms + 2ms pause = ~10-12ms max entre yields
                    vTaskDelay(pdMS_TO_TICKS(12));
                  }
                }
                tile_file.close();
              }
            }

            // Mettre à jour le cache
            if (xSemaphoreTake(cache_mutex, pdMS_TO_TICKS(50)) == pdTRUE) {
              if (loaded) {
                cached->data = tile_data;
                cached->zoom = target_zoom;
                cached->tile_x = tile_x;
                cached->tile_y = tile_y;
                cached->valid = true;
                tiles_loaded++;
              } else {
                heap_caps_free(tile_data);
                cached->data = NULL;
                tiles_failed++;
              }
              cached->loading = false;
              xSemaphoreGive(cache_mutex);
            }

            // Reprendre mutex pour prochaine itération
            if (xSemaphoreTake(cache_mutex, pdMS_TO_TICKS(50)) != pdTRUE) {
              break;
            }
          }
        }
        xSemaphoreGive(cache_mutex);
      }
    }

#ifdef DEBUG_MODE
    if (tiles_loaded > 0 || tiles_failed > 0) {
      unsigned long task_time = millis() - task_start;
      Serial.printf("[CACHE] Refresh: %d loaded, %d failed (%lu ms)\n",
                    tiles_loaded, tiles_failed, task_time);
    }
#endif
  }

#ifdef DEBUG_MODE
  Serial.println("[CACHE] Task stopped");
#endif

  vTaskDelete(NULL);
}

// Démarrer le système de cache
static void start_tile_cache_task(int zoom, double lat, double lon) {
  if (!cache_initialized) {
    init_tile_cache();
  }

  current_cache_zoom = zoom;
  cache_gps_lat = lat;
  cache_gps_lon = lon;

  if (!cache_task_running) {
    cache_task_running = true;
    xTaskCreatePinnedToCore(
      tile_cache_task,
      "TileCache",
      4096,
      NULL,
      1,  // Priorité faible
      &cache_task_handle,
      0  // Core 0
    );

#ifdef DEBUG_MODE
    Serial.println("[CACHE] Background task started");
#endif
  }
}

// Mettre à jour la position du cache
static void update_cache_position(int zoom, double lat, double lon) {
  if (!cache_initialized) return;

  current_cache_zoom = zoom;
  cache_gps_lat = lat;
  cache_gps_lon = lon;

#ifdef DEBUG_MODE
  Serial.printf("[CACHE] Position updated: zoom=%d, lat=%.6f, lon=%.6f\n",
                zoom, lat, lon);
#endif
}

// ===== CHARGEMENT DES TUILES =====

// Charger une tuile depuis SD (avec cache)
static bool load_single_tile(int zoom, int tile_x, int tile_y, uint16_t* buffer) {
  // Vérifier le cache d'abord
  uint16_t* cached = get_tile_from_cache(zoom, tile_x, tile_y);
  if (cached) {
    memcpy(buffer, cached, OSM_TILE_SIZE * OSM_TILE_SIZE * sizeof(uint16_t));
    return true;
  }

  // Sinon charger depuis SD (fallback)
  char tile_path[128];
  snprintf(tile_path, sizeof(tile_path), "%s/%s/%d/%d/%d.bin",
           OSM_TILES_DIR, OSM_SERVER_NAME, zoom, tile_x, tile_y);

  if (!SD_MMC.exists(tile_path)) {
#ifdef DEBUG_MODE
    Serial.printf("[OSM] Tile not found: %s\n", tile_path);
#endif
    return false;
  }

  File tile_file = SD_MMC.open(tile_path, FILE_READ);
  if (!tile_file) {
    return false;
  }

  size_t expected_size = OSM_TILE_SIZE * OSM_TILE_SIZE * 2;
  if (tile_file.size() != expected_size) {
    tile_file.close();
    return false;
  }

  size_t bytes_read = tile_file.read((uint8_t*)buffer, expected_size);
  tile_file.close();

  if (bytes_read != expected_size) {
    return false;
  }

  return true;
}

// ===== CREATION DE LA VUE CARTE =====

// Créer un canvas multi-tuiles pour remplir un cadre donné
static lv_obj_t* create_map_view(lv_obj_t* parent, double lat, double lon, int zoom,
                                 int view_width, int view_height) {
#ifdef DEBUG_MODE
  unsigned long start_time = millis();
#endif

  int upscale_factor = 2;  // Par defaut x2 pour meilleure lisibilite
  int actual_zoom = zoom;

  // Detecter super zoom x3
  if (zoom > MAP_ZOOM_MAX) {
    upscale_factor = 3;  // Super zoom x3
    actual_zoom = MAP_ZOOM_MAX;
#ifdef DEBUG_MODE
    Serial.println("[OSM] Super zoom x3 mode activated");
#endif
  }

#ifdef DEBUG_MODE
  Serial.printf("[OSM] Creating map view: %dx%d at zoom %d (actual: %d, scale: %dx)\n",
                view_width, view_height, zoom, actual_zoom, upscale_factor);
#endif

  // Calculer position centrale
  int center_tile_x, center_tile_y;
  double center_pixel_x, center_pixel_y;
  lat_lon_to_tile_pixel(lat, lon, actual_zoom,
                        &center_tile_x, &center_tile_y,
                        &center_pixel_x, &center_pixel_y);

#ifdef DEBUG_MODE
  Serial.printf("[OSM] Center tile: %d,%d  Pixel in tile: %.1f,%.1f\n",
                center_tile_x, center_tile_y, center_pixel_x, center_pixel_y);
#endif

  // Mettre à jour la position du cache
  update_cache_position(actual_zoom, lat, lon);

  // Calculer la zone visible en pixels de tuile (avant upscaling)
  int half_view_tiles = (view_width / upscale_factor) / 2;

  // Position GPS en coordonnées globales (pixels absolus dans le système de tuiles)
  double gps_global_x = center_tile_x * OSM_TILE_SIZE + center_pixel_x;
  double gps_global_y = center_tile_y * OSM_TILE_SIZE + center_pixel_y;

  // Coins de la zone visible (en pixels globaux)
  double view_left = gps_global_x - half_view_tiles;
  double view_top = gps_global_y - half_view_tiles;
  double view_right = gps_global_x + half_view_tiles;
  double view_bottom = gps_global_y + half_view_tiles;

  // Convertir en indices de tuiles
  int tile_left = (int)floor(view_left / OSM_TILE_SIZE);
  int tile_top = (int)floor(view_top / OSM_TILE_SIZE);
  int tile_right = (int)floor(view_right / OSM_TILE_SIZE);
  int tile_bottom = (int)floor(view_bottom / OSM_TILE_SIZE);

  int tiles_cols = tile_right - tile_left + 1;
  int tiles_rows = tile_bottom - tile_top + 1;

#ifdef DEBUG_MODE
  Serial.printf("[OSM] View area: tiles from (%d,%d) to (%d,%d) = %dx%d tiles\n",
                tile_left, tile_top, tile_right, tile_bottom, tiles_cols, tiles_rows);
#endif

  // Allouer buffer pour le canvas final
  static uint16_t* view_buffer = NULL;
  if (view_buffer != NULL) {
    heap_caps_free(view_buffer);
  }

  view_buffer = (uint16_t*)heap_caps_malloc(
    view_width * view_height * sizeof(uint16_t),
    MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);

  if (!view_buffer) {
#ifdef DEBUG_MODE
    Serial.println("[OSM] Cannot allocate view buffer");
#endif
    return NULL;
  }

#ifdef DEBUG_MODE
  unsigned long alloc_time = millis();
  Serial.printf("[OSM] Buffer allocation: %lu ms\n", alloc_time - start_time);
#endif

  // Remplir de gris par defaut
  for (int i = 0; i < view_width * view_height; i++) {
    view_buffer[i] = 0x7BEF;
  }

  // Buffer temporaire pour une tuile
  uint16_t* tile_buffer = (uint16_t*)heap_caps_malloc(
    OSM_TILE_SIZE * OSM_TILE_SIZE * sizeof(uint16_t),
    MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);

  if (!tile_buffer) {
#ifdef DEBUG_MODE
    Serial.println("[OSM] Cannot allocate tile buffer");
#endif
    heap_caps_free(view_buffer);
    return NULL;
  }

  // Position de départ pour le dessin
  int tile_display_size = OSM_TILE_SIZE * upscale_factor;

  // Le coin haut-gauche de la première tuile en coordonnées globales
  double first_tile_global_x = tile_left * OSM_TILE_SIZE;
  double first_tile_global_y = tile_top * OSM_TILE_SIZE;

  // Décalage pour centrer le GPS dans la vue
  int start_pixel_x = (view_width / 2) - (int)((gps_global_x - first_tile_global_x) * upscale_factor);
  int start_pixel_y = (view_height / 2) - (int)((gps_global_y - first_tile_global_y) * upscale_factor);

#ifdef DEBUG_MODE
  Serial.printf("[OSM] Drawing start: %d,%d (tile_size: %d)\n",
                start_pixel_x, start_pixel_y, tile_display_size);
  unsigned long tiles_start = millis();
#endif

  // Charger et copier les tuiles nécessaires
  for (int dy = 0; dy < tiles_rows; dy++) {
    for (int dx = 0; dx < tiles_cols; dx++) {
#ifdef DEBUG_MODE
      unsigned long tile_start = millis();
#endif
      int tile_x = tile_left + dx;
      int tile_y = tile_top + dy;

      // Position où dessiner cette tuile dans le canvas
      int draw_x = start_pixel_x + (dx * tile_display_size);
      int draw_y = start_pixel_y + (dy * tile_display_size);

      // Charger la tuile
      bool tile_loaded = load_single_tile(actual_zoom, tile_x, tile_y, tile_buffer);

      if (!tile_loaded) {
        // Remplir de gris si tuile manquante
        for (int i = 0; i < OSM_TILE_SIZE * OSM_TILE_SIZE; i++) {
          tile_buffer[i] = 0x7BEF;
        }
      }

#ifdef DEBUG_MODE
      unsigned long tile_load_time = millis() - tile_start;
#endif

      // Copier avec upscaling (toujours actif x2 ou x3)
      for (int ty = 0; ty < OSM_TILE_SIZE; ty++) {
        for (int tx = 0; tx < OSM_TILE_SIZE; tx++) {
          uint16_t pixel = tile_buffer[ty * OSM_TILE_SIZE + tx];

          // Dessiner NxN pixels pour chaque pixel source
          for (int py = 0; py < upscale_factor; py++) {
            for (int px = 0; px < upscale_factor; px++) {
              int dest_x = draw_x + (tx * upscale_factor) + px;
              int dest_y = draw_y + (ty * upscale_factor) + py;

              // Verifier si dans les limites du canvas
              if (dest_x >= 0 && dest_x < view_width && dest_y >= 0 && dest_y < view_height) {
                view_buffer[dest_y * view_width + dest_x] = pixel;
              }
            }
          }
        }
      }

#ifdef DEBUG_MODE
      unsigned long tile_total_time = millis() - tile_start;
      Serial.printf("[OSM] Tile %d,%d: load=%lu ms, upscale+draw=%lu ms, total=%lu ms\n",
                    tile_x, tile_y, tile_load_time,
                    tile_total_time - tile_load_time, tile_total_time);
#endif
    }
  }

#ifdef DEBUG_MODE
  unsigned long tiles_total_time = millis() - tiles_start;
  Serial.printf("[OSM] All tiles processed in: %lu ms\n", tiles_total_time);
#endif

  heap_caps_free(tile_buffer);

  // Créer canvas LVGL
  lv_obj_t* canvas = lv_canvas_create(parent);
  lv_canvas_set_buffer(canvas, view_buffer,
                       view_width, view_height,
                       LV_COLOR_FORMAT_RGB565);

#ifdef DEBUG_MODE
  unsigned long total_time = millis() - start_time;
  Serial.printf("[OSM] ===== TOTAL MAP LOADING TIME: %lu ms =====\n", total_time);
#endif

  update_cache_position(actual_zoom, lat, lon);
  return canvas;
}

#endif  // OSM_TILE_LOADER_H