#ifndef TEST_LOGGER_TASK_H
#define TEST_LOGGER_TASK_H

#include <Arduino.h>
#include "constants.h"
#include "globals.h"
#include "kalman_task.h"
#include "FS.h"
#include "SD_MMC.h"
#include "src/sd_card.h"

#ifdef TEST_MODE

static TaskHandle_t test_logger_task_handle = NULL;
static File test_log_file;
static bool test_logging_active = false;
static SemaphoreHandle_t sd_mutex = NULL;

// Buffer pour calcul vario brut baro
#define VARIO_BUFFER_SIZE 5
static float alt_buffer[VARIO_BUFFER_SIZE];
static uint32_t time_buffer[VARIO_BUFFER_SIZE];
static int buffer_idx = 0;
static bool buffer_full = false;

// Calcul accel Z monde
static float calculate_bno_accel_z(const bno080_data_t* imu) {
    float qw = imu->quat_real;
    float qx = imu->quat_i;
    float qy = imu->quat_j;
    float qz = imu->quat_k;
    
    float ax = imu->accel_x;
    float ay = imu->accel_y;
    float az = imu->accel_z;
    
    float az_world = ax * (2.0f * qx * qz - 2.0f * qw * qy) +
                     ay * (2.0f * qy * qz + 2.0f * qw * qx) +
                     az * (qw*qw - qx*qx - qy*qy + qz*qz);
    
    return az_world;
}

// Calcul vario brut baro
static float calculate_raw_vario(float current_alt, uint32_t current_time) {
    alt_buffer[buffer_idx] = current_alt;
    time_buffer[buffer_idx] = current_time;
    buffer_idx = (buffer_idx + 1) % VARIO_BUFFER_SIZE;
    
    if(!buffer_full && buffer_idx == 0) {
        buffer_full = true;
    }
    
    if(!buffer_full) return 0.0f;
    
    int oldest_idx = buffer_idx;
    float sum_t = 0, sum_alt = 0, sum_t_alt = 0, sum_t2 = 0;
    
    for(int i = 0; i < VARIO_BUFFER_SIZE; i++) {
        float t = (time_buffer[i] - time_buffer[oldest_idx]) / 1000.0f;
        float alt = alt_buffer[i];
        sum_t += t;
        sum_alt += alt;
        sum_t_alt += t * alt;
        sum_t2 += t * t;
    }
    
    float vario = (VARIO_BUFFER_SIZE * sum_t_alt - sum_t * sum_alt) /
                  (VARIO_BUFFER_SIZE * sum_t2 - sum_t * sum_t);
    
    return vario;
}

// Creation fichier
static bool test_logger_create_file() {
    if(sd_mutex == NULL) {
        sd_mutex = xSemaphoreCreateMutex();
    }
    
    if(!xSemaphoreTake(sd_mutex, pdMS_TO_TICKS(1000))) {
        #ifdef DEBUG_MODE
        Serial.println("[TEST_LOG] Cannot take SD mutex");
        #endif
        return false;
    }
    
    // Verifier SD ready
    if(!sd_is_ready()) {
        #ifdef DEBUG_MODE
        Serial.println("[TEST_LOG] SD not ready");
        #endif
        xSemaphoreGive(sd_mutex);
        return false;
    }
    
    #ifdef DEBUG_MODE
    Serial.printf("[TEST_LOG] SD OK - Total: %lluMB, Free: %lluMB\n", 
                  sd_get_total_bytes() / (1024*1024),
                  sd_get_free_bytes() / (1024*1024));
    #endif
    
    // Utiliser les fonctions sd_card.h qui gerent correctement les chemins
    const char* flights_dir = FLIGHTS_DIR;
    
    #ifdef DEBUG_MODE
    Serial.printf("[TEST_LOG] Checking dir: %s\n", flights_dir);
    Serial.printf("[TEST_LOG] Dir exists: %d\n", SD_MMC.exists(flights_dir));
    #endif
    
    // Le repertoire est normalement deja cree par sd_init()
    if(!SD_MMC.exists(flights_dir)) {
        #ifdef DEBUG_MODE
        Serial.println("[TEST_LOG] Creating flights dir...");
        #endif
        if(!SD_MMC.mkdir(flights_dir)) {
            #ifdef DEBUG_MODE
            Serial.println("[TEST_LOG] Cannot create flights dir");
            #endif
            xSemaphoreGive(sd_mutex);
            return false;
        }
    }
    
    char filename[80];
    
    if(g_sensor_data.gps.valid && g_sensor_data.gps.fix) {
        // CORRIGÉ: Construire le chemin en 2 étapes
        char file_only[60];
        snprintf(file_only, sizeof(file_only), 
                 "test_%04d%02d%02d_%02d%02d%02d.csv",
                 g_sensor_data.gps.year + 2000,
                 g_sensor_data.gps.month,
                 g_sensor_data.gps.day,
                 g_sensor_data.gps.hour,
                 g_sensor_data.gps.minute,
                 g_sensor_data.gps.seconds);
        snprintf(filename, sizeof(filename), "%s/%s", FLIGHTS_DIR, file_only);
    } else {
        // CORRIGÉ: Construire le chemin en 2 étapes
        char file_only[40];
        snprintf(file_only, sizeof(file_only), "test_%lu.csv", millis());
        snprintf(filename, sizeof(filename), "%s/%s", FLIGHTS_DIR, file_only);
    }
    
    #ifdef DEBUG_MODE
    Serial.printf("[TEST_LOG] Creating file: %s\n", filename);
    #endif
    
    test_log_file = SD_MMC.open(filename, FILE_WRITE);
    if(!test_log_file) {
        #ifdef DEBUG_MODE
        Serial.printf("[TEST_LOG] Cannot create file: %s\n", filename);
        #endif
        xSemaphoreGive(sd_mutex);
        return false;
    }
    
    // Header CSV
    test_log_file.print("Timestamp_ms,Date,Time,");
    test_log_file.print("Pressure_hPa,Temp_C,Pressure_Alt_m,Vario_Raw_Baro_ms,Alt_QNE_m,");
    test_log_file.print("BNO_Quat_W,BNO_Quat_X,BNO_Quat_Y,BNO_Quat_Z,");
    test_log_file.print("BNO_Accel_X_ms2,BNO_Accel_Y_ms2,BNO_Accel_Z_ms2,BNO_Accel_Z_World_ms2,");
    test_log_file.print("GPS_Longitude,GPS_Latitude,GPS_Alt_m,GPS_Speed_knots,GPS_Course_deg,GPS_Satellites,GPS_FixQuality,");
    test_log_file.print("Kalman_Alt_m,Kalman_Vario_ms,Kalman_Accel_ms2,");
    test_log_file.print("Kalman_P00,Kalman_P11,Kalman_P22,");
    test_log_file.println("Valid_BMP,Valid_BNO,Valid_GPS");
    
    test_log_file.flush();
    
    #ifdef DEBUG_MODE
    Serial.printf("[TEST_LOG] File created successfully: %s\n", filename);
    #endif
    
    xSemaphoreGive(sd_mutex);
    return true;
}

// Ecriture ligne
static void test_logger_write_line() {
    if(!test_log_file) return;
    
    if(!xSemaphoreTake(sd_mutex, pdMS_TO_TICKS(100))) {
        return;
    }
    
    uint32_t now = millis();
    
    // Kalman
    kalman_data_t kdata;
    kalman_get_data(&kdata);
    
    // BMP390
    float pressure_hpa = g_sensor_data.bmp390.valid ? 
                         g_sensor_data.bmp390.pressure / 100.0f : 0.0f;
    float temp_c = g_sensor_data.bmp390.valid ? 
                   g_sensor_data.bmp390.temperature : 0.0f;
    float pressure_alt = pressure_hpa > 0 ? 
                         44330.0f * (1.0f - pow(pressure_hpa / 1013.25f, 0.1903f)) : 0.0f;
    float alt_qne = pressure_alt;
    float vario_raw = calculate_raw_vario(pressure_alt, now);
    
    // BNO080
    float qw = g_sensor_data.bno080.valid ? g_sensor_data.bno080.quat_real : 0.0f;
    float qx = g_sensor_data.bno080.valid ? g_sensor_data.bno080.quat_i : 0.0f;
    float qy = g_sensor_data.bno080.valid ? g_sensor_data.bno080.quat_j : 0.0f;
    float qz = g_sensor_data.bno080.valid ? g_sensor_data.bno080.quat_k : 0.0f;
    float ax = g_sensor_data.bno080.valid ? g_sensor_data.bno080.accel_x : 0.0f;
    float ay = g_sensor_data.bno080.valid ? g_sensor_data.bno080.accel_y : 0.0f;
    float az = g_sensor_data.bno080.valid ? g_sensor_data.bno080.accel_z : 0.0f;
    float az_world = g_sensor_data.bno080.valid ? 
                     calculate_bno_accel_z(&g_sensor_data.bno080) : 0.0f;
    
    // GPS
    char date_str[16] = "00/00/0000";
    char time_str[16] = "00:00:00";
    
    if(g_sensor_data.gps.valid) {
        snprintf(date_str, sizeof(date_str), "%02d/%02d/%04d",
                 g_sensor_data.gps.day,
                 g_sensor_data.gps.month,
                 g_sensor_data.gps.year + 2000);
        snprintf(time_str, sizeof(time_str), "%02d:%02d:%02d",
                 g_sensor_data.gps.hour,
                 g_sensor_data.gps.minute,
                 g_sensor_data.gps.seconds);
    }
    
    float gps_lon = g_sensor_data.gps.valid ? g_sensor_data.gps.longitude : 0.0f;
    float gps_lat = g_sensor_data.gps.valid ? g_sensor_data.gps.latitude : 0.0f;
    float gps_alt = g_sensor_data.gps.valid ? g_sensor_data.gps.altitude : 0.0f;
    float gps_speed = g_sensor_data.gps.valid ? g_sensor_data.gps.speed : 0.0f;
    float gps_course = g_sensor_data.gps.valid ? g_sensor_data.gps.angle : 0.0f;
    int gps_sat = g_sensor_data.gps.valid ? g_sensor_data.gps.satellites : 0;
    int gps_fix = g_sensor_data.gps.valid ? g_sensor_data.gps.fixquality : 0;
    
    // Covariances Kalman
    extern KalmanFilter_t kf;
    float p00 = kf.P[0][0];
    float p11 = kf.P[1][1];
    float p22 = kf.P[2][2];
    
    // Ecriture CSV
    test_log_file.printf("%lu,%s,%s,", now, date_str, time_str);
    test_log_file.printf("%.2f,%.2f,%.2f,%.3f,%.2f,",
                         pressure_hpa, temp_c, pressure_alt, vario_raw, alt_qne);
    test_log_file.printf("%.4f,%.4f,%.4f,%.4f,", qw, qx, qy, qz);
    test_log_file.printf("%.4f,%.4f,%.4f,%.4f,", ax, ay, az, az_world);
    test_log_file.printf("%.6f,%.6f,%.2f,%.2f,%.2f,%d,%d,",
                         gps_lon, gps_lat, gps_alt, gps_speed, gps_course, gps_sat, gps_fix);
    test_log_file.printf("%.2f,%.3f,%.4f,", kdata.altitude, kdata.vario, kdata.altitude);
    test_log_file.printf("%.4f,%.4f,%.4f,", p00, p11, p22);
    test_log_file.printf("%d,%d,%d\n",
                         g_sensor_data.bmp390.valid ? 1 : 0,
                         g_sensor_data.bno080.valid ? 1 : 0,
                         g_sensor_data.gps.valid ? 1 : 0);
    
    test_log_file.flush();
    
    xSemaphoreGive(sd_mutex);
}

// Fermeture
static void test_logger_close_file() {
    if(test_log_file) {
        if(xSemaphoreTake(sd_mutex, pdMS_TO_TICKS(1000))) {
            test_log_file.close();
            xSemaphoreGive(sd_mutex);
            
            #ifdef DEBUG_MODE
            Serial.println("[TEST_LOG] File closed");
            #endif
        }
    }
}

// Tache
static void test_logger_task(void* parameter) {
    #ifdef DEBUG_MODE
    Serial.println("[TEST_LOG] Task started, waiting for mainscreen...");
    #endif
    
    // Attendre mainscreen
    uint32_t wait_count = 0;
    while(!mainscreen_active) {
        vTaskDelay(pdMS_TO_TICKS(100));
        wait_count++;
        #ifdef DEBUG_MODE
        if(wait_count % 50 == 0) {
            Serial.println("[TEST_LOG] Still waiting for mainscreen...");
        }
        #endif
    }
    
    #ifdef DEBUG_MODE
    Serial.println("[TEST_LOG] Mainscreen active, waiting 2s...");
    #endif
    
    vTaskDelay(pdMS_TO_TICKS(2000));
    
    if(!test_logger_create_file()) {
        #ifdef DEBUG_MODE
        Serial.println("[TEST_LOG] Failed to create file");
        #endif
        vTaskDelete(NULL);
        return;
    }
    
    test_logging_active = true;
    
    #ifdef DEBUG_MODE
    Serial.println("[TEST_LOG] Recording started");
    #endif
    
    TickType_t last_wake = xTaskGetTickCount();
    const TickType_t period = pdMS_TO_TICKS(1000);
    
    uint32_t line_count = 0;
    
    while(test_logging_active) {
        test_logger_write_line();
        line_count++;
        
        #ifdef DEBUG_MODE
        if(line_count % 10 == 0) {
            Serial.printf("[TEST_LOG] %lu lines written\n", line_count);
        }
        #endif
        
        vTaskDelayUntil(&last_wake, period);
    }
    
    test_logger_close_file();
    vTaskDelete(NULL);
}

// Demarrage
static bool test_logger_start() {
    BaseType_t ret = xTaskCreate(
        test_logger_task,
        "test_log",
        6144,
        NULL,
        3,
        &test_logger_task_handle
    );
    
    if(ret != pdPASS) {
        #ifdef DEBUG_MODE
        Serial.println("[TEST_LOG] Task creation failed");
        #endif
        return false;
    }
    
    return true;
}

// Arret (améliore la fonction existante)
static void test_logger_stop() {
    if(!test_logging_active) return;
    
    #ifdef DEBUG_MODE
    Serial.println("[TEST_LOG] Stopping logger...");
    #endif
    
    test_logging_active = false;
    
    // Attendre que la tâche se termine
    vTaskDelay(pdMS_TO_TICKS(200));
    
    // Fermer fichier si encore ouvert
    test_logger_close_file();
    
    #ifdef DEBUG_MODE
    Serial.println("[TEST_LOG] Logger stopped");
    #endif
}

#endif // TEST_MODE

#endif