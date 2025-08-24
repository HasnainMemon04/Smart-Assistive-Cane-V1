#pragma once
#ifndef SENSORHEALTH_H
#define SENSORHEALTH_H

#include <Arduino.h>

// Sensor health status codes
enum SensorStatus {
  SENSOR_OK = 0,
  SENSOR_ERROR = 1,
  SENSOR_TIMEOUT = 2,
  SENSOR_DISCONNECTED = 3,
  SENSOR_INIT_FAILED = 4
};

// Individual sensor health data
struct SensorHealthData {
  SensorStatus status;
  uint32_t lastUpdate;        // Last successful update timestamp
  uint32_t errorCount;        // Total error count
  char lastValue[32];         // Last known good value as string
  char errorMessage[64];      // Last error message
};

// Complete sensor health report
struct SensorHealthReport {
  SensorHealthData vl53l1x;   // ToF Distance sensor
  SensorHealthData mpu6050;   // IMU sensor
  SensorHealthData dht22;     // Temperature/Humidity sensor
  SensorHealthData bh1750;    // Light sensor
  SensorHealthData mfrc522;   // RFID reader
  SensorHealthData neo6m;     // GPS module
  uint32_t reportTimestamp;   // When this report was generated
};

class SensorHealthManager {
public:
  static void init();
  static void updateSensorHealth(const char* sensorName, SensorStatus status, const char* value = nullptr, const char* error = nullptr);
  static SensorHealthReport getHealthReport();
  static void sendHealthReport();  // Send via BLE (with rate limiting)
  static void sendHealthReportImmediate();  // Send via BLE immediately (no rate limiting)
  static bool isSensorHealthy(const char* sensorName);
  static void printHealthStatus();
  
private:
  static SensorHealthReport healthReport;
  static SensorHealthData* getSensorData(const char* sensorName);
  static const char* getStatusString(SensorStatus status);
  static uint32_t lastHealthBroadcast;
  static const uint32_t HEALTH_BROADCAST_INTERVAL = 3000; // 3 seconds
};

#endif // SENSORHEALTH_H