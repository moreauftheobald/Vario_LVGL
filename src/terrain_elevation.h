// terrain_elevation.h
#ifndef TERRAIN_ELEVATION_H
#define TERRAIN_ELEVATION_H

#include "SD_MMC.h"
#include "constants.h"

class TerrainElevation {
private:
  // Info tile courante
  String cachedFilename;
  int gridSize;
  bool tileLoaded;

  // Cache carré
  int16_t* cacheData;    // Données altitude
  int cacheSize;         // Taille actuelle cache (pixels)
  float cacheCenterLat;  // Centre cache
  float cacheCenterLon;
  int cacheStartRow;  // Position dans tile
  int cacheStartCol;
  bool cacheValid;

  SemaphoreHandle_t cache_mutex;

  String getHGTPath(float lat, float lon) {
    int ilat = (int)floor(lat);
    int ilon = (int)floor(lon);

    char path[32];
    sprintf(path, "/hgt/%c%02d%c%03d.hgt",
            (ilat >= 0) ? 'N' : 'S', abs(ilat),
            (ilon >= 0) ? 'E' : 'W', abs(ilon));

    return String(path);
  }

  // Calcul distance haversine simple (approximation)
  float distanceMeters(float lat1, float lon1, float lat2, float lon2) {
    const float R = 6371000.0;  // Rayon terre (m)
    float dlat = (lat2 - lat1) * M_PI / 180.0;
    float dlon = (lon2 - lon1) * M_PI / 180.0;
    float a = sin(dlat / 2) * sin(dlat / 2) + cos(lat1 * M_PI / 180.0) * cos(lat2 * M_PI / 180.0) * sin(dlon / 2) * sin(dlon / 2);
    return R * 2.0 * atan2(sqrt(a), sqrt(1 - a));
  }

  // Ouvre tile et détecte résolution
  bool openTile(String filename) {
    if (tileLoaded && cachedFilename == filename) {
      return true;
    }

    extern SemaphoreHandle_t sd_mutex;
    if (!sd_mutex || !xSemaphoreTake(sd_mutex, pdMS_TO_TICKS(1000))) {
#ifdef DEBUG_MODE
      Serial.println("[TERRAIN] SD mutex timeout");
#endif
      return false;
    }

    File file = SD_MMC.open(filename.c_str(), FILE_READ);
    if (!file) {
#ifdef DEBUG_MODE
      Serial.printf("[TERRAIN] Cannot open: %s\n", filename.c_str());
#endif
      xSemaphoreGive(sd_mutex);
      return false;
    }

    size_t fileSize = file.size();
    file.close();
    xSemaphoreGive(sd_mutex);

    if (fileSize == 1201 * 1201 * sizeof(int16_t)) {
      gridSize = HGT_SRTM3_SIZE;
#ifdef DEBUG_MODE
      Serial.printf("[TERRAIN] SRTM-3 (90m): %s\n", filename.c_str());
#endif
    } else if (fileSize == 3601 * 3601 * sizeof(int16_t)) {
      gridSize = HGT_SRTM1_SIZE;
#ifdef DEBUG_MODE
      Serial.printf("[TERRAIN] SRTM-1 (30m): %s\n", filename.c_str());
#endif
    } else {
#ifdef DEBUG_MODE
      Serial.printf("[TERRAIN] Invalid size: %d\n", fileSize);
#endif
      return false;
    }

    // Invalider cache si changement tile
    if (cachedFilename != filename) {
      cacheValid = false;
    }

    cachedFilename = filename;
    tileLoaded = true;

    return true;
  }

  // Charge zone carrée autour position
  bool loadCacheAround(float lat, float lon) {
    // Vérifier si position encore dans cache valide
    if (cacheValid) {
      float dist = distanceMeters(lat, lon, cacheCenterLat, cacheCenterLon);
      if (dist < CACHE_RADIUS_M * CACHE_REFRESH_RATIO) {  // Changé de 0.6 à 0.5
        return true;
      }
#ifdef DEBUG_MODE
      Serial.printf("[TERRAIN] Cache refresh (%.0fm from center)\n", dist);
#endif
    }

    int baseLat = (int)floor(lat);
    int baseLon = (int)floor(lon);

    float fracLat = lat - baseLat;
    float fracLon = lon - baseLon;

    // Position pixel centre
    float centerRow = (1.0 - fracLat) * (gridSize - 1);
    float centerCol = fracLon * (gridSize - 1);

    // Calcul taille cache en pixels selon résolution
    float pixelSizeM = (gridSize == HGT_SRTM1_SIZE) ? 30.0 : 90.0;
    int halfSize = (int)(CACHE_RADIUS_M / pixelSizeM);

    // Limiter taille max
    if (halfSize > CACHE_MAX_SIZE / 2) {
      halfSize = CACHE_MAX_SIZE / 2;
    }

    cacheSize = halfSize * 2 + 1;
    cacheStartRow = (int)centerRow - halfSize;
    cacheStartCol = (int)centerCol - halfSize;

    // Clip aux limites tile
    if (cacheStartRow < 0) cacheStartRow = 0;
    if (cacheStartCol < 0) cacheStartCol = 0;
    if (cacheStartRow + cacheSize >= gridSize) {
      cacheStartRow = gridSize - cacheSize;
    }
    if (cacheStartCol + cacheSize >= gridSize) {
      cacheStartCol = gridSize - cacheSize;
    }

#ifdef DEBUG_MODE
    Serial.printf("[TERRAIN] Loading cache: %dx%d pixels (%.0fm radius)\n",
                  cacheSize, cacheSize, halfSize * pixelSizeM);
#endif

    extern SemaphoreHandle_t sd_mutex;
    if (!sd_mutex || !xSemaphoreTake(sd_mutex, pdMS_TO_TICKS(2000))) {
#ifdef DEBUG_MODE
      Serial.println("[TERRAIN] SD mutex timeout");
#endif
      return false;
    }

    File file = SD_MMC.open(cachedFilename.c_str(), FILE_READ);
    if (!file) {
#ifdef DEBUG_MODE
      Serial.printf("[TERRAIN] Cannot reopen: %s\n", cachedFilename.c_str());
#endif
      xSemaphoreGive(sd_mutex);
      return false;
    }

    // Lecture ligne par ligne de la zone
    uint8_t lineBuffer[CACHE_MAX_SIZE * 2];

    for (int r = 0; r < cacheSize; r++) {
      int fileRow = cacheStartRow + r;
      size_t offset = fileRow * gridSize * sizeof(int16_t) + cacheStartCol * sizeof(int16_t);

      file.seek(offset);
      size_t bytesToRead = cacheSize * sizeof(int16_t);
      size_t bytesRead = file.read(lineBuffer, bytesToRead);

      if (bytesRead != bytesToRead) {
#ifdef DEBUG_MODE
        Serial.printf("[TERRAIN] Read error row %d\n", r);
#endif
        file.close();
        xSemaphoreGive(sd_mutex);
        return false;
      }

      // Conversion big-endian
      for (int c = 0; c < cacheSize; c++) {
        uint8_t high = lineBuffer[c * 2];
        uint8_t low = lineBuffer[c * 2 + 1];
        cacheData[r * CACHE_MAX_SIZE + c] = (int16_t)((high << 8) | low);
      }
    }

    file.close();
    xSemaphoreGive(sd_mutex);

    cacheCenterLat = lat;
    cacheCenterLon = lon;
    cacheValid = true;

#ifdef DEBUG_MODE
    Serial.printf("[TERRAIN] Cache loaded at %.6f,%.6f\n", lat, lon);
#endif

    return true;
  }

  // Lit altitude depuis cache
  int16_t getAltitudeFromCache(int row, int col) {
    int cacheRow = row - cacheStartRow;
    int cacheCol = col - cacheStartCol;

    if (cacheRow < 0 || cacheRow >= cacheSize || cacheCol < 0 || cacheCol >= cacheSize) {
      return HGT_NO_DATA;
    }

    return cacheData[cacheRow * CACHE_MAX_SIZE + cacheCol];
  }

public:
  TerrainElevation() {
    cachedFilename = "";
    gridSize = 0;
    tileLoaded = false;
    cacheData = nullptr;
    cacheSize = 0;
    cacheValid = false;
    cache_mutex = xSemaphoreCreateMutex();
  }

  ~TerrainElevation() {
    if (cacheData) {
      free(cacheData);
    }
    if (cache_mutex) {
      vSemaphoreDelete(cache_mutex);
    }
  }

  bool begin() {
    // Allouer cache carré max
    cacheData = (int16_t*)malloc(CACHE_MAX_SIZE * CACHE_MAX_SIZE * sizeof(int16_t));

    if (!cacheData) {
#ifdef DEBUG_MODE
      Serial.println("[TERRAIN] Cache allocation failed");
#endif
      return false;
    }

#ifdef DEBUG_MODE
    size_t cacheBytes = CACHE_MAX_SIZE * CACHE_MAX_SIZE * sizeof(int16_t);
    Serial.printf("[TERRAIN] Init OK (%.1f KB cache)\n", cacheBytes / 1024.0);
#endif
    return true;
  }

  // Obtient altitude avec IDW carré
  float getElevation(float lat, float lon) {
    String path = getHGTPath(lat, lon);

    if (!openTile(path)) {
      return NAN;
    }

    xSemaphoreTake(cache_mutex, portMAX_DELAY);

    // Charger cache si nécessaire
    if (!loadCacheAround(lat, lon)) {
      xSemaphoreGive(cache_mutex);
      return NAN;
    }

    int baseLat = (int)floor(lat);
    int baseLon = (int)floor(lon);

    float fracLat = lat - baseLat;
    float fracLon = lon - baseLon;

    float pixelRow = (1.0 - fracLat) * (gridSize - 1);
    float pixelCol = fracLon * (gridSize - 1);

    int row0 = (int)floor(pixelRow);
    int row1 = row0 + 1;
    int col0 = (int)floor(pixelCol);
    int col1 = col0 + 1;

    // Lecture 4 altitudes depuis cache
    int16_t alt_NW = getAltitudeFromCache(row0, col0);
    int16_t alt_NE = getAltitudeFromCache(row0, col1);
    int16_t alt_SW = getAltitudeFromCache(row1, col0);
    int16_t alt_SE = getAltitudeFromCache(row1, col1);

    xSemaphoreGive(cache_mutex);

    if (alt_NW == HGT_NO_DATA || alt_NE == HGT_NO_DATA || alt_SW == HGT_NO_DATA || alt_SE == HGT_NO_DATA) {
      return NAN;
    }

    // IDW carré
    float dRow = pixelRow - row0;
    float dCol = pixelCol - col0;

    float d2_NW = dRow * dRow + dCol * dCol;
    float d2_NE = dRow * dRow + (1.0 - dCol) * (1.0 - dCol);
    float d2_SW = (1.0 - dRow) * (1.0 - dRow) + dCol * dCol;
    float d2_SE = (1.0 - dRow) * (1.0 - dRow) + (1.0 - dCol) * (1.0 - dCol);

    const float epsilon = 1e-6;
    d2_NW = (d2_NW < epsilon) ? epsilon : d2_NW;
    d2_NE = (d2_NE < epsilon) ? epsilon : d2_NE;
    d2_SW = (d2_SW < epsilon) ? epsilon : d2_SW;
    d2_SE = (d2_SE < epsilon) ? epsilon : d2_SE;

    float w_NW = 1.0 / d2_NW;
    float w_NE = 1.0 / d2_NE;
    float w_SW = 1.0 / d2_SW;
    float w_SE = 1.0 / d2_SE;

    float sum_weights = w_NW + w_NE + w_SW + w_SE;

    float altitude = (alt_NW * w_NW + alt_NE * w_NE + alt_SW * w_SW + alt_SE * w_SE) / sum_weights;

    return altitude;
  }

  String getCachedTile() {
    return cachedFilename;
  }

  int getGridSize() {
    return gridSize;
  }
};

static TerrainElevation terrain;

#endif