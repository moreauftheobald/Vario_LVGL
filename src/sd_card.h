#ifndef SD_CARD_H
#define SD_CARD_H

#include <FS.h>
#include <SD_MMC.h>
#include "io_extension/io_extension.h"
#include "constants.h"

// Variables globales
static bool sd_card_ready = false;

// ===== INITIALISATION =====
bool sd_init() {
    #ifdef DEBUG_MODE
    Serial.println("SD: Initialisation...");
    #endif
    
    // Configurer les pins SDMMC
    if (!SD_MMC.setPins(SD_PIN_CLK, SD_PIN_CMD, SD_PIN_D0)) {
        #ifdef DEBUG_MODE
        Serial.println("SD: Echec configuration pins");
        #endif
        return false;
    }
    
    #ifdef DEBUG_MODE
    Serial.println("SD: Pins configurees");
    #endif
    
    delay(50);
    
    #ifdef DEBUG_MODE
    if (SD_FORMAT_IF_MOUNT_FAILED) {
        Serial.println("SD: Mode formatage automatique active");
    }
    #endif
    
    // Monter le systeme de fichiers (mode 1-bit)
    // Le 3eme parametre force le formatage en FAT32 si le montage echoue
    if (!SD_MMC.begin(SD_MOUNT_POINT, true, SD_FORMAT_IF_MOUNT_FAILED)) {
        #ifdef DEBUG_MODE
        Serial.println("SD: Echec montage");
        if (SD_FORMAT_IF_MOUNT_FAILED) {
            Serial.println("SD: Echec meme apres tentative de formatage");
            Serial.println("SD: Verifier que la carte est bien inseree");
        }
        #endif
        sd_card_ready = false;
        return false;
    }
    
    #ifdef DEBUG_MODE
    if (SD_FORMAT_IF_MOUNT_FAILED) {
        Serial.println("SD: Montage OK (formatage auto si necessaire)");
    } else {
        Serial.println("SD: Montage OK");
    }
    #endif
    
    // Verifier le type de carte
    uint8_t cardType = SD_MMC.cardType();
    if (cardType == CARD_NONE) {
        #ifdef DEBUG_MODE
        Serial.println("SD: Aucune carte detectee");
        #endif
        sd_card_ready = false;
        return false;
    }
    
    #ifdef DEBUG_MODE
    Serial.print("SD: Type carte: ");
    if (cardType == CARD_MMC) {
        Serial.println("MMC");
    } else if (cardType == CARD_SD) {
        Serial.println("SDSC");
    } else if (cardType == CARD_SDHC) {
        Serial.println("SDHC");
    } else {
        Serial.println("UNKNOWN");
    }
    Serial.printf("SD: Taille: %llu MB\n", SD_MMC.cardSize() / (1024 * 1024));
    Serial.printf("SD: Utilise: %llu MB\n", SD_MMC.usedBytes() / (1024 * 1024));
    #endif
    
    // Creer les repertoires necessaires
    if (!SD_MMC.exists(OSM_TILES_DIR)) {
        SD_MMC.mkdir(OSM_TILES_DIR);
    }
    if (!SD_MMC.exists(LOGS_DIR)) {
        SD_MMC.mkdir(LOGS_DIR);
    }
    if (!SD_MMC.exists(FLIGHTS_DIR)) {
        SD_MMC.mkdir(FLIGHTS_DIR);
    }
    if (!SD_MMC.exists(CONFIG_DIR)) {
        SD_MMC.mkdir(CONFIG_DIR);
    }
    
    sd_card_ready = true;
    #ifdef DEBUG_MODE
    Serial.println("SD: Pret");
    #endif
    return true;
}

// ===== DEMONTAGE =====
void sd_unmount() {
    if (!sd_card_ready) return;
    
    #ifdef DEBUG_MODE
    Serial.println("SD: Demontage...");
    #endif
    
    SD_MMC.end();
    sd_card_ready = false;
}

// ===== VERIFICATIONS =====
bool sd_is_ready() {
    return sd_card_ready;
}

bool sd_file_exists(const char* path) {
    if (!sd_card_ready) return false;
    return SD_MMC.exists(path);
}

size_t sd_file_size(const char* path) {
    if (!sd_card_ready) return 0;
    File file = SD_MMC.open(path, FILE_READ);
    if (!file) return 0;
    size_t size = file.size();
    file.close();
    return size;
}

// ===== LECTURE =====
bool sd_read_file(const char* path, uint8_t* buffer, size_t* size) {
    if (!sd_card_ready) return false;
    
    File file = SD_MMC.open(path, FILE_READ);
    if (!file) {
        #ifdef DEBUG_MODE
        Serial.printf("SD: Echec ouverture lecture: %s\n", path);
        #endif
        return false;
    }
    
    *size = file.read(buffer, *size);
    file.close();
    
    #ifdef DEBUG_MODE
    Serial.printf("SD: Lu %d octets de %s\n", *size, path);
    #endif
    return true;
}

String sd_read_file_string(const char* path) {
    if (!sd_card_ready) return "";
    
    File file = SD_MMC.open(path, FILE_READ);
    if (!file) return "";
    
    String content = file.readString();
    file.close();
    return content;
}

// ===== ECRITURE =====
bool sd_write_file(const char* path, const uint8_t* data, size_t size) {
    if (!sd_card_ready) return false;
    
    File file = SD_MMC.open(path, FILE_WRITE);
    if (!file) {
        #ifdef DEBUG_MODE
        Serial.printf("SD: Echec ouverture ecriture: %s\n", path);
        #endif
        return false;
    }
    
    size_t written = file.write(data, size);
    file.close();
    
    #ifdef DEBUG_MODE
    Serial.printf("SD: Ecrit %d octets dans %s\n", written, path);
    #endif
    return (written == size);
}

bool sd_append_file(const char* path, const uint8_t* data, size_t size) {
    if (!sd_card_ready) return false;
    
    File file = SD_MMC.open(path, FILE_APPEND);
    if (!file) return false;
    
    size_t written = file.write(data, size);
    file.close();
    return (written == size);
}

// ===== SUPPRESSION =====
bool sd_delete_file(const char* path) {
    if (!sd_card_ready) return false;
    
    #ifdef DEBUG_MODE
    Serial.printf("SD: Suppression: %s\n", path);
    #endif
    return SD_MMC.remove(path);
}

bool sd_delete_dir(const char* path) {
    if (!sd_card_ready) return false;
    
    #ifdef DEBUG_MODE
    Serial.printf("SD: Suppression repertoire: %s\n", path);
    #endif
    return SD_MMC.rmdir(path);
}

// ===== REPERTOIRES =====
bool sd_create_dir(const char* path) {
    if (!sd_card_ready) return false;
    
    if (SD_MMC.exists(path)) return true;
    
    #ifdef DEBUG_MODE
    Serial.printf("SD: Creation repertoire: %s\n", path);
    #endif
    return SD_MMC.mkdir(path);
}

// ===== INFOS SYSTEME =====
uint64_t sd_get_total_bytes() {
    if (!sd_card_ready) return 0;
    return SD_MMC.cardSize();
}

uint64_t sd_get_used_bytes() {
    if (!sd_card_ready) return 0;
    return SD_MMC.usedBytes();
}

uint64_t sd_get_free_bytes() {
    if (!sd_card_ready) return 0;
    return SD_MMC.cardSize() - SD_MMC.usedBytes();
}

// ===== CAPACITE =====
bool sd_get_capacity(uint64_t *total_kb, uint64_t *available_kb) {
    if (!sd_card_ready) return false;
    if (total_kb == NULL || available_kb == NULL) return false;
    
    *total_kb = SD_MMC.cardSize() / 1024;
    *available_kb = *total_kb - (SD_MMC.usedBytes() / 1024);
    
    return true;
}

// ===== COMPTAGE FICHIERS =====
/**
 * @brief Compte recursivement les fichiers dans un repertoire
 * @param dirPath Chemin du repertoire
 * @param extension Extension a filtrer (NULL pour tous les fichiers)
 * @return Nombre de fichiers
 */
int sd_count_files_recursive(const char* dirPath, const char* extension) {
    if (!sd_card_ready) return 0;
    
    #ifdef DEBUG_MODE
    Serial.printf("SD: Comptage dans %s\n", dirPath);
    #endif
    
    File dir = SD_MMC.open(dirPath);
    if (!dir || !dir.isDirectory()) {
        if (dir) dir.close();
        #ifdef DEBUG_MODE
        Serial.printf("SD: %s n'est pas un repertoire\n", dirPath);
        #endif
        return 0;
    }
    
    int count = 0;
    File file = dir.openNextFile();
    
    while (file) {
        if (file.isDirectory()) {
            // Comptage recursif dans les sous-repertoires
            char subPath[256];
            snprintf(subPath, sizeof(subPath), "%s/%s", dirPath, file.name());
            count += sd_count_files_recursive(subPath, extension);
        } else {
            // Compter le fichier si pas d'extension ou si l'extension correspond
            if (extension == NULL) {
                count++;
            } else {
                const char* fileName = file.name();
                size_t nameLen = strlen(fileName);
                size_t extLen = strlen(extension);
                
                if (nameLen >= extLen && 
                    strcasecmp(fileName + nameLen - extLen, extension) == 0) {
                    count++;
                }
            }
        }
        file.close();
        file = dir.openNextFile();
    }
    
    dir.close();
    
    #ifdef DEBUG_MODE
    Serial.printf("SD: %d fichiers trouves dans %s\n", count, dirPath);
    #endif
    
    return count;
}

/**
 * @brief Compte les fichiers de tuiles OSM
 * @return Nombre de fichiers
 */
int sd_count_osm_tiles() {
    #ifdef DEBUG_MODE
    Serial.println("SD: Debut comptage OSM tiles");
    #endif
    
    int count = sd_count_files_recursive(OSM_TILES_DIR, NULL);
    
    #ifdef DEBUG_MODE
    Serial.printf("SD: Fin comptage OSM tiles: %d\n", count);
    #endif
    
    return count;
}

/**
 * @brief Compte les fichiers IGC (vols)
 * @return Nombre de fichiers .igc
 */
int sd_count_igc_files() {
    #ifdef DEBUG_MODE
    Serial.println("SD: Debut comptage fichiers IGC");
    #endif
    
    int count = sd_count_files_recursive(FLIGHTS_DIR, ".igc");
    
    #ifdef DEBUG_MODE
    Serial.printf("SD: Fin comptage IGC: %d\n", count);
    #endif
    
    return count;
}

#endif // SD_CARD_H