#pragma once
#ifndef BLEMANAGER_H
#define BLEMANAGER_H

#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLE2902.h>
#include <cstdarg>
#include "SensorData.h"

// BLE Configuration
static const char *SVC_UUID = "0000ABCD-0000-1000-8000-00805F9B34FB";
static const char *CHR_UUID = "0000BEEF-0000-1000-8000-00805F9B34FB";

class BLEManager {
private:
  static BLEServer* pServer;
  static BLECharacteristic* pChr;
  static bool clientConnected;
  static uint32_t connectedAt;
  static bool bleInitialized;
  
  // BLE Server Callbacks
  class CaneServerCallbacks;
  
  // Helper functions
  static float rf(float a, float b);
  static int ri(int a, int b);
  static void sendLine(const char* fmt, ...);
  
public:
  // Initialization
  static void init();
  static void update();
  
  // Data transmission
  static void sendBLEData(const SensorData& s);
  
  // Status queries
  static bool isConnected();
  static uint32_t getConnectionTime();
  
  // Connection management
  static void startAdvertising();
  static void stopAdvertising();
};

#endif // BLEMANAGER_H 