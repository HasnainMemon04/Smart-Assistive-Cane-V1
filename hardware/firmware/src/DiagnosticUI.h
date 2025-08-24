#pragma once
#ifndef DIAGNOSTIC_UI_H
#define DIAGNOSTIC_UI_H

#include <Arduino.h>
#include "Pins.h"
#include "SensorHealth.h"

// Diagnostic UI States
enum DiagnosticState {
  DIAG_STARTUP,
  DIAG_SENSOR_CHECK,
  DIAG_CALIBRATION,
  DIAG_READY,
  DIAG_ERROR
};

// Additional sensor status for diagnostic UI (extending SensorHealth.h)
#define SENSOR_CONNECTED SENSOR_OK
#define SENSOR_FAILED SENSOR_ERROR
#define SENSOR_CALIBRATING SENSOR_TIMEOUT  // Reuse for calibrating state
#define SENSOR_CALIBRATED SENSOR_OK        // Reuse for calibrated state
#define SENSOR_UNKNOWN SENSOR_DISCONNECTED // Reuse for unknown state

// Sensor Information Structure
struct SensorInfo {
  const char* name;
  const char* pins;
  SensorStatus status;
  const char* errorMessage;
};

class DiagnosticUI {
public:
  static void init();
  static void showStartupMessage();
  static void showProgressBar(int progress, const char* message = nullptr);
  static void startSensorCheck();
  static bool checkSensor(const char* sensorName, const char* pins, bool (*checkFunction)());
  static void showSensorStatus(const char* sensorName, const char* pins, SensorStatus status, const char* errorMsg = nullptr);
  static void startCalibration();
  static void showCalibrationStatus(const char* item, SensorStatus status, const char* details = nullptr);
  static void showSystemReady();
  static void showError(const char* sensorName, const char* errorMessage, const char* solution);
  static void playStatusSound(SensorStatus status);
  static void vibrationFeedback(SensorStatus status);
  static void playFinalDiagnosticBeep(bool allOk);
  static bool waitForRetry();
  static void clearScreen();
  static void printSeparator();
  
private:
  static DiagnosticState currentState;
  static int totalSensors;
  static int checkedSensors;
  static bool systemReady;
  static void printCheckmark();
  static void printCross();
  static void printWarning();
  static void printProgressChar(int current, int total);
};

// Sensor check function declarations
bool checkSDCard();
bool checkAmplifier();
bool checkGPS();
bool checkRFID();
bool checkVibrationMotors();
bool checkDHT22();
bool checkIMU();
bool checkToF();
bool checkLightSensor();

#endif // DIAGNOSTIC_UI_H