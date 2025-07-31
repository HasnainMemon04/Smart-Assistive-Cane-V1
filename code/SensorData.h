#pragma once
#ifndef SENSORDATA_H
#define SENSORDATA_H

#include <Arduino.h>

// Feedback modes
#define FEEDBACK_MODE_BOTH 0      // Both buzzer and vibration
#define FEEDBACK_MODE_BUZZER 1    // Buzzer only
#define FEEDBACK_MODE_VIBRATION 2 // Vibration only
#define FEEDBACK_MODE_NONE 3      // No feedback

// Indoor zoning constants
#define MAX_ROOMS 4
#define ROOM_NAME_LENGTH 16

struct SensorData {
  float temperature = 0;
  float humidity = 0;
  float heatIndex = 0;
  float dewPoint = 0;
  float lightLux = 0;
  char lightEnvironment[32] = "";
  float imuPitch = 0, imuRoll = 0, imuYaw = 0;
  float tofDistance = 0;
  char rfidUID[30] = "";
  double gpsLat = 0, gpsLon = 0;
  double gpsAlt = 0;
  float gpsSpeed = 0;
  int gpsSatellites = 0;
  uint32_t dailySteps = 0;
  uint32_t totalSteps = 0;
  uint8_t lastResetDay = 0;
  uint8_t lastResetMonth = 0;
  uint16_t lastResetYear = 0;
  uint8_t feedbackMode = FEEDBACK_MODE_BOTH; // Current feedback mode
  
  // Indoor zoning system
  uint8_t currentRoom = 0;        // 0 = No room, 1-4 = Room 1-4
  char currentRoomName[ROOM_NAME_LENGTH] = "";
  bool roomChanged = false;       // Flag to indicate room change
};

#endif // SENSORDATA_H 