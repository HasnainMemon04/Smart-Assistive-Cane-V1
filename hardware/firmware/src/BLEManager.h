#pragma once
#ifndef BLEMANAGER_H
#define BLEMANAGER_H

#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLE2902.h>
#include "SensorData.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"

// BLE Configuration - exact same UUIDs as sketch_jul9a
#define SVC_UUID "0000ABCD-0000-1000-8000-00805F9B34FB"
#define CHR_UUID "0000BEEF-0000-1000-8000-00805F9B34FB"  // RX (ESP32 sends data to app)
#define CHR_TX_UUID "0000FEED-0000-1000-8000-00805F9B34FB"  // TX (ESP32 receives commands from app)

// Queue configuration for high-speed sensor data
#define BLE_QUEUE_SIZE 20
#define BLE_TASK_STACK_SIZE 4096
#define BLE_TASK_PRIORITY 1
#define BLE_TASK_CORE 0  // Run on Core 0 (Core 1 for main sensors)

// BLE packet structure for efficient queuing
struct BLEPacket {
  char data[64];  // Pre-formatted BLE message
  uint8_t length;
  uint32_t timestamp;
};

class BLEManager {
private:
  static BLEServer* pServer;
  static BLECharacteristic* pChr;  // RX characteristic (ESP32 sends data)
  static BLECharacteristic* pTxChr;  // TX characteristic (ESP32 receives commands)
  static bool clientConnected;
  static uint32_t connectedAt;
  
  // Step tracking for change detection
  static uint32_t lastSentStepCount;
  
  // FreeRTOS components for non-blocking operation
  static QueueHandle_t bleQueue;
  static TaskHandle_t bleTaskHandle;
  static SemaphoreHandle_t bleMutex;
  
  // BLE Server Callbacks
  class CaneServerCallbacks;
  
  // Helper functions - optimized for high-speed operation
  static float rf(float a, float b);
  static int ri(int a, int b);
  
  // FreeRTOS task for BLE transmission
  static void bleTransmissionTask(void* parameter);
  
  // Internal transmission functions
  static bool transmitPacket(const BLEPacket& packet);
  static void flushQueue();
  
public:
  // Initialization
  static void init();
  static void cleanup();
  
  // Public messaging functions
  static void sendLineImmediate(const char* fmt, ...);
  static void sendLargeData(const char* data);  // For large data like health JSON
  static void queueBLEMessage(const char* fmt, ...);
  
  // High-speed, non-blocking data transmission
  static void sendBLEData(const SensorData& s);
  static void sendBLEDataFast(const SensorData& s);  // Optimized version
  static void sendStepUpdateIfChanged(uint32_t currentStepCount);  // Send step data only when changed
  static void sendRadarLiveData(int angle, int distance);  // For real-time radar data
  
  // Status queries
  static bool isConnected();
  static uint32_t getQueuedPackets();
  static uint32_t getDroppedPackets();
  
  // Performance monitoring
  static void printStats();
  
  // Command processing
  static void processCommand(const String& command);  // Process commands from app
  
};

#endif // BLEMANAGER_H