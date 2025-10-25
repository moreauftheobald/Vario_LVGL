#ifndef OSM_TILE_LOADER_H
#define OSM_TILE_LOADER_H

#include <Arduino.h>
#include <SD_MMC.h>
#include <math.h>
#include "lvgl.h"
#include "constants.h"

// Taille standard tuile OSM
#define OSM_TILE_SIZE 256

// Position test: Décollage parapente Volmerange-les-Mines
#define TEST_LAT 49.446745
#define TEST_LON 6.099718

// Nom du serveur de cartes
#define OSM_SERVER_NAME "osm"

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

// Charger une tuile depuis SD
static bool load_single_tile(int zoom, int tile_x, int tile_y, uint16_t* buffer) {
    char tile_path[128];
    snprintf(tile_path, sizeof(tile_path), "%s/%s/%d/%d/%d.bin", 
             OSM_TILES_DIR, OSM_SERVER_NAME, zoom, tile_x, tile_y);
    
    if(!SD_MMC.exists(tile_path)) {
        #ifdef DEBUG_MODE
        Serial.printf("[OSM] Tile not found: %s\n", tile_path);
        #endif
        return false;
    }
    
    File tile_file = SD_MMC.open(tile_path, FILE_READ);
    if(!tile_file) {
        return false;
    }
    
    size_t expected_size = OSM_TILE_SIZE * OSM_TILE_SIZE * 2;
    if(tile_file.size() != expected_size) {
        tile_file.close();
        return false;
    }
    
    size_t bytes_read = tile_file.read((uint8_t*)buffer, expected_size);
    tile_file.close();
    
    if(bytes_read != expected_size) {
        return false;
    }
    
    // Swap octets RGB565
    for(int i = 0; i < OSM_TILE_SIZE * OSM_TILE_SIZE; i++) {
        uint16_t pixel = buffer[i];
        buffer[i] = (pixel >> 8) | (pixel << 8);
    }
    
    return true;
}

// Créer un canvas multi-tuiles pour remplir un cadre donné
static lv_obj_t* create_map_view(lv_obj_t* parent, double lat, double lon, int zoom,
                                 int view_width, int view_height) {
    #ifdef DEBUG_MODE
    Serial.printf("[OSM] Creating map view: %dx%d at zoom %d\n", 
                  view_width, view_height, zoom);
    #endif
    
    // Calculer position centrale
    int center_tile_x, center_tile_y;
    double center_pixel_x, center_pixel_y;
    lat_lon_to_tile_pixel(lat, lon, zoom, 
                         &center_tile_x, &center_tile_y,
                         &center_pixel_x, &center_pixel_y);
    
    #ifdef DEBUG_MODE
    Serial.printf("[OSM] Center tile: %d,%d  Pixel in tile: %.1f,%.1f\n",
                  center_tile_x, center_tile_y, center_pixel_x, center_pixel_y);
    #endif
    
    // Allouer buffer pour le canvas final
    static uint16_t* view_buffer = NULL;
    if(view_buffer != NULL) {
        heap_caps_free(view_buffer);
    }
    
    view_buffer = (uint16_t*)heap_caps_malloc(
        view_width * view_height * sizeof(uint16_t),
        MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT
    );
    
    if(!view_buffer) {
        #ifdef DEBUG_MODE
        Serial.println("[OSM] Cannot allocate view buffer");
        #endif
        return NULL;
    }
    
    // Remplir de gris par défaut
    for(int i = 0; i < view_width * view_height; i++) {
        view_buffer[i] = 0x7BEF;
    }
    
    // Buffer temporaire pour une tuile
    uint16_t* tile_buffer = (uint16_t*)heap_caps_malloc(
        OSM_TILE_SIZE * OSM_TILE_SIZE * sizeof(uint16_t),
        MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT
    );
    
    if(!tile_buffer) {
        #ifdef DEBUG_MODE
        Serial.println("[OSM] Cannot allocate tile buffer");
        #endif
        heap_caps_free(view_buffer);
        return NULL;
    }
    
    // Calculer combien de tuiles charger (3x3 centré)
    int start_x = center_tile_x - 1;
    int start_y = center_tile_y - 1;
    
    // Position de départ pour dessiner (centrer le point dans la vue)
    int start_pixel_x = (view_width / 2) - (int)center_pixel_x - OSM_TILE_SIZE;
    int start_pixel_y = (view_height / 2) - (int)center_pixel_y - OSM_TILE_SIZE;
    
    #ifdef DEBUG_MODE
    Serial.printf("[OSM] Drawing start: %d,%d\n", start_pixel_x, start_pixel_y);
    #endif
    
    // Charger et copier les 9 tuiles (3x3)
    for(int dy = 0; dy < 3; dy++) {
        for(int dx = 0; dx < 3; dx++) {
            int tile_x = start_x + dx;
            int tile_y = start_y + dy;
            
            // Position où dessiner cette tuile dans le canvas
            int draw_x = start_pixel_x + (dx * OSM_TILE_SIZE);
            int draw_y = start_pixel_y + (dy * OSM_TILE_SIZE);
            
            // Charger la tuile
            bool tile_loaded = load_single_tile(zoom, tile_x, tile_y, tile_buffer);
            
            if(!tile_loaded) {
                // Remplir de gris si tuile manquante
                for(int i = 0; i < OSM_TILE_SIZE * OSM_TILE_SIZE; i++) {
                    tile_buffer[i] = 0x7BEF;
                }
            }
            
            // Copier la portion visible de la tuile dans le canvas
            for(int ty = 0; ty < OSM_TILE_SIZE; ty++) {
                for(int tx = 0; tx < OSM_TILE_SIZE; tx++) {
                    int dest_x = draw_x + tx;
                    int dest_y = draw_y + ty;
                    
                    // Vérifier si dans les limites du canvas
                    if(dest_x >= 0 && dest_x < view_width &&
                       dest_y >= 0 && dest_y < view_height) {
                        view_buffer[dest_y * view_width + dest_x] = 
                            tile_buffer[ty * OSM_TILE_SIZE + tx];
                    }
                }
            }
            
            #ifdef DEBUG_MODE
            if(tile_loaded) {
                Serial.printf("[OSM] Tile %d,%d loaded and drawn at %d,%d\n",
                              tile_x, tile_y, draw_x, draw_y);
            }
            #endif
        }
    }
    
    heap_caps_free(tile_buffer);
    
    // Créer canvas LVGL
    lv_obj_t* canvas = lv_canvas_create(parent);
    lv_canvas_set_buffer(canvas, view_buffer, 
                        view_width, view_height,
                        LV_COLOR_FORMAT_RGB565);
    
    #ifdef DEBUG_MODE
    Serial.println("[OSM] Map view created successfully");
    #endif
    
    return canvas;
}

#endif