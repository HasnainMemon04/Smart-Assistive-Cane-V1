#include "BLEManager.h"
#include <esp_random.h>

// Static member initialization
BLEServer* BLEManager::pServer = nullptr;
BLECharacteristic* BLEManager::pChr = nullptr;
bool BLEManager::clientConnected = false;
uint32_t BLEManager::connectedAt = 0;
bool BLEManager::bleInitialized = false;

// BLE Server Callbacks
class BLEManager::CaneServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) override {
    BLEManager::clientConnected = true;
    BLEManager::connectedAt = millis();
    Serial.println("📱 Phone CONNECTED via BLE");
  }
  
  void onDisconnect(BLEServer* pServer) override {
    BLEManager::clientConnected = false;
    Serial.println("📱 Phone DISCONNECTED from BLE");
    // Restart advertising
    BLEManager::startAdvertising();
  }
};

// Helper functions
float BLEManager::rf(float a, float b) { 
  return a + (b - a) * esp_random() / float(UINT32_MAX); 
}

int BLEManager::ri(int a, int b) { 
  return a + esp_random() % (b - a + 1); 
}

void BLEManager::sendLine(const char* fmt, ...) {
  if (!clientConnected || millis() - connectedAt < 500) return; // Guard: must be connected AND grace period elapsed

  char ln[32];
  va_list args;
  va_start(args, fmt);
  const int n = vsnprintf(ln, sizeof(ln), fmt, args);
  va_end(args);
  
  if (n <= 0 || n > 20) return;

  pChr->setValue((uint8_t*)ln, n);
  pChr->notify();
  delay(40); // watchdog-safe gap
}

void BLEManager::init() {
  if (bleInitialized) return;
  
  Serial.println("📱 Initializing BLE...");
  
  // Initialize BLE
  BLEDevice::init("SmartCane-ESP32");
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new CaneServerCallbacks());
  
  BLEService *pService = pServer->createService(SVC_UUID);
  pChr = pService->createCharacteristic(
           CHR_UUID,
           BLECharacteristic::PROPERTY_READ |
           BLECharacteristic::PROPERTY_NOTIFY);
  pChr->addDescriptor(new BLE2902()); // CCCD
  
  pService->start();
  
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SVC_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->start();
  
  bleInitialized = true;
  Serial.println("📱 BLE advertising started - SmartCane-ESP32");
}

void BLEManager::update() {
  // BLE updates are handled automatically by the ESP32 BLE stack
  // This function can be used for periodic BLE maintenance if needed
}

void BLEManager::sendBLEData(const SensorData& s) {
  if (!clientConnected || millis() - connectedAt < 500) return;

  // Send temperature and humidity
  sendLine("TEMP:%.1f\n", s.temperature);
  sendLine("HUM:%.1f\n", s.humidity);
  
  // Send obstacle distance (ToF)
  sendLine("OBST:%d\n", (int)s.tofDistance);
  
  // Send battery level (simulated full to empty over time)
  static uint32_t batteryStartTime = millis();
  static int batteryLevel = 4200; // Start at full (4.2V)
  
  // Gradually decrease battery over time (simulate usage)
  uint32_t elapsedMinutes = (millis() - batteryStartTime) / 60000; // Convert to minutes
  int targetLevel = 4200 - (elapsedMinutes * 2); // Decrease 2mV per minute
  
  // Add some randomness to make it more realistic
  int randomVariation = ri(-50, 50);
  batteryLevel = constrain(targetLevel + randomVariation, 3200, 4200); // Keep between 3.2V-4.2V
  
  sendLine("BATTmV:%d\n", batteryLevel);
  
  // Send RFID zone and UID for continuous sensing
  sendLine("RFID:%d\n", s.currentRoom);
  if (strlen(s.rfidUID) > 0) {
    sendLine("RFIDUID:%s\n", s.rfidUID);
  }
  
  // Send step count
  sendLine("STEP:%d\n", s.dailySteps);
  
  // Send light level
  sendLine("LUX:%d\n", (int)s.lightLux);
  
  // Send fall detection (from IMU) - this would need to be implemented in IMU
  bool fallDetected = false; // This would come from IMU fall detection
  sendLine("FALL:%d\n", fallDetected ? 1 : 0);
  
  // Send GPS coordinates if available
  if (s.gpsLat != 0 && s.gpsLon != 0) {
    sendLine("GPS:%.2f,%.2f\n", s.gpsLat, s.gpsLon);
  }
  
  // Send additional sensor data
  sendLine("PITCH:%.1f\n", s.imuPitch);
  sendLine("ROLL:%.1f\n", s.imuRoll);
  sendLine("YAW:%.1f\n", s.imuYaw);
  sendLine("ALT:%.1f\n", s.gpsAlt);
  sendLine("SPEED:%.1f\n", s.gpsSpeed);
  sendLine("SATS:%d\n", s.gpsSatellites);
  sendLine("TOTAL:%d\n", s.totalSteps);
  
  // Send feedback mode
  int feedbackMode = s.feedbackMode;
  sendLine("MODE:%d\n", feedbackMode);
  
  // Send room name and status for continuous sensing
  if (s.currentRoom > 0 && strlen(s.currentRoomName) > 0) {
    char roomMsg[32];
    snprintf(roomMsg, sizeof(roomMsg), "ROOM:%s", s.currentRoomName);
    sendLine("%s\n", roomMsg);
  }
  
  // Send RFID sensing status
  sendLine("RFIDSENSING:%s\n", (strlen(s.rfidUID) > 0) ? "ACTIVE" : "INACTIVE");
}

bool BLEManager::isConnected() {
  return clientConnected;
}

uint32_t BLEManager::getConnectionTime() {
  return connectedAt;
}

void BLEManager::startAdvertising() {
  if (pServer) {
    BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
    pAdvertising->start();
    Serial.println("📱 BLE advertising restarted");
  }
}

void BLEManager::stopAdvertising() {
  if (pServer) {
    BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
    pAdvertising->stop();
    Serial.println("📱 BLE advertising stopped");
  }
} 