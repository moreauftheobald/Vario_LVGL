#ifndef OSM_TILE_LOADER_H
#define OSM_TILE_LOADER_H

#include <Arduino.h>
#include <SD_MMC.h>
#include <math.h>
#include "lvgl.h"
#include "constants.h"
// Choix du decodeur JPEG
#define USE_HARDWARE_JPEG 0  // 1=Hardware, 0=JPEGDEC

#if USE_HARDWARE_JPEG
#include "esp_jpeg_decoder.h"
#else
#include <JPEGDEC.h>
#endif

// ===== SYSTEME DE CACHE MULTI-ZOOM AVEC PRE-ALLOCATION =====

#define CACHE_GRID_SIZE 3
#define CACHE_ZOOM_LEVELS 3
#define TOTAL_CACHE_TILES (CACHE_GRID_SIZE * CACHE_GRID_SIZE * CACHE_ZOOM_LEVELS)

typedef struct {
  int zoom;
  int tile_x;
  int tile_y;
  uint16_t* data;  // Pointeur PRE-ALLOUE (jamais free!)
  bool valid;
  bool loading;
} CachedTile;

typedef struct {
  int center_tile_x;
  int center_tile_y;
  int zoom_level;
  CachedTile tiles[CACHE_GRID_SIZE * CACHE_GRID_SIZE];
} ZoomCache;

// Structure pour statistiques de chargement
typedef struct {
  unsigned long last_map_load_time_ms;
  unsigned long last_cache_load_time_ms;
  int last_tiles_loaded;
  int last_tiles_failed;
  int total_cache_hits;
  int total_cache_misses;
  unsigned long total_decode_time_ms;
  int total_tiles_decoded;
} TileLoadStats;

static ZoomCache tile_caches[CACHE_ZOOM_LEVELS];
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

// Stats globales
static TileLoadStats tile_stats = { 0 };

// ===== FONCTIONS STATS =====

static void get_tile_stats(TileLoadStats* stats) {
  if (stats) {
    memcpy(stats, &tile_stats, sizeof(TileLoadStats));
  }
}

static void reset_tile_stats(void) {
  tile_stats.total_cache_hits = 0;
  tile_stats.total_cache_misses = 0;
  tile_stats.total_decode_time_ms = 0;
  tile_stats.total_tiles_decoded = 0;
}

// ===== FONCTIONS DE CONVERSION =====

static void lat_lon_to_tile_pixel(double lat, double lon, int zoom,
                                  int* tile_x, int* tile_y,
                                  double* pixel_x, double* pixel_y) {
  double n = pow(2.0, zoom);
  double tile_x_f = (lon + 180.0) / 360.0 * n;
  double lat_rad = lat * M_PI / 180.0;
  double tile_y_f = (1.0 - asinh(tan(lat_rad)) / M_PI) / 2.0 * n;

  *tile_x = (int)floor(tile_x_f);
  *tile_y = (int)floor(tile_y_f);
  *pixel_x = (tile_x_f - *tile_x) * OSM_TILE_SIZE;
  *pixel_y = (tile_y_f - *tile_y) * OSM_TILE_SIZE;
}

// ===== GESTION DU CACHE AVEC PRE-ALLOCATION =====

static void init_tile_cache(void) {
  if (cache_initialized) return;

  cache_mutex = xSemaphoreCreateMutex();

#ifdef DEBUG_MODE
  Serial.println("[CACHE] Pre-allocating tile memory...");
  size_t psram_before = heap_caps_get_free_size(MALLOC_CAP_SPIRAM);
#if USE_HARDWARE_JPEG
  Serial.println("[CACHE] Using HARDWARE JPEG decoder");
#else
  Serial.println("[CACHE] Using SOFTWARE JPEG decoder");
#endif
#endif


  size_t tile_size = OSM_TILE_SIZE * OSM_TILE_SIZE * sizeof(uint16_t);
  int allocated = 0;
  int failed = 0;

  // PRE-ALLOUER TOUTES LES TILES
  for (int c = 0; c < CACHE_ZOOM_LEVELS; c++) {
    tile_caches[c].zoom_level = 0;
    tile_caches[c].center_tile_x = 0;
    tile_caches[c].center_tile_y = 0;

    for (int i = 0; i < CACHE_GRID_SIZE * CACHE_GRID_SIZE; i++) {
      // Allouer le buffer de la tile (JAMAIS libéré jusqu'au cleanup final)
      tile_caches[c].tiles[i].data = (uint16_t*)heap_caps_malloc(
        tile_size, MALLOC_CAP_SPIRAM);

      if (tile_caches[c].tiles[i].data) {
        allocated++;
        // Remplir avec gris par défaut
        for (int p = 0; p < OSM_TILE_SIZE * OSM_TILE_SIZE; p++) {
          tile_caches[c].tiles[i].data[p] = 0x7BEF;
        }
      } else {
        failed++;
#ifdef DEBUG_MODE
        Serial.printf("[CACHE] Failed to allocate tile %d/%d\n", c, i);
#endif
      }

      tile_caches[c].tiles[i].valid = false;
      tile_caches[c].tiles[i].loading = false;
      tile_caches[c].tiles[i].zoom = 0;
      tile_caches[c].tiles[i].tile_x = 0;
      tile_caches[c].tiles[i].tile_y = 0;
    }
  }

  cache_initialized = true;
  reset_tile_stats();

#ifdef DEBUG_MODE
  size_t psram_after = heap_caps_get_free_size(MALLOC_CAP_SPIRAM);
  Serial.printf("[CACHE] Pre-allocation complete:\n");
  Serial.printf("[CACHE]   Allocated: %d tiles\n", allocated);
  Serial.printf("[CACHE]   Failed: %d tiles\n", failed);
  Serial.printf("[CACHE]   Memory used: %d KB\n", (allocated * 128));
  Serial.printf("[CACHE]   PSRAM: %d KB -> %d KB (-%d KB)\n",
                psram_before / 1024, psram_after / 1024,
                (psram_before - psram_after) / 1024);
#endif

  if (failed > 0) {
    Serial.println("[CACHE] WARNING: Some tiles failed to allocate!");
  }
}

static uint16_t* get_tile_from_cache(int zoom, int tile_x, int tile_y) {
  if (!cache_initialized) return NULL;

  if (xSemaphoreTake(cache_mutex, pdMS_TO_TICKS(10)) == pdTRUE) {
    for (int c = 0; c < CACHE_ZOOM_LEVELS; c++) {
      if (tile_caches[c].zoom_level != zoom) continue;

      for (int i = 0; i < CACHE_GRID_SIZE * CACHE_GRID_SIZE; i++) {
        CachedTile* tile = &tile_caches[c].tiles[i];
        if (tile->valid && tile->zoom == zoom && tile->tile_x == tile_x && tile->tile_y == tile_y) {
          uint16_t* result = tile->data;
          tile_stats.total_cache_hits++;
          xSemaphoreGive(cache_mutex);
#ifdef DEBUG_MODE
          Serial.printf("[CACHE] HIT: %d/%d/%d (total hits: %d)\n",
                        zoom, tile_x, tile_y, tile_stats.total_cache_hits);
#endif
          return result;
        }
      }
    }
    xSemaphoreGive(cache_mutex);
  }

  tile_stats.total_cache_misses++;
#ifdef DEBUG_MODE
  Serial.printf("[CACHE] MISS: %d/%d/%d (total misses: %d)\n",
                zoom, tile_x, tile_y, tile_stats.total_cache_misses);
#endif
  return NULL;
}

// Compte le nombre de tiles valides en cache
static int count_cached_tiles(void) {
  int count = 0;
  for (int c = 0; c < CACHE_ZOOM_LEVELS; c++) {
    for (int i = 0; i < CACHE_GRID_SIZE * CACHE_GRID_SIZE; i++) {
      if (tile_caches[c].tiles[i].valid && tile_caches[c].tiles[i].data) {
        count++;
      }
    }
  }
  return count;
}

// ===== DECODAGE JPEG OPTIMISE =====

typedef struct {
  uint16_t* buffer;
  int width;
  bool error;
} JPEGContext;

static int jpegDrawCallback(JPEGDRAW* pDraw) {
  JPEGContext* ctx = (JPEGContext*)pDraw->pUser;

  if (!ctx || !ctx->buffer || ctx->error) return 0;

  if (pDraw->x + pDraw->iWidth > ctx->width || pDraw->y + pDraw->iHeight > ctx->width) {
    ctx->error = true;
    return 0;
  }

  uint16_t* dest = ctx->buffer + (pDraw->y * ctx->width) + pDraw->x;
  uint16_t* src = pDraw->pPixels;

  for (int row = 0; row < pDraw->iHeight; row++) {
    memcpy(dest, src, pDraw->iWidth * sizeof(uint16_t));
    dest += ctx->width;
    src += pDraw->iWidth;
  }

  return 1;
}

#if USE_HARDWARE_JPEG

static bool decode_jpeg_file_to_rgb565_from_sd(const char* path, uint16_t* dest_buffer) {
  if (!dest_buffer || !SD_MMC.exists(path)) return false;

  unsigned long decode_start = millis();

  File f = SD_MMC.open(path, FILE_READ);
  if (!f) return false;

  size_t file_size = f.size();
  if (file_size == 0 || file_size > 512000) {
    f.close();
    return false;
  }

  // Lire JPEG en memoire
  uint8_t* jpeg_buf = (uint8_t*)heap_caps_malloc(file_size, MALLOC_CAP_SPIRAM);
  if (!jpeg_buf) {
    f.close();
    return false;
  }

  f.read(jpeg_buf, file_size);
  f.close();

  // Config decodeur hardware
  esp_jpeg_image_cfg_t cfg = {
    .indata = jpeg_buf,
    .indata_size = file_size,
    .outbuf = (uint8_t*)dest_buffer,
    .outbuf_size = OSM_TILE_SIZE * OSM_TILE_SIZE * 2,
    .out_format = JPEG_IMAGE_FORMAT_RGB565,
    .out_scale = JPEG_IMAGE_SCALE_0,
    .flags = { .swap_color_bytes = 0 }
  };

  esp_jpeg_image_output_t out;
  esp_err_t ret = esp_jpeg_decode(&cfg, &out);

  heap_caps_free(jpeg_buf);

  if (ret != ESP_OK || out.width != OSM_TILE_SIZE || out.height != OSM_TILE_SIZE) {
    return false;
  }

  unsigned long decode_time = millis() - decode_start;
  tile_stats.total_decode_time_ms += decode_time;
  tile_stats.total_tiles_decoded++;

#ifdef DEBUG_MODE
  Serial.printf("[JPEG-HW] Decoded in %lu ms (avg: %lu ms)\n",
                decode_time,
                tile_stats.total_tiles_decoded > 0 ? tile_stats.total_decode_time_ms / tile_stats.total_tiles_decoded : 0);
#endif

  return true;
}

#else
static bool decode_jpeg_file_to_rgb565_from_sd(const char* path, uint16_t* dest_buffer) {
  if (!dest_buffer) return false;

  if (!SD_MMC.cardSize()) {
#ifdef DEBUG_MODE
    Serial.println("[JPEG] SD not ready");
#endif
    return false;
  }

  if (!SD_MMC.exists(path)) return false;

  unsigned long decode_start = millis();

  File f = SD_MMC.open(path, FILE_READ);
  if (!f) return false;

  size_t fileSize = f.size();
  if (fileSize == 0 || fileSize > 512000) {
    f.close();
    return false;
  }

  // Allouer decoder sur heap
  JPEGDEC* jpeg = (JPEGDEC*)heap_caps_malloc(sizeof(JPEGDEC), MALLOC_CAP_INTERNAL);
  if (!jpeg) {
#ifdef DEBUG_MODE
    Serial.println("[JPEG] Cannot allocate decoder");
#endif
    f.close();
    return false;
  }

  JPEGContext ctx;
  ctx.buffer = dest_buffer;
  ctx.width = OSM_TILE_SIZE;
  ctx.error = false;

  bool success = false;

  if (jpeg->open(f, jpegDrawCallback) == 1) {
    if (jpeg->getWidth() == OSM_TILE_SIZE && jpeg->getHeight() == OSM_TILE_SIZE) {
      jpeg->setUserPointer(&ctx);
      if (jpeg->decode(0, 0, 0) == 1 && !ctx.error) {
        success = true;
        tile_stats.total_tiles_decoded++;
      }
    }
#ifdef DEBUG_MODE
    else {
      Serial.printf("[JPEG] Bad size: %dx%d\n", jpeg->getWidth(), jpeg->getHeight());
    }
#endif
  }

  jpeg->close();
  heap_caps_free(jpeg);
  f.close();

  unsigned long decode_time = millis() - decode_start;
  tile_stats.total_decode_time_ms += decode_time;

#ifdef DEBUG_MODE
  if (success) {
    Serial.printf("[JPEG] Decoded %s in %lu ms (avg: %lu ms)\n",
                  path, decode_time,
                  tile_stats.total_tiles_decoded > 0 ? tile_stats.total_decode_time_ms / tile_stats.total_tiles_decoded : 0);
  }
#endif

  return success;
}
#endif

// ===== TASK DE CACHE (avec pre-allocation) =====

static void tile_cache_task(void* parameter) {
#ifdef DEBUG_MODE
  Serial.println("[CACHE] Task started");
#endif

  while (cache_task_running) {
    vTaskDelay(pdMS_TO_TICKS(1000));

    if (!cache_initialized || !SD_MMC.cardSize()) continue;

    unsigned long cache_start = millis();
    int tiles_loaded = 0;
    int tiles_failed = 0;

    bool zoom_changed = (previous_cache_zoom != current_cache_zoom && previous_cache_zoom != -1);
    bool position_changed = (fabs(previous_gps_lat - cache_gps_lat) > 0.0001 || fabs(previous_gps_lon - cache_gps_lon) > 0.0001);

    if (!zoom_changed && !position_changed && previous_cache_zoom != -1) {
      continue;
    }

#ifdef DEBUG_MODE
    if (zoom_changed) {
      Serial.printf("[CACHE] Zoom changed: %d -> %d\n",
                    previous_cache_zoom, current_cache_zoom);
    }
    if (position_changed) {
      Serial.println("[CACHE] Position changed");
    }
#endif

    int zoom_direction = 0;
    int zoom_start = -1;
    int zoom_end = 1;

    if (zoom_changed) {
      zoom_direction = (current_cache_zoom > previous_cache_zoom) ? 1 : -1;
      zoom_start = zoom_direction;
      zoom_end = zoom_direction;
    }

    previous_cache_zoom = current_cache_zoom;
    previous_gps_lat = cache_gps_lat;
    previous_gps_lon = cache_gps_lon;

    for (int zoom_offset = zoom_start; zoom_offset <= zoom_end; zoom_offset++) {
      int target_zoom = current_cache_zoom + zoom_offset;

      if (target_zoom < MAP_ZOOM_MIN || target_zoom > MAP_ZOOM_MAX) continue;

      int target_tile_x, target_tile_y;
      double dummy_x, dummy_y;
      lat_lon_to_tile_pixel(cache_gps_lat, cache_gps_lon, target_zoom,
                            &target_tile_x, &target_tile_y, &dummy_x, &dummy_y);

      int cache_idx = zoom_offset + 1;

      if (xSemaphoreTake(cache_mutex, pdMS_TO_TICKS(100)) != pdTRUE) continue;

      tile_caches[cache_idx].zoom_level = target_zoom;
      tile_caches[cache_idx].center_tile_x = target_tile_x;
      tile_caches[cache_idx].center_tile_y = target_tile_y;

      for (int dy = -1; dy <= 1; dy++) {
        for (int dx = -1; dx <= 1; dx++) {
          int tile_idx = (dy + 1) * CACHE_GRID_SIZE + (dx + 1);
          int tile_x = target_tile_x + dx;
          int tile_y = target_tile_y + dy;

          CachedTile* cached = &tile_caches[cache_idx].tiles[tile_idx];

          // Tile déjà en cache et valide
          if (cached->valid && cached->zoom == target_zoom && cached->tile_x == tile_x && cached->tile_y == tile_y) continue;

          // Déjà en chargement
          if (cached->loading) continue;

          // Vérifier que le buffer est alloué
          if (!cached->data) {
#ifdef DEBUG_MODE
            Serial.printf("[CACHE] Tile %d buffer not allocated!\n", tile_idx);
#endif
            continue;
          }

          cached->loading = true;
          cached->valid = false;

          xSemaphoreGive(cache_mutex);

          // Construire chemin JPG
          char tile_path[128];
          snprintf(tile_path, sizeof(tile_path), "%s/%s/%d/%d/%d.jpg",
                   OSM_TILES_DIR, OSM_SERVER_NAME, target_zoom, tile_x, tile_y);

          bool loaded = false;
          if (SD_MMC.exists(tile_path)) {
            // Décoder DIRECTEMENT dans le buffer pré-alloué
            loaded = decode_jpeg_file_to_rgb565_from_sd(tile_path, cached->data);
          }

          // Mise à jour cache
          if (xSemaphoreTake(cache_mutex, pdMS_TO_TICKS(100)) == pdTRUE) {
            if (loaded) {
              cached->zoom = target_zoom;
              cached->tile_x = tile_x;
              cached->tile_y = tile_y;
              cached->valid = true;
              tiles_loaded++;
            } else {
              // Remettre en gris si échec
              for (int p = 0; p < OSM_TILE_SIZE * OSM_TILE_SIZE; p++) {
                cached->data[p] = 0x7BEF;
              }
              tiles_failed++;
            }
            cached->loading = false;
            xSemaphoreGive(cache_mutex);
          } else {
            tiles_failed++;
          }

          vTaskDelay(pdMS_TO_TICKS(20));
        }
      }
      xSemaphoreGive(cache_mutex);
    }

    unsigned long cache_time = millis() - cache_start;

    if (tiles_loaded > 0 || tiles_failed > 0) {
      tile_stats.last_cache_load_time_ms = cache_time;
      tile_stats.last_tiles_loaded = tiles_loaded;
      tile_stats.last_tiles_failed = tiles_failed;

#ifdef DEBUG_MODE
      Serial.printf("[CACHE] === REFRESH COMPLETE ===\n");
      Serial.printf("[CACHE] Loaded: %d, Failed: %d\n", tiles_loaded, tiles_failed);
      Serial.printf("[CACHE] Time: %lu ms (avg %lu ms/tile)\n",
                    cache_time,
                    tiles_loaded > 0 ? cache_time / tiles_loaded : 0);
      Serial.printf("[CACHE] Total cached: %d tiles = %d KB\n",
                    count_cached_tiles(),
                    count_cached_tiles() * 128);
      Serial.printf("[CACHE] Cache hits: %d, misses: %d (hit rate: %d%%)\n",
                    tile_stats.total_cache_hits,
                    tile_stats.total_cache_misses,
                    (tile_stats.total_cache_hits + tile_stats.total_cache_misses) > 0 ? (tile_stats.total_cache_hits * 100) / (tile_stats.total_cache_hits + tile_stats.total_cache_misses) : 0);
      Serial.printf("[CACHE] PSRAM free: %d KB\n",
                    heap_caps_get_free_size(MALLOC_CAP_SPIRAM) / 1024);
      Serial.printf("[CACHE] ==================\n");
#endif
    }
  }

  vTaskDelete(NULL);
}

static void start_tile_cache_task(int zoom, double lat, double lon) {
  if (!cache_initialized) init_tile_cache();

  current_cache_zoom = zoom;
  cache_gps_lat = lat;
  cache_gps_lon = lon;

  if (!cache_task_running) {
    cache_task_running = true;
    xTaskCreatePinnedToCore(
      tile_cache_task,
      "TileCache",
      8192,
      NULL,
      1,
      &cache_task_handle,
      0);

#ifdef DEBUG_MODE
    Serial.println("[CACHE] Task started on core 0");
#endif
  }
}

static void update_cache_position(int zoom, double lat, double lon) {
  if (!cache_initialized) return;
  current_cache_zoom = zoom;
  cache_gps_lat = lat;
  cache_gps_lon = lon;
}

// ===== CHARGEMENT TILE =====

static bool load_single_tile(int zoom, int tile_x, int tile_y, uint16_t* buffer) {
  uint16_t* cached = get_tile_from_cache(zoom, tile_x, tile_y);
  if (cached) {
    memcpy(buffer, cached, OSM_TILE_SIZE * OSM_TILE_SIZE * sizeof(uint16_t));
    return true;
  }

  char tile_path[128];
  snprintf(tile_path, sizeof(tile_path), "%s/%s/%d/%d/%d.jpg",
           OSM_TILES_DIR, OSM_SERVER_NAME, zoom, tile_x, tile_y);

  if (!SD_MMC.exists(tile_path)) return false;

  return decode_jpeg_file_to_rgb565_from_sd(tile_path, buffer);
}

// ===== CREATION VUE CARTE =====

static lv_obj_t* create_map_view(lv_obj_t* parent, double lat, double lon, int zoom,
                                 int view_width, int view_height) {
  unsigned long map_start = millis();

#ifdef DEBUG_MODE
  UBaseType_t stack_remaining = uxTaskGetStackHighWaterMark(NULL);
  Serial.printf("\n[OSM] ===== MAP VIEW CREATION START =====\n");
  Serial.printf("[OSM] Stack available: %d bytes\n", stack_remaining * sizeof(StackType_t));
  Serial.printf("[OSM] View size: %dx%d, Zoom: %d\n", view_width, view_height, zoom);
#endif

  int upscale_factor = 2;
  int actual_zoom = zoom;

  if (zoom > MAP_ZOOM_MAX) {
    upscale_factor = 3;
    actual_zoom = MAP_ZOOM_MAX;
#ifdef DEBUG_MODE
    Serial.printf("[OSM] Super zoom mode x3 (zoom %d -> %d)\n", zoom, actual_zoom);
#endif
  }

  unsigned long calc_start = millis();

  int center_tile_x, center_tile_y;
  double center_pixel_x, center_pixel_y;
  lat_lon_to_tile_pixel(lat, lon, actual_zoom,
                        &center_tile_x, &center_tile_y,
                        &center_pixel_x, &center_pixel_y);

  update_cache_position(actual_zoom, lat, lon);

  int half_view_tiles = (view_width / upscale_factor) / 2;

  double gps_global_x = center_tile_x * OSM_TILE_SIZE + center_pixel_x;
  double gps_global_y = center_tile_y * OSM_TILE_SIZE + center_pixel_y;

  int tile_left = (int)floor((gps_global_x - half_view_tiles) / OSM_TILE_SIZE);
  int tile_top = (int)floor((gps_global_y - half_view_tiles) / OSM_TILE_SIZE);
  int tile_right = (int)floor((gps_global_x + half_view_tiles) / OSM_TILE_SIZE);
  int tile_bottom = (int)floor((gps_global_y + half_view_tiles) / OSM_TILE_SIZE);

  int tiles_cols = tile_right - tile_left + 1;
  int tiles_rows = tile_bottom - tile_top + 1;
  int total_tiles = tiles_cols * tiles_rows;

#ifdef DEBUG_MODE
  Serial.printf("[OSM] Tiles grid: %dx%d = %d tiles\n",
                tiles_cols, tiles_rows, total_tiles);
  Serial.printf("[OSM] Calc time: %lu ms\n", millis() - calc_start);
#endif

  unsigned long alloc_start = millis();

  uint16_t* view_buffer = (uint16_t*)heap_caps_malloc(
    view_width * view_height * sizeof(uint16_t),
    MALLOC_CAP_SPIRAM);

  if (!view_buffer) {
#ifdef DEBUG_MODE
    Serial.println("[OSM] View buffer allocation FAILED");
#endif
    return NULL;
  }

  uint16_t* tile_buffer = (uint16_t*)heap_caps_malloc(
    OSM_TILE_SIZE * OSM_TILE_SIZE * sizeof(uint16_t),
    MALLOC_CAP_SPIRAM);

  if (!tile_buffer) {
    heap_caps_free(view_buffer);
#ifdef DEBUG_MODE
    Serial.println("[OSM] Tile buffer allocation FAILED");
#endif
    return NULL;
  }

#ifdef DEBUG_MODE
  Serial.printf("[OSM] Buffers allocated in %lu ms\n", millis() - alloc_start);
#endif

  memset(view_buffer, 0x7BEF, view_width * view_height * sizeof(uint16_t));

  int tile_display_size = OSM_TILE_SIZE * upscale_factor;
  double first_tile_global_x = tile_left * OSM_TILE_SIZE;
  double first_tile_global_y = tile_top * OSM_TILE_SIZE;

  int start_pixel_x = (view_width / 2) - (int)((gps_global_x - first_tile_global_x) * upscale_factor);
  int start_pixel_y = (view_height / 2) - (int)((gps_global_y - first_tile_global_y) * upscale_factor);

  unsigned long tiles_start = millis();
  int tiles_loaded_count = 0;
  int tiles_from_cache = 0;

  for (int dy = 0; dy < tiles_rows; dy++) {
    for (int dx = 0; dx < tiles_cols; dx++) {
      unsigned long tile_start = millis();

      int tile_x = tile_left + dx;
      int tile_y = tile_top + dy;

      int draw_x = start_pixel_x + (dx * tile_display_size);
      int draw_y = start_pixel_y + (dy * tile_display_size);

      int cache_hits_before = tile_stats.total_cache_hits;
      bool loaded = load_single_tile(actual_zoom, tile_x, tile_y, tile_buffer);
      bool from_cache = (tile_stats.total_cache_hits > cache_hits_before);

      if (loaded) {
        tiles_loaded_count++;
        if (from_cache) tiles_from_cache++;
      } else {
        memset(tile_buffer, 0x7BEF, OSM_TILE_SIZE * OSM_TILE_SIZE * sizeof(uint16_t));
      }

      unsigned long tile_load_time = millis() - tile_start;
      unsigned long upscale_start = millis();

      // Upscale et copie
      for (int ty = 0; ty < OSM_TILE_SIZE; ty++) {
        for (int tx = 0; tx < OSM_TILE_SIZE; tx++) {
          uint16_t pixel = tile_buffer[ty * OSM_TILE_SIZE + tx];

          for (int py = 0; py < upscale_factor; py++) {
            for (int px = 0; px < upscale_factor; px++) {
              int dest_x = draw_x + (tx * upscale_factor) + px;
              int dest_y = draw_y + (ty * upscale_factor) + py;

              if (dest_x >= 0 && dest_x < view_width && dest_y >= 0 && dest_y < view_height) {
                view_buffer[dest_y * view_width + dest_x] = pixel;
              }
            }
          }
        }
      }

      unsigned long tile_total = millis() - tile_start;

#ifdef DEBUG_MODE
      Serial.printf("[OSM] Tile [%d,%d] %d/%d/%d: load=%lu ms %s, upscale=%lu ms, total=%lu ms\n",
                    dx, dy, actual_zoom, tile_x, tile_y,
                    tile_load_time, from_cache ? "(cache)" : "(SD)",
                    millis() - upscale_start, tile_total);
#endif
    }
  }

  unsigned long tiles_time = millis() - tiles_start;

  heap_caps_free(tile_buffer);

  unsigned long canvas_start = millis();

  lv_obj_t* canvas = lv_canvas_create(parent);
  lv_canvas_set_buffer(canvas, view_buffer,
                       view_width, view_height,
                       LV_COLOR_FORMAT_RGB565);

  unsigned long canvas_time = millis() - canvas_start;
  unsigned long total_time = millis() - map_start;

  tile_stats.last_map_load_time_ms = total_time;

#ifdef DEBUG_MODE
  UBaseType_t stack_after = uxTaskGetStackHighWaterMark(NULL);

  Serial.printf("\n[OSM] ===== MAP VIEW CREATION COMPLETE =====\n");
  Serial.printf("[OSM] Tiles processing: %lu ms (avg %lu ms/tile)\n",
                tiles_time, total_tiles > 0 ? tiles_time / total_tiles : 0);
  Serial.printf("[OSM] Tiles loaded: %d/%d (%d from cache, %d from SD)\n",
                tiles_loaded_count, total_tiles, tiles_from_cache,
                tiles_loaded_count - tiles_from_cache);
  Serial.printf("[OSM] Canvas creation: %lu ms\n", canvas_time);
  Serial.printf("[OSM] === TOTAL MAP TIME: %lu ms ===\n", total_time);
  Serial.printf("[OSM] Stack used: %d bytes\n",
                (stack_remaining - stack_after) * sizeof(StackType_t));
  Serial.printf("[MEM] PSRAM free: %d KB, SRAM free: %d KB\n",
                heap_caps_get_free_size(MALLOC_CAP_SPIRAM) / 1024,
                heap_caps_get_free_size(MALLOC_CAP_INTERNAL) / 1024);
  Serial.printf("[OSM] ========================================\n\n");
#else
  Serial.printf("[OSM] Map loaded in %lu ms (%d tiles, %d from cache)\n",
                total_time, tiles_loaded_count, tiles_from_cache);
#endif

  return canvas;
}

// ===== NETTOYAGE =====

static void cleanup_tile_cache(void) {
  if (!cache_initialized) return;

  cache_task_running = false;

  if (cache_task_handle) {
    vTaskDelay(pdMS_TO_TICKS(100));
    cache_task_handle = NULL;
  }

#ifdef DEBUG_MODE
  Serial.println("[CACHE] Cleanup: freeing pre-allocated tiles...");
#endif

  // Libérer tous les buffers pré-alloués
  if (cache_mutex && xSemaphoreTake(cache_mutex, pdMS_TO_TICKS(1000)) == pdTRUE) {
    int freed = 0;
    for (int c = 0; c < CACHE_ZOOM_LEVELS; c++) {
      for (int i = 0; i < CACHE_GRID_SIZE * CACHE_GRID_SIZE; i++) {
        if (tile_caches[c].tiles[i].data) {
          heap_caps_free(tile_caches[c].tiles[i].data);
          tile_caches[c].tiles[i].data = NULL;
          freed++;
        }
        tile_caches[c].tiles[i].valid = false;
      }
    }
    xSemaphoreGive(cache_mutex);

#ifdef DEBUG_MODE
    Serial.printf("[CACHE] Freed %d pre-allocated tiles\n", freed);
#endif
  }

#ifdef DEBUG_MODE
  Serial.println("[CACHE] Cleanup complete");
  Serial.printf("[STATS] Total tiles decoded: %d in %lu ms (avg %lu ms/tile)\n",
                tile_stats.total_tiles_decoded,
                tile_stats.total_decode_time_ms,
                tile_stats.total_tiles_decoded > 0 ? tile_stats.total_decode_time_ms / tile_stats.total_tiles_decoded : 0);
#endif
}

#endif  // OSM_TILE_LOADER_H