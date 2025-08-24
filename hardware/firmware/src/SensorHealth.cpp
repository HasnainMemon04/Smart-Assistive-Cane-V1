#include "SensorHealth.h"
#include "BLEManager.h"
#include <ArduinoJson.h>

// Static member definitions
SensorHealthReport SensorHealthManager::healthReport;
uint32_t SensorHealthManager::lastHealthBroadcast = 0;

void SensorHealthManager::init() {
  // Initialize all sensor health data
  memset(&healthReport, 0, sizeof(healthReport));
  
  // Set initial status for all sensors
  healthReport.vl53l1x.status = SENSOR_INIT_FAILED;
  healthReport.mpu6050.status = SENSOR_INIT_FAILED;
  healthReport.dht22.status = SENSOR_INIT_FAILED;
  healthReport.bh1750.status = SENSOR_INIT_FAILED;
  healthReport.mfrc522.status = SENSOR_INIT_FAILED;
  healthReport.neo6m.status = SENSOR_INIT_FAILED;
  
  strcpy(healthReport.vl53l1x.lastValue, "N/A");
  strcpy(healthReport.mpu6050.lastValue, "N/A");
  strcpy(healthReport.dht22.lastValue, "N/A");
  strcpy(healthReport.bh1750.lastValue, "N/A");
  strcpy(healthReport.mfrc522.lastValue, "N/A");
  strcpy(healthReport.neo6m.lastValue, "N/A");
  
  Serial.println("✅ Sensor Health Manager initialized");
}

SensorHealthData* SensorHealthManager::getSensorData(const char* sensorName) {
  if (strcmp(sensorName, "vl53l1x") == 0) return &healthReport.vl53l1x;
  if (strcmp(sensorName, "mpu6050") == 0) return &healthReport.mpu6050;
  if (strcmp(sensorName, "dht22") == 0) return &healthReport.dht22;
  if (strcmp(sensorName, "bh1750") == 0) return &healthReport.bh1750;
  if (strcmp(sensorName, "mfrc522") == 0) return &healthReport.mfrc522;
  if (strcmp(sensorName, "neo6m") == 0) return &healthReport.neo6m;
  return nullptr;
}

const char* SensorHealthManager::getStatusString(SensorStatus status) {
  switch (status) {
    case SENSOR_OK: return "OK";
    case SENSOR_ERROR: return "ERROR";
    case SENSOR_TIMEOUT: return "TIMEOUT";
    case SENSOR_DISCONNECTED: return "DISCONNECTED";
    case SENSOR_INIT_FAILED: return "INIT_FAILED";
    default: return "UNKNOWN";
  }
}

void SensorHealthManager::updateSensorHealth(const char* sensorName, SensorStatus status, const char* value, const char* error) {
  SensorHealthData* sensor = getSensorData(sensorName);
  if (!sensor) return;
  
  sensor->status = status;
  sensor->lastUpdate = millis();
  
  if (status != SENSOR_OK) {
    sensor->errorCount++;
    if (error) {
      strncpy(sensor->errorMessage, error, sizeof(sensor->errorMessage) - 1);
      sensor->errorMessage[sizeof(sensor->errorMessage) - 1] = '\0';
    }
  } else {
    // Clear error message on successful update
    sensor->errorMessage[0] = '\0';
    if (value) {
      strncpy(sensor->lastValue, value, sizeof(sensor->lastValue) - 1);
      sensor->lastValue[sizeof(sensor->lastValue) - 1] = '\0';
    }
  }
}

SensorHealthReport SensorHealthManager::getHealthReport() {
  healthReport.reportTimestamp = millis();
  return healthReport;
}

bool SensorHealthManager::isSensorHealthy(const char* sensorName) {
  SensorHealthData* sensor = getSensorData(sensorName);
  if (!sensor) return false;
  
  // Consider sensor healthy if status is OK and last update was recent (within 10 seconds)
  return (sensor->status == SENSOR_OK) && (millis() - sensor->lastUpdate < 10000);
}

void SensorHealthManager::sendHealthReport() {
  Serial.println("🏥 [DEBUG] sendHealthReport() called");
  uint32_t now = millis();
  
  // Only send health report every HEALTH_BROADCAST_INTERVAL milliseconds
  if (now - lastHealthBroadcast < HEALTH_BROADCAST_INTERVAL) {
    Serial.printf("🏥 [DEBUG] Rate limited - last broadcast %lu ms ago (min interval: %lu ms)\n", 
                  now - lastHealthBroadcast, HEALTH_BROADCAST_INTERVAL);
    return;
  }
  
  lastHealthBroadcast = now;
  
  if (!BLEManager::isConnected()) {
    Serial.println("🏥 [DEBUG] BLE not connected - cannot send health report");
    return;
  }
  
  Serial.println("🏥 [DEBUG] BLE connected - proceeding with health report generation");
  
  // Create JSON health report
  DynamicJsonDocument doc(1024);
  doc["type"] = "sensor_health";
  doc["timestamp"] = now;
  
  JsonObject sensors = doc.createNestedObject("sensors");
  
  // VL53L1X ToF Sensor
  JsonObject vl53 = sensors.createNestedObject("vl53l1x");
  vl53["name"] = "ToF Distance";
  vl53["status"] = getStatusString(healthReport.vl53l1x.status);
  vl53["lastUpdate"] = healthReport.vl53l1x.lastUpdate;
  vl53["lastValue"] = healthReport.vl53l1x.lastValue;
  vl53["errorCount"] = healthReport.vl53l1x.errorCount;
  if (strlen(healthReport.vl53l1x.errorMessage) > 0) {
    vl53["error"] = healthReport.vl53l1x.errorMessage;
  }
  
  // MPU6050 IMU
  JsonObject mpu = sensors.createNestedObject("mpu6050");
  mpu["name"] = "IMU";
  mpu["status"] = getStatusString(healthReport.mpu6050.status);
  mpu["lastUpdate"] = healthReport.mpu6050.lastUpdate;
  mpu["lastValue"] = healthReport.mpu6050.lastValue;
  mpu["errorCount"] = healthReport.mpu6050.errorCount;
  if (strlen(healthReport.mpu6050.errorMessage) > 0) {
    mpu["error"] = healthReport.mpu6050.errorMessage;
  }
  
  // DHT22 Temperature/Humidity
  JsonObject dht = sensors.createNestedObject("dht22");
  dht["name"] = "Temp/Humidity";
  dht["status"] = getStatusString(healthReport.dht22.status);
  dht["lastUpdate"] = healthReport.dht22.lastUpdate;
  dht["lastValue"] = healthReport.dht22.lastValue;
  dht["errorCount"] = healthReport.dht22.errorCount;
  if (strlen(healthReport.dht22.errorMessage) > 0) {
    dht["error"] = healthReport.dht22.errorMessage;
  }
  
  // BH1750 Light Sensor
  JsonObject bh = sensors.createNestedObject("bh1750");
  bh["name"] = "Light Sensor";
  bh["status"] = getStatusString(healthReport.bh1750.status);
  bh["lastUpdate"] = healthReport.bh1750.lastUpdate;
  bh["lastValue"] = healthReport.bh1750.lastValue;
  bh["errorCount"] = healthReport.bh1750.errorCount;
  if (strlen(healthReport.bh1750.errorMessage) > 0) {
    bh["error"] = healthReport.bh1750.errorMessage;
  }
  
  // MFRC522 RFID
  JsonObject rfid = sensors.createNestedObject("mfrc522");
  rfid["name"] = "RFID Reader";
  rfid["status"] = getStatusString(healthReport.mfrc522.status);
  rfid["lastUpdate"] = healthReport.mfrc522.lastUpdate;
  rfid["lastValue"] = healthReport.mfrc522.lastValue;
  rfid["errorCount"] = healthReport.mfrc522.errorCount;
  if (strlen(healthReport.mfrc522.errorMessage) > 0) {
    rfid["error"] = healthReport.mfrc522.errorMessage;
  }
  
  // NEO-6M GPS
  JsonObject gps = sensors.createNestedObject("neo6m");
  gps["name"] = "GPS Module";
  gps["status"] = getStatusString(healthReport.neo6m.status);
  gps["lastUpdate"] = healthReport.neo6m.lastUpdate;
  gps["lastValue"] = healthReport.neo6m.lastValue;
  gps["errorCount"] = healthReport.neo6m.errorCount;
  if (strlen(healthReport.neo6m.errorMessage) > 0) {
    gps["error"] = healthReport.neo6m.errorMessage;
  }
  
  // Serialize and send
  String jsonString;
  serializeJson(doc, jsonString);
  
  Serial.printf("🏥 [DEBUG] JSON created (length: %d): %s\n", jsonString.length(), jsonString.c_str());
  
  Serial.println("🏥 [DEBUG] Sending health data via BLE...");
  BLEManager::sendLineImmediate("HEALTH:%s", jsonString.c_str());
  Serial.println("🏥 [DEBUG] Health data sent via BLE successfully");
}

void SensorHealthManager::sendHealthReportImmediate() {
  Serial.println("🏥 [DEBUG] sendHealthReportImmediate() called - bypassing rate limiting");
  
  if (!BLEManager::isConnected()) {
    Serial.println("🏥 [DEBUG] BLE not connected - cannot send health report");
    return;
  }
  
  Serial.println("🏥 [DEBUG] BLE connected - proceeding with immediate health report generation");
  
  // Create JSON health report
  DynamicJsonDocument doc(1024);
  doc["type"] = "sensor_health";
  doc["timestamp"] = millis();
  
  JsonObject sensors = doc.createNestedObject("sensors");
  
  // VL53L1X ToF Sensor
  JsonObject vl53 = sensors.createNestedObject("vl53l1x");
  vl53["name"] = "ToF Distance";
  vl53["status"] = getStatusString(healthReport.vl53l1x.status);
  vl53["lastUpdate"] = healthReport.vl53l1x.lastUpdate;
  vl53["lastValue"] = healthReport.vl53l1x.lastValue;
  vl53["errorCount"] = healthReport.vl53l1x.errorCount;
  if (strlen(healthReport.vl53l1x.errorMessage) > 0) {
    vl53["error"] = healthReport.vl53l1x.errorMessage;
  }
  
  // MPU6050 IMU
  JsonObject mpu = sensors.createNestedObject("mpu6050");
  mpu["name"] = "IMU";
  mpu["status"] = getStatusString(healthReport.mpu6050.status);
  mpu["lastUpdate"] = healthReport.mpu6050.lastUpdate;
  mpu["lastValue"] = healthReport.mpu6050.lastValue;
  mpu["errorCount"] = healthReport.mpu6050.errorCount;
  if (strlen(healthReport.mpu6050.errorMessage) > 0) {
    mpu["error"] = healthReport.mpu6050.errorMessage;
  }
  
  // DHT22 Temperature/Humidity
  JsonObject dht = sensors.createNestedObject("dht22");
  dht["name"] = "Temp/Humidity";
  dht["status"] = getStatusString(healthReport.dht22.status);
  dht["lastUpdate"] = healthReport.dht22.lastUpdate;
  dht["lastValue"] = healthReport.dht22.lastValue;
  dht["errorCount"] = healthReport.dht22.errorCount;
  if (strlen(healthReport.dht22.errorMessage) > 0) {
    dht["error"] = healthReport.dht22.errorMessage;
  }
  
  // BH1750 Light Sensor
  JsonObject bh = sensors.createNestedObject("bh1750");
  bh["name"] = "Light Sensor";
  bh["status"] = getStatusString(healthReport.bh1750.status);
  bh["lastUpdate"] = healthReport.bh1750.lastUpdate;
  bh["lastValue"] = healthReport.bh1750.lastValue;
  bh["errorCount"] = healthReport.bh1750.errorCount;
  if (strlen(healthReport.bh1750.errorMessage) > 0) {
    bh["error"] = healthReport.bh1750.errorMessage;
  }
  
  // MFRC522 RFID
  JsonObject rfid = sensors.createNestedObject("mfrc522");
  rfid["name"] = "RFID Reader";
  rfid["status"] = getStatusString(healthReport.mfrc522.status);
  rfid["lastUpdate"] = healthReport.mfrc522.lastUpdate;
  rfid["lastValue"] = healthReport.mfrc522.lastValue;
  rfid["errorCount"] = healthReport.mfrc522.errorCount;
  if (strlen(healthReport.mfrc522.errorMessage) > 0) {
    rfid["error"] = healthReport.mfrc522.errorMessage;
  }
  
  // NEO-6M GPS
  JsonObject gps = sensors.createNestedObject("neo6m");
  gps["name"] = "GPS Module";
  gps["status"] = getStatusString(healthReport.neo6m.status);
  gps["lastUpdate"] = healthReport.neo6m.lastUpdate;
  gps["lastValue"] = healthReport.neo6m.lastValue;
  gps["errorCount"] = healthReport.neo6m.errorCount;
  if (strlen(healthReport.neo6m.errorMessage) > 0) {
    gps["error"] = healthReport.neo6m.errorMessage;
  }
  
  // Serialize JSON and send via BLE
  String jsonString;
  serializeJson(doc, jsonString);
  Serial.printf("🏥 [DEBUG] Immediate JSON created (length: %d): %s\n", jsonString.length(), jsonString.c_str());
  
  // Create the full message with HEALTH prefix
  String fullMessage = "HEALTH:" + jsonString;
  
  Serial.println("🏥 [DEBUG] Sending immediate health data via BLE using sendLargeData...");
  BLEManager::sendLargeData(fullMessage.c_str());
  Serial.println("🏥 [DEBUG] Immediate health data sent via BLE successfully");
}

void SensorHealthManager::printHealthStatus() {
  Serial.println("\n🏥 Sensor Health Status:");
  Serial.println("═══════════════════════════════════════════════════════════════");
  
  const char* sensors[] = {"vl53l1x", "mpu6050", "dht22", "bh1750", "mfrc522", "neo6m"};
  const char* names[] = {"ToF Distance", "IMU", "Temp/Humidity", "Light Sensor", "RFID Reader", "GPS Module"};
  
  for (int i = 0; i < 6; i++) {
    SensorHealthData* sensor = getSensorData(sensors[i]);
    if (sensor) {
      const char* statusIcon = (sensor->status == SENSOR_OK) ? "✅" : "❌";
      uint32_t timeSinceUpdate = millis() - sensor->lastUpdate;
      
      Serial.printf("%s %-15s: %s (Last: %s, %lu ms ago, Errors: %lu)\n",
                   statusIcon, names[i], getStatusString(sensor->status),
                   sensor->lastValue, timeSinceUpdate, sensor->errorCount);
      
      if (strlen(sensor->errorMessage) > 0) {
        Serial.printf("   └─ Error: %s\n", sensor->errorMessage);
      }
    }
  }
  
  Serial.println("═══════════════════════════════════════════════════════════════");
}