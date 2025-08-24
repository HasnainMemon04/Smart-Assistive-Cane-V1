#include "ToF.h"
#include "Pins.h"
#include "BLEManager.h"
#include "SensorHealth.h"
#include <Wire.h>
#include <VL53L1X.h>
#include <ESP32Servo.h>
#include <string.h>

// ============= Sensor Configuration =============
static VL53L1X sensor;
static const uint8_t NUM_SAMPLES = 5;
static uint16_t samples[NUM_SAMPLES];
static uint8_t sampleIndex = 0;
static uint8_t validSamples = 0;
static float filteredDistance = 3500.0; // Initialize to max range
static uint32_t lastModeSwitch = 0;
static uint32_t lastAlert = 0;
static bool alertActive = false;
static uint32_t alertStartTime = 0;
static bool vibActive = false;
static uint8_t adaptiveSpeed = 3; // TEMPORARILY: Only Fast mode enabled (1=conservative, 2=balanced, 3=fast)

// Radar Mode Variables - Optimized for Real-Time Performance
static Servo scanServo;
// Full 180-degree scan from 0° to 180° with 1-degree increments
static const int NUM_RADAR_ANGLES = 181; // 0° to 180° inclusive
static uint16_t scanData[NUM_RADAR_ANGLES] = {0}; // Distance readings for each angle
static uint32_t lastRadarScan = 0;
static const uint32_t RADAR_SCAN_INTERVAL = 4000; // 4 seconds for full sweep
static bool radarModeActive = false;

// Real-time radar scanning variables
static int currentScanAngle = 0;
static uint32_t lastAngleTime = 0;
static const uint32_t ANGLE_INTERVAL = 22; // 22ms per angle (4000ms / 181 angles)
static bool scanInProgress = false;
static TaskHandle_t radarTaskHandle = nullptr;

// Operating modes
static OperationMode currentMode = SIMPLE_MODE;

// Parameters
static const uint16_t MAX_LONG_DISTANCE_MM = 3500; // 350cm max range
static const uint16_t MIN_DISTANCE_MM = 30;
static const uint16_t CRITICAL_DISTANCE_MM = 300;  // 30cm critical (reverted)
static const uint16_t WARNING_DISTANCE_MM = 700;   // 70cm warning (reverted)
static const uint32_t MODE_SWITCH_DELAY = 2000;
static const uint16_t LONG_RANGE_TIMING = 33;
static const uint16_t SHORT_RANGE_TIMING = 15;
static const uint16_t STAIR_TIMING = 20;
static const float ALPHA_MIN = 0.07f;  // Balanced for speed and safety
static const float ALPHA_MID = 0.22f;  // Balanced for speed and safety
static const float ALPHA_MAX = 0.35f;  // Balanced for speed and safety
static const float CHANGE_THRESHOLD_SMALL = 3.0f;  // Reduced for faster response
static const float CHANGE_THRESHOLD_LARGE = 20.0f; // Reduced for faster response

// Buzzer hysteresis to prevent flickering
static uint8_t lastBuzzerLevel = 0;
static const uint16_t BUZZER_HYSTERESIS_CM = 10; // 10cm hysteresis

// ============= Error Detection & Recovery =============
static uint32_t lastValidReading = 0;
static uint16_t consecutiveMaxReadings = 0;
static const uint16_t MAX_CONSECUTIVE_MAX_READINGS = 50; // Changed from 10 to 50
static const uint32_t ERROR_RECOVERY_TIMEOUT = 5000; // 5 seconds

static bool isSensorStuck() {
  return (filteredDistance >= MAX_LONG_DISTANCE_MM - 50) && 
         (consecutiveMaxReadings > MAX_CONSECUTIVE_MAX_READINGS);
}

static void resetSensor() {
  Serial.println("🔄 ToF Sensor Reset - Reinitializing...");
  
  // Stop continuous reading
  sensor.stopContinuous();
  delay(100);
  
  // Reconfigure sensor - call the static function directly
  sensor.setDistanceMode(VL53L1X::Long);
  sensor.setMeasurementTimingBudget(LONG_RANGE_TIMING * 1000);
  sensor.startContinuous(20);
  
  // Reset all variables
  filteredDistance = MAX_LONG_DISTANCE_MM;
  consecutiveMaxReadings = 0;
  lastValidReading = millis();
  
  // Clear sample buffer
  for (uint8_t i = 0; i < NUM_SAMPLES; i++) {
    samples[i] = MAX_LONG_DISTANCE_MM;
  }
  sampleIndex = 0;
  validSamples = 0;
  
  Serial.println("✅ ToF Sensor Reset Complete");
}

// ============= Function Prototypes =============
static void configureSensor(OperationMode mode);
static void updateBuzzer();
static void updateBuffer(uint16_t value);
static uint16_t getMedianDistance();
static void applyStableEMA(float newDistance);
static void checkDistanceAlerts(uint16_t distance, uint32_t currentTime);
static void handleModeSwitching(uint32_t currentTime);
static void printStatus();
static void printCurrentConfig();

// Radar Mode Function Prototypes
static void runRadarScan();
static void radarScanTask(void* parameter);
static uint16_t getQuickDistanceReading();
static void moveServoToAngle(int angle);
static uint16_t getMedianReading(uint16_t readings[], int count);
static void analyzeRadarData();
static const char* getSafestDirection();
static void printRadarResults();

void ToF_init() {
  Wire.begin(I2C_SDA, I2C_SCL, 400000); // Safe for all I2C sensors (BH1750, MPU6050, VL53L1X)
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);
  pinMode(VIB1_PIN, OUTPUT);
  pinMode(VIB2_PIN, OUTPUT);
  digitalWrite(VIB1_PIN, LOW);
  digitalWrite(VIB2_PIN, LOW);
  
  // Initialize servo for radar mode
  scanServo.attach(SERVO_PIN);
  scanServo.write(90); // Center position
  delay(1000); // Allow servo to settle
  
  if (!sensor.init()) {
    Serial.println("Failed to initialize sensor!");
    SensorHealthManager::updateSensorHealth("vl53l1x", SENSOR_INIT_FAILED, nullptr, "VL53L1X init failed");
    unsigned long errorStart = millis();
    bool errorFlag = true;
    while (millis() - errorStart < 10000) {
      // LED error indication removed - unnecessary for blind users
      delay(50);
    }
    errorFlag = false;
    // LED error indication removed - unnecessary for blind users
    return;
  }
  
  // Report successful initialization
  SensorHealthManager::updateSensorHealth("vl53l1x", SENSOR_OK, "VL53L1X initialized");
  configureSensor(SIMPLE_MODE);
  sensor.startContinuous(30);
  for (uint8_t i = 0; i < NUM_SAMPLES; i++) samples[i] = MAX_LONG_DISTANCE_MM;
#ifdef SC_DEBUG_TOF
  Serial.println(F("\nVL53L1X Smart Cane System Initialized"));
  Serial.println(F("========================================"));
  Serial.println(F("Modes: SIMPLE (fixed) / RADAR (scanning)"));
#endif
  printCurrentConfig();
  // Ensure I2C bus is set to 400kHz for all sensors
  Wire.setClock(400000);
}

void ToF_update(SensorData* data) {
  uint32_t currentTime = millis();
  
  // Handle different modes
  if (currentMode == RADAR_MODE) {
    // Ensure radar scanning task is running
    if (radarModeActive && radarTaskHandle == nullptr) {
      runRadarScan(); // Start the real-time radar task
    }
    
    // Keep sensor in continuous mode for real-time readings
    static bool sensorStarted = false;
    if (!sensorStarted) {
      sensor.startContinuous(20); // Fast timing for real-time
      sensorStarted = true;
    }
  } else {
    // Simple Mode: Original fixed ToF logic
    if (sensor.dataReady()) {
      uint16_t rawDist = sensor.read();
      
      // Error Detection: Check for stuck sensor
      if (rawDist >= MAX_LONG_DISTANCE_MM - 50) {
        consecutiveMaxReadings++;
        if (consecutiveMaxReadings > MAX_CONSECUTIVE_MAX_READINGS) {
          Serial.println("⚠️ ToF Sensor appears stuck at max range");
          SensorHealthManager::updateSensorHealth("vl53l1x", SENSOR_ERROR, nullptr, "Sensor stuck at max range");
          if (currentTime - lastValidReading > ERROR_RECOVERY_TIMEOUT) {
            resetSensor();
            return;
          }
        }
      } else {
        consecutiveMaxReadings = 0;
        lastValidReading = currentTime;
        // Report healthy status with current reading
        SensorHealthManager::updateSensorHealth("vl53l1x", SENSOR_OK, String(rawDist).c_str());
      }
      
      updateBuffer(rawDist);
      uint16_t medianDist = getMedianDistance();
      
      // Debug: Show raw vs filtered values with adaptive speed
      static uint32_t lastDebugTime = 0;
      if (currentTime - lastDebugTime > 500) { // Every 500ms
        const char* speedMode[] = {"Conservative", "Balanced", "Fast"};
        Serial.printf("ToF Debug: Raw=%d, Median=%d, Filtered=%.1f, Mode=%s [SIMPLE MODE]\n", 
                      rawDist, medianDist, filteredDistance, speedMode[adaptiveSpeed-1]);
        lastDebugTime = currentTime;
      }
      
      applyStableEMA(medianDist);
      checkDistanceAlerts(static_cast<uint16_t>(filteredDistance), currentTime);
      
      // Extra safety feedback for blind users - alert on rapid changes
      static uint16_t lastDistance = 3500;
      static uint32_t lastChangeTime = 0;
      float distanceChange = fabs(filteredDistance - lastDistance);
      
      if (distanceChange > 100.0f && currentTime - lastChangeTime > 1000) {
        // Significant distance change - provide extra feedback
        Serial.printf("⚠️ Distance change: %.1f cm\n", distanceChange / 10.0f);
        lastChangeTime = currentTime;
      }
      lastDistance = filteredDistance;
    }
  }
  // Non-blocking feedback turn-off
  if (vibActive && millis() - alertStartTime > 100) {
    digitalWrite(VIB1_PIN, LOW);
    digitalWrite(VIB2_PIN, LOW);
    digitalWrite(BUZZER_PIN, LOW);
    vibActive = false;
  }
  if (data) {
    data->tofDistance = filteredDistance;
  }
  updateBuzzer();
  static uint32_t lastPrint = 0;
  if (millis() - lastPrint > 100) {
    printStatus();
    lastPrint = millis();
  }
  // TEMPORARILY DISABLED: Automatic mode switching
  // handleModeSwitching(millis());
  // if (currentMode == STAIR_DETECTION_MODE &&
  //     filteredDistance > 1000 &&
  //     millis() - lastModeSwitch > 5000) {
  //   Serial.println(F("\u21a9\ufe0f Returning to long range mode"));
  //   configureSensor(LONG_RANGE_MODE);
  // }
}

// ============= Sensor Configuration =============
static void configureSensor(OperationMode mode) {
  sensor.stopContinuous();
  delay(10);
  switch (mode) {
    case SIMPLE_MODE:
      sensor.setDistanceMode(VL53L1X::Long);
      sensor.setMeasurementTimingBudget(LONG_RANGE_TIMING * 1000);
      break;
    case RADAR_MODE:
      sensor.setDistanceMode(VL53L1X::Long);
      sensor.setMeasurementTimingBudget(LONG_RANGE_TIMING * 1000);
      break;
  }
  sensor.startContinuous(20);
  currentMode = mode;
  lastModeSwitch = millis();
  Serial.print(F("\n🔄 Switched to mode: "));
  switch (mode) {
    case SIMPLE_MODE: Serial.println(F("SIMPLE (Fixed ToF)")); break;
    case RADAR_MODE: Serial.println(F("RADAR (Servo Scanning)")); break;
  }
}

// ============= Buzzer Control Function =============
static void updateBuzzer() {
  static uint8_t buzzerState = LOW;
  static uint32_t lastChangeTime = 0;
  static int8_t currentLevel = -1;
  const uint32_t currentMillis = millis();
  uint8_t newLevel = 0;
  float distance_cm = filteredDistance / 10.0f;
  
  // Get current feedback mode from SensorData (passed via global or parameter)
  extern SensorData sensorData; // Access the global sensorData
  uint8_t feedbackMode = sensorData.feedbackMode;
  
  // Fallback: If feedbackMode is invalid, default to BOTH
  if (feedbackMode > 2) feedbackMode = 0;
  
  // Debug: Print feedback mode for troubleshooting
  static uint32_t lastDebugTime = 0;
  if (currentMillis - lastDebugTime > 2000) { // Every 2 seconds
    Serial.printf("🔊 Feedback Mode: %d (0=BOTH, 1=BUZZER, 2=VIBRATION)\n", feedbackMode);
    lastDebugTime = currentMillis;
  }
  
  // Determine new level based on distance
  if (distance_cm >= 180.0f) newLevel = 0;
  else if (distance_cm >= 140.0f) newLevel = 1;
  else if (distance_cm >= 100.0f) newLevel = 2;
  else if (distance_cm >= 70.0f) newLevel = 3;
  else if (distance_cm >= 40.0f) newLevel = 4;
  else if (distance_cm >= 30.0f) newLevel = 5;
  else newLevel = 6;
  
  // Apply hysteresis to prevent flickering
  if (newLevel != lastBuzzerLevel) {
    if (newLevel > lastBuzzerLevel) {
      // Moving closer - use lower threshold
      if (distance_cm >= (180.0f - BUZZER_HYSTERESIS_CM)) newLevel = 0;
      else if (distance_cm >= (140.0f - BUZZER_HYSTERESIS_CM)) newLevel = 1;
      else if (distance_cm >= (100.0f - BUZZER_HYSTERESIS_CM)) newLevel = 2;
      else if (distance_cm >= (70.0f - BUZZER_HYSTERESIS_CM)) newLevel = 3;
      else if (distance_cm >= (40.0f - BUZZER_HYSTERESIS_CM)) newLevel = 4;
      else if (distance_cm >= (30.0f - BUZZER_HYSTERESIS_CM)) newLevel = 5;
      else newLevel = 6;
    } else {
      // Moving away - use higher threshold
      if (distance_cm >= (180.0f + BUZZER_HYSTERESIS_CM)) newLevel = 0;
      else if (distance_cm >= (140.0f + BUZZER_HYSTERESIS_CM)) newLevel = 1;
      else if (distance_cm >= (100.0f + BUZZER_HYSTERESIS_CM)) newLevel = 2;
      else if (distance_cm >= (70.0f + BUZZER_HYSTERESIS_CM)) newLevel = 3;
      else if (distance_cm >= (40.0f + BUZZER_HYSTERESIS_CM)) newLevel = 4;
      else if (distance_cm >= (30.0f + BUZZER_HYSTERESIS_CM)) newLevel = 5;
      else newLevel = 6;
    }
  }
  
  static const uint16_t onTimes[7] = {0, 920, 520, 320, 120, 40, 80};
  static const uint16_t offTimes[7] = {0, 100, 100, 100, 100, 100, 1920};
  
  if (newLevel != currentLevel) {
    buzzerState = LOW;
    if (feedbackMode == FEEDBACK_MODE_BOTH || feedbackMode == FEEDBACK_MODE_BUZZER) {
      digitalWrite(BUZZER_PIN, buzzerState);
    }
    // Turn off vibration motors when level changes
    if (feedbackMode == FEEDBACK_MODE_BOTH || feedbackMode == FEEDBACK_MODE_VIBRATION) {
      digitalWrite(VIB1_PIN, LOW);
      digitalWrite(VIB2_PIN, LOW);
    }
    lastChangeTime = currentMillis;
    currentLevel = newLevel;
    lastBuzzerLevel = newLevel;
  }
  
  // Special case: Level 0 (above 180cm) = completely ON (reversed logic)
  if (currentLevel == 0) {
    if (feedbackMode == FEEDBACK_MODE_BOTH || feedbackMode == FEEDBACK_MODE_BUZZER) {
      digitalWrite(BUZZER_PIN, HIGH);
    }
    // Vibration motors OFF when buzzer is ON (OPPOSITE phase)
    if (feedbackMode == FEEDBACK_MODE_BOTH || feedbackMode == FEEDBACK_MODE_VIBRATION) {
      digitalWrite(VIB1_PIN, LOW);
      digitalWrite(VIB2_PIN, LOW);
    }
    return;
  }
  
  // SIMPLIFIED: Always enable vibration when buzzer is active (for testing)
  if (feedbackMode == FEEDBACK_MODE_BOTH || feedbackMode == FEEDBACK_MODE_VIBRATION) {
    // Simple vibration pattern: ON when buzzer is OFF
    if (buzzerState == HIGH) {
      digitalWrite(VIB1_PIN, LOW);
      digitalWrite(VIB2_PIN, LOW);
    } else {
      digitalWrite(VIB1_PIN, HIGH);
      digitalWrite(VIB2_PIN, HIGH);
    }
  }
  
  // Normal buzzer timing for other levels
  if (buzzerState == HIGH) {
    if (currentMillis - lastChangeTime >= onTimes[currentLevel]) {
      buzzerState = LOW;
      lastChangeTime = currentMillis;
      if (feedbackMode == FEEDBACK_MODE_BOTH || feedbackMode == FEEDBACK_MODE_BUZZER) {
        digitalWrite(BUZZER_PIN, buzzerState);
      }
    }
  } else {
    if (currentMillis - lastChangeTime >= offTimes[currentLevel]) {
      buzzerState = HIGH;
      lastChangeTime = currentMillis;
      if (feedbackMode == FEEDBACK_MODE_BOTH || feedbackMode == FEEDBACK_MODE_BUZZER) {
        digitalWrite(BUZZER_PIN, buzzerState);
      }
    }
  }
}

// ============= Filtering & Processing =============
static void updateBuffer(uint16_t value) {
  uint16_t maxDist = (currentMode == SIMPLE_MODE) ? MAX_LONG_DISTANCE_MM : MAX_LONG_DISTANCE_MM;
  if (value > maxDist) value = maxDist;
  samples[sampleIndex] = value;
  sampleIndex = (sampleIndex + 1) % NUM_SAMPLES;
  if (validSamples < NUM_SAMPLES) validSamples++;
}

static uint16_t getMedianDistance() {
  uint16_t temp[NUM_SAMPLES];
  uint8_t count = validSamples < NUM_SAMPLES ? validSamples : NUM_SAMPLES;
  memcpy(temp, samples, count * sizeof(uint16_t));
  for (uint8_t i = 1; i < count; i++) {
    uint16_t key = temp[i];
    int8_t j = i - 1;
    while (j >= 0 && temp[j] > key) {
      temp[j + 1] = temp[j];
      j--;
    }
    temp[j + 1] = key;
  }
  return temp[count >> 1];
}

static void applyStableEMA(float newDistance) {
  static uint8_t initCount = 0;
  static float lastStableDistance = 3500.0;
  static uint32_t lastLargeChangeTime = 0;
  static bool recoveringFromObstacle = false;
  static uint8_t consecutiveStableReadings = 0;
  
  if (initCount < 8) {  // Balanced initialization
    if (newDistance >= MIN_DISTANCE_MM && newDistance <= MAX_LONG_DISTANCE_MM) {
      filteredDistance = (filteredDistance * initCount + newDistance) / (initCount + 1);
      lastStableDistance = filteredDistance;
      initCount++;
    }
    return;
  }
  
  // Check for sudden large changes (outliers)
  float change = fabs(newDistance - lastStableDistance);
  uint32_t currentTime = millis();
  
      // SAFETY FIRST: Immediate response to new obstacles (critical for blind users)
    if (newDistance < lastStableDistance - 25.0f && newDistance < 1500.0f) {
      // New obstacle detected - IMMEDIATE response for safety
      recoveringFromObstacle = false;
      consecutiveStableReadings = 0;
      // adaptiveSpeed = 1; // TEMPORARILY DISABLED: Conservative mode for safety
      filteredDistance = newDistance; // Immediate response
      lastStableDistance = filteredDistance;
      return;
    }
  
  // Smart adaptive recovery from obstacle removal
  if (newDistance > lastStableDistance + 35.0f && lastStableDistance < 800.0f) {
    // Obstacle was removed - use adaptive recovery
    recoveringFromObstacle = true;
    lastLargeChangeTime = currentTime;
    consecutiveStableReadings = 0;
    
    // Adaptive recovery based on environment stability
    float recoveryRate = 0.95f; // Almost instant jump
    if (adaptiveSpeed == 3) recoveryRate = 1.0f;   // Full jump in one cycle
    else if (adaptiveSpeed == 1) recoveryRate = 0.9f; // Still fast
    
    filteredDistance = lastStableDistance + (newDistance - lastStableDistance) * recoveryRate;
  }
  // Normal filtering with adaptive speed
  else if (change <= 70.0f) {
    float alpha = ALPHA_MID;
    if (change > CHANGE_THRESHOLD_LARGE) alpha = ALPHA_MAX;
    else if (change < CHANGE_THRESHOLD_SMALL) alpha = ALPHA_MIN;
    
    // Adaptive alpha based on environment stability
    if (recoveringFromObstacle && newDistance > filteredDistance) {
      if (adaptiveSpeed == 3) alpha = ALPHA_MAX * 1.2f; // Fast recovery
      else if (adaptiveSpeed == 1) alpha = ALPHA_MAX * 0.7f; // Conservative
      else alpha = ALPHA_MAX * 0.9f; // Balanced
    }
    
    filteredDistance = alpha * newDistance + (1 - alpha) * filteredDistance;
    
    // Track stable readings and adjust adaptive speed
    if (change < 15.0f) {
      consecutiveStableReadings++;
      // TEMPORARILY DISABLED: Mode switching
      // if (consecutiveStableReadings > 8 && adaptiveSpeed < 3) {
      //   adaptiveSpeed++; // Increase speed in stable environment
      // }
    } else {
      consecutiveStableReadings = 0;
      // TEMPORARILY DISABLED: Mode switching
      // if (change > 50.0f && adaptiveSpeed > 1) {
      //   adaptiveSpeed--; // Decrease speed in unstable environment
      // }
    }
  }
  // Large change - adaptive conservative approach
  else {
    float conservativeRate = 0.12f; // Balanced default
    if (adaptiveSpeed == 1) conservativeRate = 0.08f; // More conservative
    else if (adaptiveSpeed == 3) conservativeRate = 0.18f; // Less conservative
    
    filteredDistance = lastStableDistance + (newDistance - lastStableDistance) * conservativeRate;
    consecutiveStableReadings = 0;
  }
  
  // Smart recovery reset based on environment
  uint32_t resetTimeout = (adaptiveSpeed == 1) ? 3500 : (adaptiveSpeed == 3) ? 2000 : 2500;
  uint8_t requiredStableReadings = (adaptiveSpeed == 1) ? 6 : (adaptiveSpeed == 3) ? 3 : 4;
  
  if (recoveringFromObstacle && (currentTime - lastLargeChangeTime > resetTimeout || consecutiveStableReadings > requiredStableReadings)) {
    recoveringFromObstacle = false;
  }
  
  // Ensure bounds
  if (filteredDistance > MAX_LONG_DISTANCE_MM) filteredDistance = MAX_LONG_DISTANCE_MM;
  if (filteredDistance < MIN_DISTANCE_MM) filteredDistance = MIN_DISTANCE_MM;
  
  // Update stable distance reference
  lastStableDistance = filteredDistance;
}

// ============= Alert Handling =============
static void checkDistanceAlerts(uint16_t distance, uint32_t currentTime) {
  if (alertActive && (currentTime - lastAlert > 1000)) alertActive = false;
  if (distance < CRITICAL_DISTANCE_MM && !alertActive) {
    lastAlert = currentTime;
    alertActive = true;
    digitalWrite(VIB1_PIN, HIGH);
    digitalWrite(VIB2_PIN, HIGH);
    digitalWrite(BUZZER_PIN, HIGH);
    alertStartTime = millis();
    vibActive = true;
  } else if (distance < WARNING_DISTANCE_MM && !alertActive) {
    lastAlert = currentTime;
    alertActive = true;
  }
}

// ============= Mode Management =============
static void handleModeSwitching(uint32_t currentTime) {
  // TEMPORARILY DISABLED: Automatic mode switching
  // if (currentTime - lastModeSwitch < MODE_SWITCH_DELAY) return;
  // static uint16_t lastDist = MAX_LONG_DISTANCE_MM;
  // if (currentMode != STAIR_DETECTION_MODE &&
  //     abs(filteredDistance - lastDist) > 200 &&
  //     filteredDistance < 800) {
  //   Serial.println(F("⛰️ Possible stair detected - activating stair mode"));
  //   configureSensor(STAIR_DETECTION_MODE);
  // }
  // lastDist = filteredDistance;
}

// ============= System Status =============
static void printStatus() {
  float distance_cm = filteredDistance / 10.0f;
  Serial.print(F("📏 "));
  if (distance_cm >= (MAX_LONG_DISTANCE_MM/10.0f) - 5) {
    Serial.print(F("CLEAR"));
  } else if (distance_cm < (MIN_DISTANCE_MM/10.0f)) {
    Serial.print(F("DANGER "));
    Serial.print(distance_cm, 0);
    Serial.print(F("cm"));
  } else {
    Serial.print(distance_cm, 0);
    Serial.print(F("cm"));
    if (distance_cm < (WARNING_DISTANCE_MM/10.0f)) {
      Serial.print(F(" ⚠️"));
    }
  }
  Serial.print(F(" | Mode: "));
  switch (currentMode) {
    case SIMPLE_MODE: Serial.print(F("SIMPLE")); break;
    case RADAR_MODE: Serial.print(F("RADAR")); break;
  }
  if (alertActive) Serial.print(F(" | ⚠️ ALERT"));
  Serial.println();
}

static void printCurrentConfig() {
  Serial.println(F("\n⚙️ Current Configuration:"));
  Serial.print(F("  Max Range: "));
  Serial.print((currentMode == SIMPLE_MODE) ? 350 : 350);
  Serial.println(F("0cm"));
  Serial.print(F("  Current Mode: "));
  switch (currentMode) {
    case SIMPLE_MODE: Serial.println(F("SIMPLE (Fixed ToF)")); break;
    case RADAR_MODE: Serial.println(F("RADAR (Servo Scanning)")); break;
  }
  Serial.println(F("========================================"));
}

// ============= Public Interface Functions =============

void ToF_switchToRadarMode() {
  if (currentMode != RADAR_MODE) {
    Serial.println(F("🔄 Switching to RADAR MODE"));
    currentMode = RADAR_MODE;
    radarModeActive = true;
    
    // Stop any existing radar task
    if (radarTaskHandle != nullptr) {
      vTaskDelete(radarTaskHandle);
      radarTaskHandle = nullptr;
    }
    
    // Start real-time radar scanning
    runRadarScan();
  }
}

void ToF_switchToSimpleMode() {
  if (currentMode != SIMPLE_MODE) {
    Serial.println(F("🔄 Switching to SIMPLE MODE"));
    
    // Stop radar scanning
    radarModeActive = false;
    if (radarTaskHandle != nullptr) {
      vTaskDelete(radarTaskHandle);
      radarTaskHandle = nullptr;
    }
    
    currentMode = SIMPLE_MODE;
    
    // Return servo to center and restart continuous mode
    moveServoToAngle(90);
    sensor.stopContinuous();
    delay(100);
    sensor.startContinuous(20);
  }
}

bool ToF_isRadarMode() {
  return currentMode == RADAR_MODE;
}

// ============= Radar Mode Functions =============

// Real-time radar scanning task (FreeRTOS)
static void radarScanTask(void* parameter) {
  Serial.println(F("🔄 Real-time radar task started"));
  
  while (radarModeActive) {
    uint32_t currentTime = millis();
    
    // Check if it's time to move to next angle
    if (currentTime - lastAngleTime >= ANGLE_INTERVAL) {
      // Move servo to current angle
      moveServoToAngle(currentScanAngle);
      
      // Take distance reading (non-blocking)
      uint16_t distance = getQuickDistanceReading();
      scanData[currentScanAngle] = distance;
      
      // Immediately transmit angle and distance data via BLE
      BLEManager::sendRadarLiveData(currentScanAngle, distance);
      
      // Move to next angle
      currentScanAngle++;
      if (currentScanAngle >= NUM_RADAR_ANGLES) {
        currentScanAngle = 0; // Reset to start new sweep
        Serial.println(F("🔄 Radar sweep completed, starting new cycle"));
      }
      
      lastAngleTime = currentTime;
    }
    
    // Small delay to prevent task from consuming too much CPU
    vTaskDelay(pdMS_TO_TICKS(1));
  }
  
  // Clean up when radar mode is disabled
  moveServoToAngle(90); // Return to center
  radarTaskHandle = nullptr;
  vTaskDelete(nullptr);
}

// Quick distance reading optimized for real-time scanning
static uint16_t getQuickDistanceReading() {
  if (!sensor.dataReady()) {
    return 3500; // Return max range if no data ready
  }
  
  uint16_t distance = sensor.read();
  
  // Validate reading
  if (distance < MIN_DISTANCE_MM || distance > MAX_LONG_DISTANCE_MM) {
    return 3500; // Return max range for invalid readings
  }
  
  return distance;
}

// Legacy function - now starts real-time scanning
static void runRadarScan() {
  if (!radarModeActive) return;
  
  // Start real-time scanning if not already running
  if (radarTaskHandle == nullptr) {
    currentScanAngle = 0;
    lastAngleTime = millis();
    
    // Create FreeRTOS task for real-time radar scanning
    xTaskCreatePinnedToCore(
      radarScanTask,
      "RadarScan",
      4096,
      nullptr,
      2, // High priority
      &radarTaskHandle,
      1  // Pin to core 1
    );
    
    Serial.println(F("🔄 Real-time radar scanning started"));
  }
}

static void moveServoToAngle(int angle) {
  // Direct mapping: 0° to 180° maps to servo 0° to 180°
  int servoAngle = angle;
  scanServo.write(servoAngle);
  // No delay for real-time performance - servo moves while we process next angle
}

static uint16_t getMedianReading(uint16_t readings[], int count) {
  // Simple bubble sort for median
  for (int i = 0; i < count - 1; i++) {
    for (int j = 0; j < count - i - 1; j++) {
      if (readings[j] > readings[j + 1]) {
        uint16_t temp = readings[j];
        readings[j] = readings[j + 1];
        readings[j + 1] = temp;
      }
    }
  }
  return readings[count / 2]; // Return median
}

static void analyzeRadarData() {
  // Find safest direction
  const char* safestDir = getSafestDirection();
  
  // Update filtered distance for main system
  filteredDistance = scanData[90]; // Use center reading (90°)
  
  // Check for immediate obstacles
  bool immediateObstacle = false;
  for (int i = 0; i < NUM_RADAR_ANGLES; i++) {
    if (scanData[i] < 1000) { // Obstacle within 1m (1000mm)
      immediateObstacle = true;
      break;
    }
  }
  
  // Trigger alerts if needed
  if (immediateObstacle) {
    checkDistanceAlerts(filteredDistance, millis());
  }
}

static const char* getSafestDirection() {
  // Find direction with maximum distance
  uint16_t maxDistance = 0;
  int bestAngle = 0;
  
  for (int i = 0; i < NUM_RADAR_ANGLES; i++) {
    if (scanData[i] > maxDistance) {
      maxDistance = scanData[i];
      bestAngle = i; // Angle is the index (0° to 180°)
    }
  }
  
  // Determine safest direction based on angle ranges
  if (bestAngle < 60) return "RIGHT";        // 0° to 59°
  else if (bestAngle > 120) return "LEFT";   // 121° to 180°
  else return "CENTER";                      // 60° to 120°
}

static void printRadarResults() {
  Serial.println(F("\n📊 RADAR SCAN RESULTS (Key Angles):"));
  Serial.println(F("┌─────────┬────────────┬──────────┐"));
  Serial.println(F("│ Angle   │ Distance   │ Obstacle? │"));
  Serial.println(F("├─────────┼────────────┼──────────┤"));
  
  // Show key angles: every 30 degrees
  int keyAngles[] = {0, 30, 60, 90, 120, 150, 180};
  int numKeyAngles = 7;
  
  for (int i = 0; i < numKeyAngles; i++) {
    int angle = keyAngles[i];
    uint16_t distance = scanData[angle];
    bool hasObstacle = distance < 1500; // Obstacle within 1.5m (1500mm)
    
    Serial.printf("│ %6d° │ %8d mm │ %8s │\n", 
                  angle, distance, hasObstacle ? "⚠️ Yes" : "✅ No");
  }
  
  Serial.println(F("└─────────┴────────────┴──────────┘"));
  
  const char* safestDir = getSafestDirection();
  Serial.printf("🎯 SAFEST DIRECTION: %s\n", safestDir);
  Serial.printf("📏 CENTER DISTANCE (90°): %d mm\n", scanData[90]);
}

// ============= Public Radar Mode Functions (duplicates removed) =============

void ToF_manualReset() {
  resetSensor();
}

void ToF_diagnostics() {
  Serial.println("\n🔧 ToF Sensor Diagnostics:");
  Serial.println("================================");
  
  // Check I2C communication
  Wire.beginTransmission(0x29); // VL53L1X default address
  uint8_t error = Wire.endTransmission();
  Serial.printf("I2C Communication: %s\n", error == 0 ? "✅ OK" : "❌ FAILED");
  
  if (error != 0) {
    SensorHealthManager::updateSensorHealth("vl53l1x", SENSOR_DISCONNECTED, nullptr, "I2C communication failed");
  }
  
  // Check sensor initialization status
  Serial.printf("Current Mode: %s\n", currentMode == RADAR_MODE ? "RADAR" : "SIMPLE");
  Serial.printf("Radar Mode Active: %s\n", radarModeActive ? "Yes" : "No");
  
  // Test sensor data ready status
  Serial.print("Sensor Data Ready: ");
  if (sensor.dataReady()) {
    uint16_t reading = sensor.read();
    Serial.printf("✅ YES (Reading: %d cm)\n", reading);
  } else {
    Serial.println("❌ NO");
  }
  
  // Check servo status
  Serial.printf("Servo Pin: %d\n", SERVO_PIN);
  Serial.println("Servo Status: Attached");
  
  // Test a single reading with timeout
  Serial.println("\nTesting single reading with 2s timeout...");
  uint32_t timeout = millis() + 2000;
  bool gotReading = false;
  
  while (millis() < timeout && !gotReading) {
    if (sensor.dataReady()) {
      uint16_t testReading = sensor.read();
      Serial.printf("✅ Test reading successful: %d cm\n", testReading);
      gotReading = true;
    }
    delay(50);
  }
  
  if (!gotReading) {
    Serial.println("❌ Test reading failed - sensor timeout");
    Serial.println("💡 Suggestions:");
    Serial.println("   - Check I2C wiring (SDA=8, SCL=9)");
    Serial.println("   - Check sensor power supply");
    Serial.println("   - Try 'tofreset' command");
  }
  
  Serial.println("================================\n");
}

// Getter functions for BLE access
OperationMode ToF_getCurrentMode() {
  return currentMode;
}

uint16_t* ToF_getScanData() {
  return scanData;
}