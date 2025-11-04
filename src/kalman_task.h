#ifndef KALMAN_TASK_H
#define KALMAN_TASK_H

#include <Arduino.h>
#include "constants.h"
#include "globals.h"
#include "terrain_elevation.h" 

// Structure des donnees filtrees
typedef struct {
  float altitude;
  float vario;
  float altitude_qne;
  float altitude_qnh;
  float altitude_qfe;
  uint32_t timestamp;
  bool valid;
} kalman_data_t;

// Structure interne du filtre
typedef struct {
  float x[3];
  float P[3][3];
  float Q[3][3];
  float K[3];
  bool initialized;
} KalmanFilter_t;

// Variables globales
static KalmanFilter_t kf;
static kalman_data_t kalman_data;
static SemaphoreHandle_t kalman_mutex = NULL;
static float qnh_setting = 1013.25f;
static float qfe_offset = 0.0f;  
static float last_qnh = 1013.25f;
static bool qnh_ready = false;
static uint32_t startup_time = 0;
static const uint32_t SENSOR_STABILIZATION_TIME = 3000;

// Buffer init
static float init_buffer[INIT_SAMPLES];
static int init_count = 0;

// Fonction pour appeler depuis metar_task quand QNH est recupere
void kalman_set_qnh_ready(float qnh_hpa) {
  qnh_setting = qnh_hpa;
  qnh_ready = true;
  last_qnh = qnh_hpa;
  
#ifdef DEBUG_MODE
  Serial.printf("[KALMAN] QNH ready: %.2f hPa\n", qnh_hpa);
#endif
}

// Fonction pour forcer le demarrage avec QNH standard si timeout
void kalman_force_start_standard_qnh() {
  qnh_setting = 1013.25f;
  qnh_ready = true;
  last_qnh = 1013.25f;
  
#ifdef DEBUG_MODE
  Serial.println("[KALMAN] Forced start with standard QNH (1013.25 hPa)");
#endif
}

// METHOD 3: Appliquer un offset d'altitude direct
void kalman_apply_altitude_offset(float offset_m) {
#ifdef DEBUG_MODE
  Serial.printf("[KALMAN] Applying altitude offset: %.1f m (before: %.1f m)\n", 
                offset_m, kf.x[0]);
#endif
  
  // Appliquer l'offset sur l'altitude uniquement
  kf.x[0] += offset_m;
  
#ifdef DEBUG_MODE
  Serial.printf("[KALMAN] Altitude after offset: %.1f m\n", kf.x[0]);
#endif
}

// Conversion pression -> altitude
static float pressure_to_altitude(float pressure_pa, float qnh_hpa) {
  float pressure_hpa = pressure_pa / 100.0f;
  return 44330.0f * (1.0f - pow(pressure_hpa / qnh_hpa, 0.1903f));
}

// Reinitialisation Kalman lors changement QNH (pas utilisee avec les nouvelles methodes)
void kalman_reset_on_qnh_change(float new_altitude, float current_qnh) {
    if (fabs(current_qnh - last_qnh) > 0.5) {
#ifdef DEBUG_MODE
        Serial.printf("[KALMAN] QNH changed: %.2f -> %.2f hPa, reinitializing state\n", 
                      last_qnh, current_qnh);
#endif
        kf.x[0] = new_altitude;
        kf.x[1] = 0.0;
        
        kf.P[0][0] = 100.0;
        kf.P[1][1] = 10.0;
        
        last_qnh = current_qnh;
    }
}

// Init filtre
static void kalman_init() {
  kf.x[0] = 0.0f;
  kf.x[1] = 0.0f;
  kf.x[2] = 0.0f;
  kf.initialized = false;
  init_count = 0;

  for (int i = 0; i < 3; i++) {
    for (int j = 0; j < 3; j++) {
      kf.P[i][j] = (i == j) ? 1.0f : 0.0f;
    }
  }

  kf.Q[0][0] = 0.001f;
  kf.Q[1][1] = 0.01f;
  kf.Q[2][2] = 0.1f;
  kf.Q[0][1] = kf.Q[1][0] = 0.0f;
  kf.Q[0][2] = kf.Q[2][0] = 0.0f;
  kf.Q[1][2] = kf.Q[2][1] = 0.0f;

#ifdef DEBUG_MODE
  Serial.println("[KALMAN] Filter initialized");
#endif
}

// Predict
static void kalman_predict(float dt) {
  float x_pred[3];
  x_pred[0] = kf.x[0] + kf.x[1] * dt + 0.5f * kf.x[2] * dt * dt;
  x_pred[1] = kf.x[1] + kf.x[2] * dt;
  x_pred[2] = kf.x[2];

  float F[3][3] = {
    {1.0f, dt, 0.5f * dt * dt},
    {0.0f, 1.0f, dt},
    {0.0f, 0.0f, 1.0f}
  };

  float P_pred[3][3] = {0};
  for (int i = 0; i < 3; i++) {
    for (int j = 0; j < 3; j++) {
      for (int k = 0; k < 3; k++) {
        P_pred[i][j] += F[i][k] * kf.P[k][j];
      }
    }
  }

  float temp[3][3] = {0};
  for (int i = 0; i < 3; i++) {
    for (int j = 0; j < 3; j++) {
      temp[i][j] = P_pred[i][j];
    }
  }

  for (int i = 0; i < 3; i++) {
    for (int j = 0; j < 3; j++) {
      P_pred[i][j] = 0;
      for (int k = 0; k < 3; k++) {
        P_pred[i][j] += temp[i][k] * F[j][k];
      }
    }
  }

  for (int i = 0; i < 3; i++) {
    for (int j = 0; j < 3; j++) {
      P_pred[i][j] += kf.Q[i][j];
    }
  }

  for (int i = 0; i < 3; i++) {
    kf.x[i] = x_pred[i];
    for (int j = 0; j < 3; j++) {
      kf.P[i][j] = P_pred[i][j];
    }
  }
}

// Update
static void kalman_update(float measurement, float variance, int measurement_type) {
  float H[3] = {0};
  if (measurement_type == 0 || measurement_type == 1) {
    H[0] = 1.0f;
  } else if (measurement_type == 2) {
    H[2] = 1.0f;
  }

  float y = measurement;
  for (int i = 0; i < 3; i++) {
    y -= H[i] * kf.x[i];
  }

  float S = variance;
  for (int i = 0; i < 3; i++) {
    for (int j = 0; j < 3; j++) {
      S += H[i] * kf.P[i][j] * H[j];
    }
  }

  for (int i = 0; i < 3; i++) {
    kf.K[i] = 0.0f;
    for (int j = 0; j < 3; j++) {
      kf.K[i] += kf.P[i][j] * H[j];
    }
    kf.K[i] /= S;
  }

  for (int i = 0; i < 3; i++) {
    kf.x[i] += kf.K[i] * y;
  }

  float I_KH[3][3];
  for (int i = 0; i < 3; i++) {
    for (int j = 0; j < 3; j++) {
      I_KH[i][j] = (i == j) ? 1.0f : 0.0f;
      I_KH[i][j] -= kf.K[i] * H[j];
    }
  }

  float P_new[3][3] = {0};
  for (int i = 0; i < 3; i++) {
    for (int j = 0; j < 3; j++) {
      for (int k = 0; k < 3; k++) {
        P_new[i][j] += I_KH[i][k] * kf.P[k][j];
      }
    }
  }

  for (int i = 0; i < 3; i++) {
    for (int j = 0; j < 3; j++) {
      kf.P[i][j] = P_new[i][j];
    }
  }
}

// Rotation quaternion -> acceleration monde
static float get_accel_z_world(bno080_data_t* imu) {
  float qw = imu->quat_real;
  float qx = imu->quat_i;
  float qy = imu->quat_j;
  float qz = imu->quat_k;

  float ax = imu->accel_x;
  float ay = imu->accel_y;
  float az = imu->accel_z;

  // Rotation quaternion correcte
  float az_world = ax * (2.0f * qx * qz - 2.0f * qw * qy)
                 + ay * (2.0f * qy * qz + 2.0f * qw * qx)
                 + az * (qw * qw - qx * qx - qy * qy + qz * qz);

  return az_world;  // Pas de "- 9.81f" avec LINEAR_ACCELERATION
}

// Tache principale
static void kalman_task(void* parameter) {
#ifdef DEBUG_MODE
  Serial.println("[KALMAN] Task started");
#endif

  kalman_init();
  if (!kalman_mutex) kalman_mutex = xSemaphoreCreateMutex();

  startup_time = millis();

  TickType_t last_wake = xTaskGetTickCount();
  uint32_t last_baro_time = 0;
  uint32_t last_gps_time = 0;

  while (1) {
    if (!qnh_ready) {
      vTaskDelay(pdMS_TO_TICKS(100));
      continue;
    }

    if ((millis() - startup_time) < SENSOR_STABILIZATION_TIME) {
      vTaskDelay(pdMS_TO_TICKS(100));
      continue;
    }

    if (!kf.initialized) {
      if (g_sensor_data.bmp390.valid) {
        float alt = pressure_to_altitude(g_sensor_data.bmp390.pressure, qnh_setting);
        init_buffer[init_count++] = alt;

        if (init_count >= INIT_SAMPLES) {
          float sum = 0.0f;
          for (int i = 0; i < INIT_SAMPLES; i++) {
            sum += init_buffer[i];
          }
          kf.x[0] = sum / INIT_SAMPLES;
          kf.x[1] = 0.0f;
          kf.x[2] = 0.0f;
          kf.initialized = true;
          qfe_offset = kf.x[0];

#ifdef DEBUG_MODE
          Serial.printf("[KALMAN] Initialized at %.1f m\n", kf.x[0]);
#endif
        }
      }
      vTaskDelay(pdMS_TO_TICKS(50));
      continue;
    }

    uint32_t now = millis();
    kalman_predict(0.02f);

    // Update Baro
    if (g_sensor_data.bmp390.valid) {
      if (now - last_baro_time >= 200) {
        float alt_baro = pressure_to_altitude(g_sensor_data.bmp390.pressure, qnh_setting);
        kalman_update(alt_baro, 0.25f, 0);
        last_baro_time = now;
      }
    }

    // Update GPS
    if (g_sensor_data.gps.valid && g_sensor_data.gps.fix && g_sensor_data.gps.fixquality >= 1) {
      if (now - last_gps_time >= 500) {
        kalman_update(g_sensor_data.gps.altitude, 5.0f, 0);
        last_gps_time = now;
      }
    }

    // Update IMU avec ajustement confiance si METHOD 2
    if (g_sensor_data.bno080.valid) {
      float az_world = get_accel_z_world(&g_sensor_data.bno080);
      if (fabs(az_world) < 0.05f) az_world = 0.0f;
      
      // METHOD 2: Si en transition QNH, augmenter la confiance IMU
      float imu_variance = 1.0f;  // Variance par defaut
      
#if QNH_ADJUST_METHOD == 2
      if (g_sensor_data.qnh_transition) {
        imu_variance = 0.1f;  // 10x plus de confiance pendant transition
#ifdef DEBUG_MODE
        static uint32_t last_debug_transition = 0;
        if (now - last_debug_transition >= 1000) {
          Serial.println("[KALMAN] QNH transition: IMU confidence boosted");
          last_debug_transition = now;
        }
#endif
      }
#endif
      
      kalman_update(az_world, imu_variance, 2);
    }

    // Maj donnees filtrees
    if (g_sensor_data.bmp390.valid) {
      if (xSemaphoreTake(kalman_mutex, pdMS_TO_TICKS(5))) {
        kalman_data.altitude = kf.x[0];
        kalman_data.vario = kf.x[1];
        kalman_data.altitude_qne = pressure_to_altitude(g_sensor_data.bmp390.pressure, 1013.25f);
        kalman_data.altitude_qnh = kf.x[0];
        kalman_data.altitude_qfe = kf.x[0] - qfe_offset;
        kalman_data.timestamp = now;
        kalman_data.valid = true;
        xSemaphoreGive(kalman_mutex);
      }
    }

#ifdef DEBUG_MODE
    static uint32_t last_debug = 0;
    if (now - last_debug >= 1000) {
      float terrain_alt = NAN;
      float hauteur_sol = NAN;
      
#ifdef FLIGHT_TEST_MODE
      // Mode test: coordonnées fixes
      terrain_alt = terrain.getElevation(TEST_LAT, TEST_LON);
      
      if (!isnan(terrain_alt)) {
        hauteur_sol = kf.x[0] - terrain_alt;
      }
      
      /*Serial.printf("[KALMAN] Alt:%.1fm Vario:%.2fm/s Accel:%.2fm/s2 TerrainAlt:%.1fm HauteurSol:%.1fm (TEST_MODE)\n", 
                    kf.x[0], kf.x[1], kf.x[2], terrain_alt, hauteur_sol);*/
#else
      // Mode réel: coordonnées GPS, altitude Kalman
      if (g_sensor_data.gps.valid && g_sensor_data.gps.fix) {
        terrain_alt = terrain.getElevation(
          g_sensor_data.gps.latitude, 
          g_sensor_data.gps.longitude
        );
        
        if (!isnan(terrain_alt)) {
          hauteur_sol = kf.x[0] - terrain_alt;  // Altitude Kalman - Terrain
        }
      }
      
      /*Serial.printf("[KALMAN] Alt:%.1fm Vario:%.2fm/s Accel:%.2fm/s2 TerrainAlt:%.1fm HauteurSol:%.1fm\n", 
                    kf.x[0], kf.x[1], kf.x[2], terrain_alt, hauteur_sol);*/
#endif
      
      last_debug = now;
    }
#endif
    vTaskDelayUntil(&last_wake, pdMS_TO_TICKS(20));
  }
}

// Demarrage
static TaskHandle_t kalman_task_handle = NULL;

static bool kalman_start() {
  BaseType_t ret = xTaskCreatePinnedToCore(
    kalman_task,
    "kalman",
    4096,
    NULL,
    4,
    &kalman_task_handle,
    0);

  if (ret != pdPASS) {
#ifdef DEBUG_MODE
    Serial.println("[KALMAN] Task creation failed");
#endif
    return false;
  }

  return true;
}

// Fonctions publiques d'acces aux donnees
bool kalman_get_data(kalman_data_t* out) {
  if (!kalman_mutex) return false;
  
  if (xSemaphoreTake(kalman_mutex, pdMS_TO_TICKS(10))) {
    memcpy(out, &kalman_data, sizeof(kalman_data_t));
    xSemaphoreGive(kalman_mutex);
    return kalman_data.valid;
  }
  return false;
}

void kalman_set_qfe(void) {
  if (kf.initialized) {
    qfe_offset = kf.x[0];
#ifdef DEBUG_MODE
    Serial.printf("[KALMAN] QFE offset set to %.1f m\n", qfe_offset);
#endif
  }
}

#ifdef TEST_MODE
extern KalmanFilter_t kf;
#endif

#endif