#pragma once
#ifndef FEEDBACKMANAGER_H
#define FEEDBACKMANAGER_H

#include <Arduino.h>
#include "Pins.h"
#include "SensorData.h"

class FeedbackManager {
private:
  static uint32_t buttonPressStart;
  static bool buttonPressed;
  static bool modeChanged;
  
  // ToF Mode Button Variables
  static uint32_t tofButtonPressStart;
  static bool tofButtonPressed;
  static bool tofModeChanged;
  
public:
  static void init();
  static void update(SensorData* sensorData);
  static void cycleMode(SensorData* sensorData);
  static void cycleToFMode();
  static const char* getModeName(uint8_t mode);
  static bool shouldUseBuzzer(uint8_t mode);
  static bool shouldUseVibration(uint8_t mode);
};

#endif // FEEDBACKMANAGER_H 