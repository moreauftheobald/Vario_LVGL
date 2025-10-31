#ifndef __SENSORS_I2C_TASK_H
#define __SENSORS_I2C_TASK_H

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "src/BMP3XX_ESP32/BMP3XX_ESP32.h"
#include "src/BNO08x_ESP32/BNO08x_ESP32.h"
#include "src/GPS_I2C_ESP32/GPS_I2C_ESP32.h"
#include "src/i2c/i2c.h"
#include "constants.h"
#include "globals.h"

static BMP3XX_ESP32 bmp390;
static BNO08x_ESP32 bno080(BNO080_RESET_PIN);
static gps_i2c_esp32_t gps;
static TaskHandle_t sensors_task_handle = NULL;

static void sensors_i2c_task(void *pvParameters) {
  TickType_t xLastWakeTime = xTaskGetTickCount();
  const TickType_t xFrequency = pdMS_TO_TICKS(1000 / BMP390_SAMPLE_RATE_HZ);

#ifdef DEBUG_MODE
  Serial.println("[SENSORS] Task started");
#endif

  while (1) {
    // Lecture BMP390
    if (bmp390.performReading()) {
      g_sensor_data.bmp390.temperature = bmp390.temperature;
      g_sensor_data.bmp390.pressure = bmp390.pressure;
      g_sensor_data.bmp390.timestamp = millis();
      g_sensor_data.bmp390.valid = true;
      float alti = bmp390.readAltitude(g_sensor_data.qnh_metar);
      // Utiliser QNH METAR si disponible, sinon standard
      float qnh_ref = g_sensor_data.qnh_metar;

      /*#ifdef DEBUG_MODE
      Serial.printf("[BMP390] Using QNH: %.1f hPa altitude: %.1f\n",
                    qnh_ref, alti);
#endif*/
    } else {
      g_sensor_data.bmp390.valid = false;
#ifdef DEBUG_MODE
      Serial.println("[BMP390] Read failed");
#endif
    }
    // Lecture BNO080
    sh2_SensorValue_t sensorValue;
    if (bno080.getSensorEvent(&sensorValue)) {
      switch (sensorValue.sensorId) {
        case SH2_ROTATION_VECTOR:
          g_sensor_data.bno080.quat_i = sensorValue.un.rotationVector.i;
          g_sensor_data.bno080.quat_j = sensorValue.un.rotationVector.j;
          g_sensor_data.bno080.quat_k = sensorValue.un.rotationVector.k;
          g_sensor_data.bno080.quat_real = sensorValue.un.rotationVector.real;
          g_sensor_data.bno080.timestamp = millis();
          g_sensor_data.bno080.valid = true;
          /*#ifdef DEBUG_MODE
          Serial.printf("[BNO080] Quat: %.3f %.3f %.3f %.3f\n",
                        g_sensor_data.bno080.quat_real,
                        g_sensor_data.bno080.quat_i,
                        g_sensor_data.bno080.quat_j,
                        g_sensor_data.bno080.quat_k);
#endif*/
          break;

        case SH2_LINEAR_ACCELERATION:
          g_sensor_data.bno080.accel_x = sensorValue.un.linearAcceleration.x;
          g_sensor_data.bno080.accel_y = sensorValue.un.linearAcceleration.y;
          g_sensor_data.bno080.accel_z = sensorValue.un.linearAcceleration.z;
          /*#ifdef DEBUG_MODE
          Serial.printf("[BNO080] Accel: %.2f %.2f %.2f m/s²\n",
                        g_sensor_data.bno080.accel_x,
                        g_sensor_data.bno080.accel_y,
                        g_sensor_data.bno080.accel_z);
#endif*/
          break;

        case SH2_GYROSCOPE_CALIBRATED:
          g_sensor_data.bno080.gyro_x = sensorValue.un.gyroscope.x;
          g_sensor_data.bno080.gyro_y = sensorValue.un.gyroscope.y;
          g_sensor_data.bno080.gyro_z = sensorValue.un.gyroscope.z;
          break;
      }
    } else {
      g_sensor_data.bno080.valid = false;
    }

    // Lecture GPS
    // Lecture GPS
    char c = GPS_I2C_ESP32_read(&gps);
    if (c) {
      if (GPS_I2C_ESP32_new_nmea_received(&gps)) {
        if (GPS_I2C_ESP32_parse(&gps, GPS_I2C_ESP32_last_nmea(&gps))) {
          // Copier trame brute
          strncpy(g_sensor_data.gps.lastline,
                  GPS_I2C_ESP32_last_nmea(&gps),
                  sizeof(g_sensor_data.gps.lastline) - 1);
          g_sensor_data.gps.lastline[sizeof(g_sensor_data.gps.lastline) - 1] = '\0';

          // OPTIMISE: Copie bloc au lieu de champ par champ
          g_sensor_data.gps.fix = gps.fix;
          g_sensor_data.gps.fixquality = gps.fixquality;
          g_sensor_data.gps.satellites = gps.satellites;
          g_sensor_data.gps.latitude = gps.latitudeDegrees;
          g_sensor_data.gps.longitude = gps.longitudeDegrees;
          g_sensor_data.gps.altitude = gps.altitude;
          g_sensor_data.gps.speed = gps.speed;
          g_sensor_data.gps.angle = gps.angle;
          g_sensor_data.gps.hour = gps.hour;
          g_sensor_data.gps.minute = gps.minute;
          g_sensor_data.gps.seconds = gps.seconds;
          g_sensor_data.gps.year = gps.year;
          g_sensor_data.gps.month = gps.month;
          g_sensor_data.gps.day = gps.day;
          g_sensor_data.gps.timestamp = millis();
          g_sensor_data.gps.valid = true;

#ifdef DEBUG_MODE
          // Debug commenté pour performances
          /*Serial.printf("[GPS] Fix:%d Sat:%d Lat:%.6f Lon:%.6f Alt:%.1fm\n",
                    g_sensor_data.gps.fix,
                    g_sensor_data.gps.satellites,
                    g_sensor_data.gps.latitude,
                    g_sensor_data.gps.longitude,
                    g_sensor_data.gps.altitude);*/
#endif
        }
      }
    } else {
      // Invalider si timeout
      static uint32_t last_gps_time = 0;
      if (g_sensor_data.gps.valid) {
        last_gps_time = millis();
      } else if (millis() - last_gps_time > 2000) {
        g_sensor_data.gps.valid = false;
      }
    }

    vTaskDelayUntil(&xLastWakeTime, xFrequency);
  }
}

static bool sensors_i2c_init() {
  // Recuperer le bus I2C existant
  DEV_I2C_Port i2c_port = DEV_I2C_Get_Handle();

  // Initialiser BMP390
  if (!bmp390.begin_I2C(BMP390_I2C_ADDR, i2c_port.bus)) {
#ifdef DEBUG_MODE
    Serial.println("[SENSORS] BMP390 init failed");
#endif
    return false;
  }

  // Config BMP390 pour vario: oversampling eleve + filtre IIR
  bmp390.setTemperatureOversampling(BMP3_OVERSAMPLING_2X);
  bmp390.setPressureOversampling(BMP3_OVERSAMPLING_16X);
  bmp390.setIIRFilterCoeff(BMP3_IIR_FILTER_COEFF_7);
  bmp390.setOutputDataRate(BMP3_ODR_50_HZ);

#ifdef DEBUG_MODE
  Serial.printf("[SENSORS] BMP390 init OK (ID=0x%02X)\n", bmp390.chipID());
#endif

  // Initialiser BNO080
  if (!bno080.begin_I2C(i2c_port.bus, BNO080_I2C_ADDR, 400000)) {
#ifdef DEBUG_MODE
    Serial.println("[SENSORS] BNO080 init failed");
#endif
    return false;
  }

  // Activer rapports rotation vector (quaternions) et accelerometre lineaire
  bno080.enableReport(SH2_ROTATION_VECTOR, 10000);  // 100Hz = 10000us
  bno080.enableReport(SH2_LINEAR_ACCELERATION, 10000);
  bno080.enableReport(SH2_GYROSCOPE_CALIBRATED, 10000);

#ifdef DEBUG_MODE
  Serial.println("[SENSORS] BNO080 init OK");
  /*Serial.printf("[SENSORS] Part: %d, Ver: %d.%d.%d, Build: %d\n",
                bno080.prodIds.entry[0].swPartNumber,
                bno080.prodIds.entry[0].swVersionMajor,
                bno080.prodIds.entry[0].swVersionMinor,
                bno080.prodIds.entry[0].swVersionPatch,
                bno080.prodIds.entry[0].swBuildNumber);*/
#endif

  // Initialiser GPS
  gps_i2c_esp32_config_t gps_config = {
    .i2c_addr = GPS_I2C_ADDR,
    .i2c_speed_hz = 400000
  };

  if (GPS_I2C_ESP32_init(&gps, i2c_port.bus, &gps_config) != ESP_OK) {
#ifdef DEBUG_MODE
    Serial.println("[SENSORS] GPS init failed");
#endif
    return false;
  }

  // Attendre stabilisation
  vTaskDelay(pdMS_TO_TICKS(500));

  // Configuration GPS: trames RMC+GGA uniquement a 2Hz
  GPS_I2C_ESP32_send_command(&gps, PMTK_SET_NMEA_OUTPUT_RMCGGA);
  vTaskDelay(pdMS_TO_TICKS(100));
  GPS_I2C_ESP32_send_command(&gps, PMTK_SET_NMEA_UPDATE_2HZ);
  vTaskDelay(pdMS_TO_TICKS(100));
  GPS_I2C_ESP32_send_command(&gps, PMTK_API_SET_FIX_CTL_2HZ);
  vTaskDelay(pdMS_TO_TICKS(100));

#ifdef DEBUG_MODE
  Serial.println("[SENSORS] GPS init OK (2Hz, RMC+GGA)");
#endif

  return true;
}

static bool sensors_i2c_start() {
  if (!sensors_i2c_init()) {
    return false;
  }

  BaseType_t ret = xTaskCreatePinnedToCore(
    sensors_i2c_task,
    "sensors_i2c",
    SENSORS_TASK_STACK_SIZE,
    NULL,
    SENSORS_TASK_PRIORITY,
    &sensors_task_handle,
    0);

  if (ret != pdPASS) {
#ifdef DEBUG_MODE
    Serial.println("[SENSORS] Task creation failed");
#endif
    return false;
  }

#ifdef DEBUG_MODE
  Serial.println("[SENSORS] Task created successfully");
#endif

  return true;
}

#endif