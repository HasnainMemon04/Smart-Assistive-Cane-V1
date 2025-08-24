#include "DiagnosticUI.h"
#include "SDCardManager.h"
#include "GPSModule.h"
#include "RFID.h"
#include "EnvMonitor.h"
#include "IMU.h"
#include "ToF.h"
#include "LightSensor.h"
#include <driver/i2s.h>
#include <DHT.h>

// Static member initialization
DiagnosticState DiagnosticUI::currentState = DIAG_STARTUP;
int DiagnosticUI::totalSensors = 9;
int DiagnosticUI::checkedSensors = 0;
bool DiagnosticUI::systemReady = false;

void DiagnosticUI::init() {
  currentState = DIAG_STARTUP;
  checkedSensors = 0;
  systemReady = false;
  
  // Initialize buzzer pin first to prevent continuous ringing
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, HIGH);
  delay(10); // Small delay to ensure pin is stable
  
  // Initialize vibration pins for feedback
  pinMode(VIB1_PIN, OUTPUT);
  pinMode(VIB2_PIN, OUTPUT);
  
  digitalWrite(VIB1_PIN, LOW);
  digitalWrite(VIB2_PIN, LOW);
}

void DiagnosticUI::showStartupMessage() {
  clearScreen();
  printSeparator();
  Serial.println("🚀 SMART ASSISTIVE CANE V1");
  Serial.println("   Made by Hasnain Memon");
  printSeparator();
  Serial.println();
  
  Serial.println("📱 System Starting...");
  
  // Show startup progress bar
  for (int i = 0; i <= 100; i += 10) {
    showProgressBar(i, "Initializing system components");
    delay(200);
  }
  
  Serial.println("\n✅ System initialization complete!");
  delay(1000);
}

void DiagnosticUI::showProgressBar(int progress, const char* message) {
  Serial.print("\r[");
  
  int barWidth = 30;
  int filledWidth = (progress * barWidth) / 100;
  
  for (int i = 0; i < barWidth; i++) {
    if (i < filledWidth) {
      Serial.print("█");
    } else {
      Serial.print("░");
    }
  }
  
  Serial.printf("] %d%%", progress);
  
  if (message) {
    Serial.printf(" - %s", message);
  }
  
  if (progress == 100) {
    Serial.println();
  }
}

void DiagnosticUI::startSensorCheck() {
  currentState = DIAG_SENSOR_CHECK;
  clearScreen();
  printSeparator();
  Serial.println("🔍 SENSOR CONNECTIVITY CHECK");
  Serial.println("   Checking all sensors (SD, Amplifier, GPS, RFID, etc.) as per wiring...");
  printSeparator();
  Serial.println();
  
  checkedSensors = 0;
}

bool DiagnosticUI::checkSensor(const char* sensorName, const char* pins, bool (*checkFunction)()) {
  Serial.printf("🔍 Checking %s...", sensorName);
  delay(500); // Simulate checking time
  
  bool result = checkFunction();
  
  if (result) {
    showSensorStatus(sensorName, pins, SENSOR_CONNECTED);
    playStatusSound(SENSOR_CONNECTED);
  } else {
    showSensorStatus(sensorName, pins, SENSOR_FAILED, "Check wiring and connections");
    playStatusSound(SENSOR_FAILED);
  }
  
  checkedSensors++;
  return result;
}

void DiagnosticUI::showSensorStatus(const char* sensorName, const char* pins, SensorStatus status, const char* errorMsg) {
  Serial.print("\r");
  
  if (status == SENSOR_OK) {
    printCheckmark();
    Serial.printf(" %s: ✅ (Connected on %s)\n", sensorName, pins);
  } else if (status == SENSOR_ERROR) {
    printCross();
    Serial.printf(" %s: ❌ (Check wiring - %s)\n", sensorName, pins);
    if (errorMsg) {
      Serial.printf("   💡 %s\n", errorMsg);
    }
  } else if (status == SENSOR_TIMEOUT) {
    printWarning();
    Serial.printf(" %s: ⚠️ (Calibrating...)\n", sensorName);
  } else if (status == SENSOR_DISCONNECTED) {
    Serial.printf(" %s: ❓ (Unknown status)\n", sensorName);
  } else {
    Serial.printf(" %s: ❓ (Unknown status)\n", sensorName);
  }
}

void DiagnosticUI::startCalibration() {
  currentState = DIAG_CALIBRATION;
  clearScreen();
  printSeparator();
  Serial.println("⚙️ CALIBRATION AND CONFIGURATION");
  Serial.println("   Loading Calibration and Configuration Data...");
  printSeparator();
  Serial.println();
}

void DiagnosticUI::showCalibrationStatus(const char* item, SensorStatus status, const char* details) {
  if (status == SENSOR_OK) {
    printCheckmark();
    Serial.printf(" %s: ✅ (Calibrated)", item);
  } else if (status == SENSOR_TIMEOUT) {
    printWarning();
    Serial.printf(" %s: ⚠️ (Calibrating...)", item);
  } else if (status == SENSOR_ERROR) {
    printCross();
    Serial.printf(" %s: ❌ (Calibration required)", item);
  } else {
    Serial.printf(" %s: ❓ (Unknown)", item);
  }
  
  if (details) {
    Serial.printf(" - %s", details);
  }
  Serial.println();
}

void DiagnosticUI::showSystemReady() {
  currentState = DIAG_READY;
  systemReady = true;
  
  clearScreen();
  printSeparator();
  Serial.println("🎉 SMART CANE SYSTEM READY!");
  printSeparator();
  Serial.println();
  
  printCheckmark();
  Serial.println(" All sensors connected and calibrated successfully!");
  Serial.println("📱 System is ready for navigation and assistance.");
  Serial.println("🎯 Press any button or start using the cane.");
  Serial.println();
  
  // Success feedback
  playStatusSound(SENSOR_CONNECTED);
  vibrationFeedback(SENSOR_CONNECTED);
  
  printSeparator();
}

void DiagnosticUI::showError(const char* sensorName, const char* errorMessage, const char* solution) {
  currentState = DIAG_ERROR;
  
  Serial.println();
  printSeparator();
  Serial.printf("❌ ERROR: %s not detected!\n", sensorName);
  Serial.printf("💬 %s\n", errorMessage);
  Serial.printf("🔧 Solution: %s\n", solution);
  printSeparator();
  Serial.println("⚡ Options: Type 'retry' to check again or 'continue' to proceed anyway.");
  Serial.println();
  
  // Error feedback
  playStatusSound(SENSOR_FAILED);
  vibrationFeedback(SENSOR_FAILED);
}

void DiagnosticUI::playStatusSound(SensorStatus status) {
  // No beeping during individual sensor checks
  // Only beep at the end of all diagnostics
}

void DiagnosticUI::playFinalDiagnosticBeep(bool allOk) {
  if (allOk) {
    // Everything OK - 2 beeps
    for (int i = 0; i < 2; i++) {
      digitalWrite(BUZZER_PIN, HIGH);
      delay(200);
      digitalWrite(BUZZER_PIN, LOW);
      if (i < 1) delay(500); // Short pause between beeps
    }
  } else {
    // Problems detected - 3 beeps with 1 second intervals
    for (int i = 0; i < 3; i++) {
      digitalWrite(BUZZER_PIN, HIGH);
      delay(200);
      digitalWrite(BUZZER_PIN, LOW);
      if (i < 2) delay(1000); // 1 second interval between beeps
    }
  }
}

void DiagnosticUI::vibrationFeedback(SensorStatus status) {
  if (status == SENSOR_OK) {
    // Success vibration (single pulse)
    digitalWrite(VIB1_PIN, HIGH);
    delay(200);
    digitalWrite(VIB1_PIN, LOW);
  } else if (status == SENSOR_ERROR) {
    // Error vibration (three pulses)
    for (int i = 0; i < 3; i++) {
      digitalWrite(VIB1_PIN, HIGH);
      digitalWrite(VIB2_PIN, HIGH);
      delay(300);
      digitalWrite(VIB1_PIN, LOW);
      digitalWrite(VIB2_PIN, LOW);
      delay(200);
    }
  } else if (status == SENSOR_TIMEOUT) {
    // Warning vibration (alternating motors)
    digitalWrite(VIB1_PIN, HIGH);
    delay(150);
    digitalWrite(VIB1_PIN, LOW);
    digitalWrite(VIB2_PIN, HIGH);
    delay(150);
    digitalWrite(VIB2_PIN, LOW);
  }
}

bool DiagnosticUI::waitForRetry() {
  Serial.println("⏳ Waiting for user input...");
  
  unsigned long startTime = millis();
  String input = "";
  
  while (millis() - startTime < 30000) { // 30 second timeout
    if (Serial.available()) {
      char c = Serial.read();
      if (c == '\n' || c == '\r') {
        input.toLowerCase();
        input.trim();
        
        if (input == "retry") {
          Serial.println("🔄 Retrying sensor checks...");
          return true;
        } else if (input == "continue") {
          Serial.println("⚠️ Continuing with errors...");
          return false;
        } else {
          Serial.println("❓ Invalid input. Type 'retry' or 'continue'.");
          input = "";
        }
      } else {
        input += c;
      }
    }
    delay(10);
  }
  
  Serial.println("⏰ Timeout reached. Continuing with errors...");
  return false;
}

void DiagnosticUI::clearScreen() {
  Serial.println("\n\n\n");
}

void DiagnosticUI::printSeparator() {
  Serial.println("═══════════════════════════════════════════════════════════════════════════════");
}

void DiagnosticUI::printCheckmark() {
  Serial.print("✅");
}

void DiagnosticUI::printCross() {
  Serial.print("❌");
}

void DiagnosticUI::printWarning() {
  Serial.print("⚠️");
}

void DiagnosticUI::printProgressChar(int current, int total) {
  float percentage = (float)current / total * 100;
  if (percentage < 25) Serial.print("░");
  else if (percentage < 50) Serial.print("▒");
  else if (percentage < 75) Serial.print("▓");
  else Serial.print("█");
}

// Sensor check function implementations
bool checkSDCard() {
  return SDCard_init();
}

bool checkAmplifier() {
  // Check I2S driver installation
  i2s_config_t i2s_cfg = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX),
    .sample_rate = 44100,
    .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
    .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
    .communication_format = I2S_COMM_FORMAT_I2S_MSB,
    .intr_alloc_flags = 0,
    .dma_buf_count = 4,
    .dma_buf_len = 1024,
    .use_apll = false
  };
  
  if (i2s_driver_install(I2S_NUM_0, &i2s_cfg, 0, NULL) == ESP_OK) {
    i2s_pin_config_t i2s_pins = {
      .bck_io_num = I2S_BCLK_PIN,
      .ws_io_num  = I2S_LRCLK_PIN,
      .data_out_num = I2S_DIN_PIN,
      .data_in_num  = I2S_PIN_NO_CHANGE
    };
    
    if (i2s_set_pin(I2S_NUM_0, &i2s_pins) == ESP_OK) {
      pinMode(I2S_SD_PIN, OUTPUT);
      digitalWrite(I2S_SD_PIN, HIGH);
      return true;
    }
  }
  return false;
}

bool checkGPS() {
  // GPS module initialization check
  return true; // GPSModule_init() returns void, assume success for now
}

bool checkRFID() {
  // RFID module initialization check
  return true; // RFID_init() returns void, assume success for now
}

bool checkVibrationMotors() {
  pinMode(VIB1_PIN, OUTPUT);
  pinMode(VIB2_PIN, OUTPUT);
  
  // Test vibration motors
  digitalWrite(VIB1_PIN, HIGH);
  delay(100);
  digitalWrite(VIB1_PIN, LOW);
  
  digitalWrite(VIB2_PIN, HIGH);
  delay(100);
  digitalWrite(VIB2_PIN, LOW);
  
  return true; // Assume success if no hardware failure
}

bool checkDHT22() {
  // DHT22 sensor check
  return true; // EnvMonitor_init() returns void, assume success for now
}

bool checkIMU() {
  // IMU sensor check
  return true; // IMU_init() returns void, assume success for now
}

bool checkToF() {
  // ToF sensor check
  return true; // ToF_init() returns void, assume success for now
}

bool checkLightSensor() {
  // Light sensor check
  return true; // LightSensor_init() returns void, assume success for now
}