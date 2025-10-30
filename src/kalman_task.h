#ifndef KALMAN_TASK_H
#define KALMAN_TASK_H

#include <Arduino.h>
#include "constants.h"
#include "globals.h"

// Structure des donnees filtrees
typedef struct {
  float altitude;      // m
  float vario;         // m/s
  float altitude_qne;  // m (QNE 1013.25 hPa)
  float altitude_qnh;  // m (QNH reglable)
  float altitude_qfe;  // m (hauteur sol)
  uint32_t timestamp;  // millis()
  bool valid;
} kalman_data_t;

// Structure interne du filtre
typedef struct {
  float x[3];        // Etat: [altitude, vario, accel_z]
  float P[3][3];     // Covariance
  float Q[3][3];     // Bruit processus
  float K[3];        // Gain
  bool initialized;  // Flag initialisation
} KalmanFilter_t;

// Variables globales
static KalmanFilter_t kf;
static kalman_data_t kalman_data;
static SemaphoreHandle_t kalman_mutex = NULL;
static float qnh_setting = 1013.25f;  // hPa
static float qfe_offset = 0.0f;  
static float last_qnh = 1013.25f;     // hPa (correction: c'était marqué "m")
static bool qnh_ready = false;           // QNH récupéré ou timeout
static uint32_t startup_time = 0;        // Timestamp démarrage
static const uint32_t SENSOR_STABILIZATION_TIME = 3000;  // 3 secondes

// Buffer init
static float init_buffer[INIT_SAMPLES];
static int init_count = 0;

// Fonction à appeler depuis metar_task quand QNH est récupéré
void kalman_set_qnh_ready(float qnh_hpa) {
  qnh_setting = qnh_hpa;
  qnh_ready = true;
  last_qnh = qnh_hpa;
  
#ifdef DEBUG_MODE
  Serial.printf("[KALMAN] QNH ready: %.2f hPa\n", qnh_hpa);
#endif
}

// Fonction pour forcer le démarrage avec QNH standard si timeout
void kalman_force_start_standard_qnh() {
  qnh_setting = 1013.25f;
  qnh_ready = true;
  last_qnh = 1013.25f;
  
#ifdef DEBUG_MODE
  Serial.println("[KALMAN] Forced start with standard QNH (1013.25 hPa)");
#endif
}

// Conversion pression -> altitude
static float pressure_to_altitude(float pressure_pa, float qnh_hpa) {
  float pressure_hpa = pressure_pa / 100.0f;
  return 44330.0f * (1.0f - pow(pressure_hpa / qnh_hpa, 0.1903f));
}

// Réinitialisation Kalman lors changement QNH
void kalman_reset_on_qnh_change(float new_altitude, float current_qnh) {
    if (fabs(current_qnh - last_qnh) > 0.5) {  // QNH a changé de > 0.5 hPa
#ifdef DEBUG_MODE
        Serial.printf("[KALMAN] QNH changed: %.2f -> %.2f hPa, reinitializing state\n", 
                      last_qnh, current_qnh);
#endif
        // Réinitialiser l'état avec la nouvelle altitude
        kf.x[0] = new_altitude;  // Position (correction: kf.x au lieu de state)
        kf.x[1] = 0.0;           // Vitesse remise à 0
        
        // Augmenter temporairement l'incertitude
        kf.P[0][0] = 100.0;  // Grande incertitude sur position (correction: kf.P au lieu de P)
        kf.P[1][1] = 10.0;   // Grande incertitude sur vitesse
        
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

  // Covariance initiale
  for (int i = 0; i < 3; i++) {
    for (int j = 0; j < 3; j++) {
      kf.P[i][j] = (i == j) ? 10.0f : 0.0f;
    }
  }

  // Bruit processus
  kf.Q[0][0] = 0.001f;
  kf.Q[0][1] = 0.0f;
  kf.Q[0][2] = 0.0f;
  kf.Q[1][0] = 0.0f;
  kf.Q[1][1] = 0.01f;
  kf.Q[1][2] = 0.0f;
  kf.Q[2][0] = 0.0f;
  kf.Q[2][1] = 0.0f;
  kf.Q[2][2] = 0.1f;

  kalman_data.altitude = 0.0f;
  kalman_data.vario = 0.0f;
  kalman_data.altitude_qne = 0.0f;
  kalman_data.altitude_qnh = 0.0f;
  kalman_data.altitude_qfe = 0.0f;
  kalman_data.timestamp = 0;
  kalman_data.valid = false;

  kalman_mutex = xSemaphoreCreateMutex();

#ifdef DEBUG_MODE
  Serial.println("[KALMAN] Init OK");
#endif
}

// Initialisation avec moyenne
static bool kalman_try_init() {
  // Vérifier que QNH est prêt
  if (!qnh_ready) {
#ifdef DEBUG_MODE
    static uint32_t last_qnh_msg = 0;
    if (millis() - last_qnh_msg > 1000) {
      Serial.println("[KALMAN] Waiting for QNH...");
      last_qnh_msg = millis();
    }
#endif
    return false;
  }
  
  // Attendre stabilisation capteurs (3 secondes après démarrage)
  if (millis() - startup_time < SENSOR_STABILIZATION_TIME) {
#ifdef DEBUG_MODE
    static uint32_t last_stab_msg = 0;
    if (millis() - last_stab_msg > 1000) {
      uint32_t remaining = SENSOR_STABILIZATION_TIME - (millis() - startup_time);
      Serial.printf("[KALMAN] Sensor stabilization... %lums remaining\n", remaining);
      last_stab_msg = millis();
    }
#endif
    return false;
  }
  
  if (!g_sensor_data.bmp390.valid) {
#ifdef DEBUG_MODE
    static uint32_t last_bmp_msg = 0;
    if (millis() - last_bmp_msg > 1000) {
      Serial.println("[KALMAN] Waiting for BMP390...");
      last_bmp_msg = millis();
    }
#endif
    return false;
  }

  float alt = pressure_to_altitude(g_sensor_data.bmp390.pressure, qnh_setting);

#ifdef DEBUG_MODE
  Serial.printf("[KALMAN] Init sample %d: P=%.1fPa QNH=%.2fhPa Alt=%.1fm\n", 
                init_count, g_sensor_data.bmp390.pressure, qnh_setting, alt);
#endif

  init_buffer[init_count++] = alt;

  if (init_count >= INIT_SAMPLES) {
    // Calcul moyenne
    float sum = 0.0f;
    for (int i = 0; i < INIT_SAMPLES; i++) {
      sum += init_buffer[i];
    }
    float alt_init = sum / INIT_SAMPLES;

    kf.x[0] = alt_init;
    kf.x[1] = 0.0f;
    kf.x[2] = 0.0f;
    kf.initialized = true;

#ifdef DEBUG_MODE
    Serial.printf("[KALMAN] Initialized with QNH=%.2f hPa, avg alt=%.1fm\n", 
                  qnh_setting, alt_init);
#endif

    return true;
  }

  return false;
}

// Prediction
static void kalman_predict(float dt) {
  if (!kf.initialized) return;

  if (dt > 0.1f) dt = 0.1f;
  if (dt < 0.001f) dt = 0.001f;

  float F[3][3] = {
    { 1.0f, dt, 0.5f * dt * dt },
    { 0.0f, 1.0f, dt },
    { 0.0f, 0.0f, 0.95f }
  };

  float x_new[3] = { 0 };
  for (int i = 0; i < 3; i++) {
    for (int j = 0; j < 3; j++) {
      x_new[i] += F[i][j] * kf.x[j];
    }
  }
  memcpy(kf.x, x_new, sizeof(x_new));

  float P_tmp[3][3] = { 0 };
  for (int i = 0; i < 3; i++) {
    for (int j = 0; j < 3; j++) {
      for (int k = 0; k < 3; k++) {
        P_tmp[i][j] += F[i][k] * kf.P[k][j];
      }
    }
  }

  for (int i = 0; i < 3; i++) {
    for (int j = 0; j < 3; j++) {
      kf.P[i][j] = kf.Q[i][j];
      for (int k = 0; k < 3; k++) {
        kf.P[i][j] += P_tmp[i][k] * F[j][k];
      }
    }
  }
}

// Update mesure
static void kalman_update(float measurement, float variance, int measurement_idx) {
  if (!kf.initialized) return;

  float y = measurement - kf.x[measurement_idx];

  // Outlier rejection (plus tolerant)
  if (measurement_idx == 0 && fabs(y) > 100.0f) {
#ifdef DEBUG_MODE
    Serial.printf("[KALMAN] Outlier rejected: meas=%.1fm state=%.1fm diff=%.1fm\n", measurement, kf.x[0], y);
#endif
    return;
  }

  float S = kf.P[measurement_idx][measurement_idx] + variance;
  if (S < 0.001f) S = 0.001f;

  for (int i = 0; i < 3; i++) {
    kf.K[i] = kf.P[i][measurement_idx] / S;
  }

  for (int i = 0; i < 3; i++) {
    kf.x[i] += kf.K[i] * y;
  }

  if (kf.x[1] > 20.0f) kf.x[1] = 20.0f;
  if (kf.x[1] < -20.0f) kf.x[1] = -20.0f;

  float P_new[3][3];
  for (int i = 0; i < 3; i++) {
    for (int j = 0; j < 3; j++) {
      P_new[i][j] = kf.P[i][j] - kf.K[i] * kf.P[measurement_idx][j];
    }
  }
  memcpy(kf.P, P_new, sizeof(P_new));
}

// Rotation acceleration
static float get_accel_z_world(const bno080_data_t* imu) {
  float qw = imu->quat_real;
  float qx = imu->quat_i;
  float qy = imu->quat_j;
  float qz = imu->quat_k;

  float ax = imu->accel_x;
  float ay = imu->accel_y;
  float az = imu->accel_z;

  float az_world = ax * (2.0f * qx * qz - 2.0f * qw * qy) + ay * (2.0f * qy * qz + 2.0f * qw * qx) + az * (qw * qw - qx * qx - qy * qy + qz * qz);

  return az_world;
}

// Getters
void kalman_get_data(kalman_data_t* out) {
  if (xSemaphoreTake(kalman_mutex, pdMS_TO_TICKS(10))) {
    memcpy(out, &kalman_data, sizeof(kalman_data_t));
    xSemaphoreGive(kalman_mutex);
  }
}

void kalman_set_qnh(float qnh_hpa) {
  qnh_setting = qnh_hpa;
}

void kalman_reset_qfe() {
  qfe_offset = kf.x[0];
}

// Tache principale
static void kalman_task(void* parameter) {
  kalman_init();
  
  startup_time = millis();  // Sauvegarder temps de démarrage

  TickType_t last_wake = xTaskGetTickCount();
  const TickType_t period = pdMS_TO_TICKS(50);

  uint32_t last_baro_time = 0;
  uint32_t last_gps_time = 0;

#ifdef DEBUG_MODE
  Serial.println("[KALMAN] Task started - waiting for QNH and sensor stabilization");
#endif

  while (1) {
    uint32_t now = millis();
    TickType_t current_tick = xTaskGetTickCount();
    float dt = (current_tick - last_wake) * portTICK_PERIOD_MS / 1000.0f;
    last_wake = current_tick;

    // Phase init
    if (!kf.initialized) {
      if (kalman_try_init()) {
        last_wake = xTaskGetTickCount();
      }
      vTaskDelayUntil(&last_wake, period);
      continue;
    }

    // Phase normale - 50Hz
    kalman_predict(dt);

    // Update baro
    if (g_sensor_data.bmp390.valid) {
      if (now - last_baro_time >= 20) {
        float alt_baro = pressure_to_altitude(g_sensor_data.bmp390.pressure, qnh_setting);
        kalman_reset_on_qnh_change(alt_baro, qnh_setting);  // Correction: alt_baro et qnh_setting
        kalman_update(alt_baro, 0.25f, 0);
        last_baro_time = now;
      }
    }

    // Update GPS
    if (g_sensor_data.gps.valid && g_sensor_data.gps.fix && g_sensor_data.gps.fixquality >= 1) {
      if (now - last_gps_time >= 500) {
        // Pas besoin de reset QNH ici, l'altitude GPS ne dépend pas du QNH
        kalman_update(g_sensor_data.gps.altitude, 5.0f, 0);
        last_gps_time = now;
      }
    }

    // Update IMU
    if (g_sensor_data.bno080.valid) {
      float az_world = get_accel_z_world(&g_sensor_data.bno080);
      if (fabs(az_world) < 0.05f) az_world = 0.0f;
      // Pas de reset QNH pour l'accélération
      kalman_update(az_world, 1.0f, 2);
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
      Serial.printf("[KALMAN] Alt:%.1fm Vario:%.2fm/s Accel:%.2fm/s2\n", kf.x[0], kf.x[1], kf.x[2]);
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

#ifdef TEST_MODE
extern KalmanFilter_t kf;
#endif

#endif