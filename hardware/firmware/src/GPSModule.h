#pragma once
#ifndef GPSMODULE_H
#define GPSMODULE_H
#include "SensorData.h"

// GPS Configuration and Status
struct GPSConfig {
  uint16_t updateRate;        // Navigation update rate (1-5Hz)
  bool enableSBAS;            // SBAS (WAAS/EGNOS) support
  bool enableMultiGNSS;       // Multi-constellation support
  uint8_t dynamicModel;       // Dynamic platform model
  uint16_t elevationMask;     // Satellite elevation mask (degrees)
  // Jamming detection completely removed for performance
};

struct GPSStatus {
  bool isFixed;
  uint8_t fixType;            // 0=No fix, 2=2D, 3=3D
  uint8_t satellitesUsed;
  uint8_t satellitesVisible;
  float hdop;
  float vdop;
  float pdop;
  float accuracy;             // Position accuracy (meters)
  int8_t signalStrength;      // Average signal strength
  uint32_t timeToFirstFix;    // TTFF in milliseconds
};

// Enhanced GPS Functions
void GPSModule_init();
void GPSModule_update(SensorData* data);
void GPSModule_setUpdateRate(uint8_t rate);  // 1-5Hz
void GPSModule_enableSBAS(bool enable);
void GPSModule_setDynamicModel(uint8_t model);
void GPSModule_enableAntiJamming(bool enable); // Removed for performance
void GPSModule_performColdStart();
void GPSModule_performWarmStart();
void GPSModule_performHotStart();
GPSStatus GPSModule_getStatus();
GPSConfig GPSModule_getConfig();
void GPSModule_saveConfig();
void GPSModule_loadConfig();
bool GPSModule_isJammingDetected(); // Completely removed, always returns false
float GPSModule_getPositionAccuracy();
uint32_t GPSModule_getTimeToFirstFix();
void GPSModule_enablePowerSaving(bool enable);
void GPSModule_setElevationMask(uint8_t degrees);

#endif