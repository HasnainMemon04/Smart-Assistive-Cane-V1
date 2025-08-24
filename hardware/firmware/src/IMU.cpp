#include "IMU.h"
#include "Pins.h"
#include "SensorHealth.h"
#include <Wire.h>
#include <MadgwickAHRS.h>
#include "SDCardManager.h"
#include <math.h>
#include <time.h>

#include "ConnectivityManager.h"

#define MPU_ADDR 0x68
constexpr uint32_t SAMPLE_US = 1000000 / 100;  // 100 Hz

// EEPROM addresses for step data
const int STEP_DATA_ADDR = 64;  // After calibration data
const uint32_t STEP_DATA_MAGIC = 0x53544550; // "STEP" in ASCII

struct StepData {
  uint32_t magic;
  uint32_t dailySteps;
  uint32_t totalSteps;
  uint8_t lastResetDay;
  uint8_t lastResetMonth;
  uint16_t lastResetYear;
  uint32_t checksum;
};

struct CalibrationData {
  uint16_t magic;
  uint8_t version;
  int16_t accelOffsets[3];
  int16_t gyroOffsets[3];
};
const int EEPROM_ADDR = 0;
const uint16_t EEPROM_MAGIC = 0xCAFE;
const uint8_t EEPROM_VERSION = 1;

static uint8_t i2cErrorCount = 0;
static bool i2cErrorFlag = false;

enum FeedbackState {
  FB_IDLE, FB_CALIBRATING, FB_CALIB_COMPLETE, FB_FALL_ALERT, FB_I2C_ERROR
};
static FeedbackState fbState = FB_IDLE;
static uint32_t fbTimer = 0;

static Madgwick filter;
static uint32_t nextMicros = 0;

static int16_t axBuf[5], ayBuf[5], azBuf[5];
static int16_t gxBuf[5], gyBuf[5], gzBuf[5];
static uint8_t bufIdx = 0;

static int16_t gyroOffsets[3] = {0};
static int16_t accelOffsets[3] = {0};

static float smoothRoll = 0, smoothPitch = 0, smoothYaw = 0;
static float smoothAx = 0, smoothAy = 0, smoothAz = 0;
static float smoothGx = 0, smoothGy = 0, smoothGz = 0;
static const float SMOOTH_ALPHA = 0.15;
static const float FAST_ALPHA = 0.4;

static float motionEnergy = 0;
static const float MOTION_THRESH = 0.5;

static const float STEP_THRESHOLD_HIGH = 1.3f;
static const float STEP_THRESHOLD_LOW = 0.9f;
static const uint32_t STEP_MIN_PERIOD = 400000;
static const uint32_t STEP_MAX_PERIOD = 1200000;
static uint32_t stepStartTime = 0;
static uint32_t stepCount = 0;
static uint32_t dailySteps = 0;
static uint32_t totalSteps = 0;
static uint8_t lastResetDay = 0;
static uint8_t lastResetMonth = 0;
static uint16_t lastResetYear = 0;
enum StepState { STEP_IDLE, STEP_RISING, STEP_PEAK, STEP_FALLING };
static StepState stepState = STEP_IDLE;
static float stepPeakValue = 0;

static const float FALL_IMPACT_THRESHOLD = 3.0f;
static const float FALL_FREE_THRESHOLD = 0.4f;
static const uint32_t FALL_INACTIVITY_TIME = 3000;
static uint32_t fallStartTime = 0;
static uint32_t impactTime = 0;
static uint32_t lastMovementTime = 0;
static FallState fallState = FALL_NONE;

static const float SLOPE_THRESHOLD_HIGH = 16.0f;
static const float SLOPE_THRESHOLD_LOW = 12.0f;
static bool slopeWarningActive = false;

inline void swap16(int16_t &a, int16_t &b) { int16_t t = a; a = b; b = t; }
static int16_t median5(int16_t samples[5]) {
  for (int i = 0; i < 4; i++) {
    for (int j = i+1; j < 5; j++) {
      if (samples[j] < samples[i]) swap16(samples[i], samples[j]);
    }
  }
  return samples[2];
}

// EEPROM
static void saveCalibrationToEEPROM() {
  CalibrationData data;
  data.magic = EEPROM_MAGIC;
  data.version = EEPROM_VERSION;
  memcpy(data.accelOffsets, accelOffsets, sizeof(accelOffsets));
  memcpy(data.gyroOffsets, gyroOffsets, sizeof(gyroOffsets));
  SDCard_writeFile("/data/imu_calib.dat", (uint8_t*)&data, sizeof(CalibrationData));
}
static bool loadCalibrationFromEEPROM() {
  CalibrationData data;
  size_t bytesRead;
  if (!SDCard_readFile("/data/imu_calib.dat", (uint8_t*)&data, sizeof(CalibrationData), &bytesRead) || bytesRead != sizeof(CalibrationData)) {
    return false;
  }
  if (data.magic != EEPROM_MAGIC || data.version != EEPROM_VERSION) return false;
  memcpy(accelOffsets, data.accelOffsets, sizeof(accelOffsets));
  memcpy(gyroOffsets, data.gyroOffsets, sizeof(gyroOffsets));
  return true;
}
static void factoryResetEEPROM() {
  // Delete calibration file
  SDCard_deleteFile("/data/imu_calib.dat");
  
  // Delete step data file
  SDCard_deleteFile("/data/step_data.dat");
  
  Serial.println("Factory Reset - Calibration and step data cleared");
}

// Step data management functions
static uint32_t calculateChecksum(const StepData& data) {
  uint32_t sum = 0;
  const uint8_t* bytes = (const uint8_t*)&data;
  for (size_t i = 0; i < sizeof(StepData) - sizeof(uint32_t); i++) {
    sum += bytes[i];
  }
  return sum;
}

static void saveStepDataToEEPROM() {
  StepData data;
  data.magic = STEP_DATA_MAGIC;
  data.dailySteps = dailySteps;
  data.totalSteps = totalSteps;
  data.lastResetDay = lastResetDay;
  data.lastResetMonth = lastResetMonth;
  data.lastResetYear = lastResetYear;
  data.checksum = calculateChecksum(data);
  
  SDCard_writeFile("/data/step_data.dat", (uint8_t*)&data, sizeof(StepData));
#ifdef SC_DEBUG_IMU
  Serial.printf("Saved step data: Daily=%lu, Total=%lu, Date=%02d/%02d/%04d\n", 
                dailySteps, totalSteps, lastResetDay, lastResetMonth, lastResetYear);
#endif
}

static bool loadStepDataFromEEPROM() {
  StepData data;
  size_t bytesRead;
  if (!SDCard_readFile("/data/step_data.dat", (uint8_t*)&data, sizeof(StepData), &bytesRead) || bytesRead != sizeof(StepData)) {
#ifdef SC_DEBUG_IMU
    Serial.println("No valid step data found on SD card");
#endif
    return false;
  }
  
  if (data.magic != STEP_DATA_MAGIC) {
#ifdef SC_DEBUG_IMU
    Serial.println("No valid step data found on SD card");
#endif
    return false;
  }
  
  uint32_t expectedChecksum = calculateChecksum(data);
  if (data.checksum != expectedChecksum) {
#ifdef SC_DEBUG_IMU
    Serial.println("Step data checksum mismatch - using defaults");
#endif
    return false;
  }
  
  dailySteps = data.dailySteps;
  totalSteps = data.totalSteps;
  lastResetDay = data.lastResetDay;
  lastResetMonth = data.lastResetMonth;
  lastResetYear = data.lastResetYear;
  
#ifdef SC_DEBUG_IMU
  Serial.printf("Loaded step data: Daily=%lu, Total=%lu, Date=%02d/%02d/%04d\n", 
                dailySteps, totalSteps, lastResetDay, lastResetMonth, lastResetYear);
#endif
  return true;
}

static void resetDailySteps() {
  dailySteps = 0;
  lastResetDay = 0;
  lastResetMonth = 0;
  lastResetYear = 0;
  saveStepDataToEEPROM();
#ifdef SC_DEBUG_IMU
  Serial.println("Daily steps reset");
#endif
}

static bool shouldResetDailySteps() {
  // Priority 1: Use internet time (NTP) if available
  if (ConnectivityManager::isTimeSynced()) {
    struct tm timeinfo;
    if (getLocalTime(&timeinfo)) {
      int currentHour = timeinfo.tm_hour;
      int currentMinute = timeinfo.tm_min;
      int currentDay = timeinfo.tm_mday;
      int currentMonth = timeinfo.tm_mon + 1;
      int currentYear = timeinfo.tm_year + 1900;
      
      // Reset at 23:59 PKT (18:59 UTC)
      if (currentHour == 18 && currentMinute == 59) {
        // Check if we haven't already reset today
        if (lastResetDay != currentDay || lastResetMonth != currentMonth || lastResetYear != currentYear) {
          lastResetDay = currentDay;
          lastResetMonth = currentMonth;
          lastResetYear = currentYear;
#ifdef SC_DEBUG_IMU
          Serial.printf("Daily reset triggered by NTP time: %02d:%02d %02d/%02d/%04d\n", 
                        currentHour, currentMinute, currentDay, currentMonth, currentYear);
#endif
          return true;
        }
      }
      return false;
    }
  }
  
  // Priority 2: Fall back to GPS time if available
  time_t now = time(nullptr);
  if (now >= 24 * 60 * 60) { // If time is set (likely by GPS)
    struct tm* timeinfo = localtime(&now);
    if (timeinfo) {
      int currentHour = timeinfo->tm_hour;
      int currentMinute = timeinfo->tm_min;
      int currentDay = timeinfo->tm_mday;
      int currentMonth = timeinfo->tm_mon + 1;
      int currentYear = timeinfo->tm_year + 1900;
      
      // Reset at 23:59 PKT (18:59 UTC)
      if (currentHour == 18 && currentMinute == 59) {
        // Check if we haven't already reset today
        if (lastResetDay != currentDay || lastResetMonth != currentMonth || lastResetYear != currentYear) {
          lastResetDay = currentDay;
          lastResetMonth = currentMonth;
          lastResetYear = currentYear;
#ifdef SC_DEBUG_IMU
          Serial.printf("Daily reset triggered by GPS time: %02d:%02d %02d/%02d/%04d\n", 
                        currentHour, currentMinute, currentDay, currentMonth, currentYear);
#endif
          return true;
        }
      }
    }
  }
  
  return false;
}

// Function to set time for testing (call this from GPS module when time is available)
void IMU_setTime(uint8_t hour, uint8_t minute, uint8_t day, uint8_t month, uint16_t year) {
  struct tm timeinfo;
  timeinfo.tm_hour = hour;
  timeinfo.tm_min = minute;
  timeinfo.tm_sec = 0;
  timeinfo.tm_mday = day;
  timeinfo.tm_mon = month - 1; // tm_mon is 0-based
  timeinfo.tm_year = year - 1900; // tm_year is years since 1900
  timeinfo.tm_isdst = 0;
  
  time_t timestamp = mktime(&timeinfo);
  if (timestamp != -1) {
    struct timeval tv;
    tv.tv_sec = timestamp;
    tv.tv_usec = 0;
    settimeofday(&tv, nullptr);
#ifdef SC_DEBUG_IMU
    Serial.printf("GPS time set to: %02d:%02d %02d/%02d/%04d\n", hour, minute, day, month, year);
#endif
  }
}

// Function to get current time source for debugging
String IMU_getTimeSource() {
  if (ConnectivityManager::isTimeSynced()) {
    return "NTP (Internet)";
  } else {
    time_t now = time(nullptr);
    if (now >= 24 * 60 * 60) {
      return "GPS";
    } else {
      return "None";
    }
  }
}

// MPU
static void configureMPU() {
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x6B); Wire.write(0x80); Wire.endTransmission(); delay(100);
  
  uint8_t error = 0;
  Wire.beginTransmission(MPU_ADDR); Wire.write(0x6B); Wire.write(0x01); error += Wire.endTransmission();
  Wire.beginTransmission(MPU_ADDR); Wire.write(0x1A); Wire.write(0x03); error += Wire.endTransmission();
  Wire.beginTransmission(MPU_ADDR); Wire.write(0x19); Wire.write(0x04); error += Wire.endTransmission();
  Wire.beginTransmission(MPU_ADDR); Wire.write(0x1C); Wire.write(0x10); error += Wire.endTransmission();
  Wire.beginTransmission(MPU_ADDR); Wire.write(0x1B); Wire.write(0x10); error += Wire.endTransmission();
  
  if (error == 0) {
    SensorHealthManager::updateSensorHealth("mpu6050", SENSOR_OK, "MPU6050 configured");
    i2cErrorCount = 0;
  } else {
    SensorHealthManager::updateSensorHealth("mpu6050", SENSOR_ERROR, nullptr, "MPU6050 config failed");
  }
}
static bool readMPUData(int16_t& ax, int16_t& ay, int16_t& az, int16_t& gx, int16_t& gy, int16_t& gz) {
  Wire.beginTransmission(MPU_ADDR); Wire.write(0x3B);
  if (Wire.endTransmission(false) != 0) {
    SensorHealthManager::updateSensorHealth("mpu6050", SENSOR_ERROR, nullptr, "I2C transmission failed");
    return false;
  }
  uint8_t bytesRead = Wire.requestFrom(MPU_ADDR, 14);
  if (bytesRead != 14) {
    SensorHealthManager::updateSensorHealth("mpu6050", SENSOR_ERROR, nullptr, "Incomplete data read");
    return false;
  }
  ax = Wire.read()<<8 | Wire.read();
  ay = Wire.read()<<8 | Wire.read();
  az = Wire.read()<<8 | Wire.read();
  Wire.read(); Wire.read();
  gx = Wire.read()<<8 | Wire.read();
  gy = Wire.read()<<8 | Wire.read();
  gz = Wire.read()<<8 | Wire.read();
  
  // Report successful read with acceleration magnitude as value
  float accelMag = sqrt((ax/4096.0f)*(ax/4096.0f) + (ay/4096.0f)*(ay/4096.0f) + (az/4096.0f)*(az/4096.0f));
  SensorHealthManager::updateSensorHealth("mpu6050", SENSOR_OK, String(accelMag, 3).c_str());
  return true;
}
static void handleI2CError() {
  i2cErrorCount++;
  if (i2cErrorCount == 1) i2cErrorFlag = true;
  if (i2cErrorCount >= 5) { configureMPU(); i2cErrorCount = 0; nextMicros = micros() + SAMPLE_US; }
}

// Feedback
static void updateFeedback() {
  uint32_t currentMillis = millis();
  
  // Get current feedback mode from SensorData
  extern SensorData sensorData;
  uint8_t feedbackMode = sensorData.feedbackMode;
  
  if (i2cErrorFlag) { fbState = FB_I2C_ERROR; fbTimer = currentMillis; i2cErrorFlag = false; }
  switch (fbState) {
    case FB_IDLE: 
      if (feedbackMode == FEEDBACK_MODE_BOTH || feedbackMode == FEEDBACK_MODE_BUZZER) {
        digitalWrite(FEEDBACK_PIN, LOW);
      }
      if (feedbackMode == FEEDBACK_MODE_BOTH || feedbackMode == FEEDBACK_MODE_VIBRATION) {
        digitalWrite(VIB1_PIN, LOW);
        digitalWrite(VIB2_PIN, LOW);
      }
      break;
    case FB_CALIBRATING:
      if (currentMillis - fbTimer >= 500) { 
        bool feedbackState = !digitalRead(FEEDBACK_PIN);
        if (feedbackMode == FEEDBACK_MODE_BOTH || feedbackMode == FEEDBACK_MODE_BUZZER) {
          digitalWrite(FEEDBACK_PIN, feedbackState); 
        }
        // Vibration motors SAME phase as feedback pin
        if (feedbackMode == FEEDBACK_MODE_BOTH || feedbackMode == FEEDBACK_MODE_VIBRATION) {
          digitalWrite(VIB1_PIN, feedbackState);
          digitalWrite(VIB2_PIN, feedbackState);
        }
        fbTimer = currentMillis; 
      }
      break;
    case FB_CALIB_COMPLETE:
      if (currentMillis - fbTimer > 1000) { fbState = FB_IDLE; }
      else { 
        uint8_t pulse = (currentMillis - fbTimer) / 100; 
        bool feedbackState = (pulse == 0 || pulse == 2);
        if (feedbackMode == FEEDBACK_MODE_BOTH || feedbackMode == FEEDBACK_MODE_BUZZER) {
          digitalWrite(FEEDBACK_PIN, feedbackState); 
        }
        // Vibration motors SAME phase as feedback pin
        if (feedbackMode == FEEDBACK_MODE_BOTH || feedbackMode == FEEDBACK_MODE_VIBRATION) {
          digitalWrite(VIB1_PIN, feedbackState);
          digitalWrite(VIB2_PIN, feedbackState);
        }
      }
      break;
    case FB_FALL_ALERT:
      if (currentMillis - fbTimer >= 100) { 
        bool feedbackState = !digitalRead(FEEDBACK_PIN);
        if (feedbackMode == FEEDBACK_MODE_BOTH || feedbackMode == FEEDBACK_MODE_BUZZER) {
          digitalWrite(FEEDBACK_PIN, feedbackState); 
        }
        // Vibration motors SAME phase as feedback pin
        if (feedbackMode == FEEDBACK_MODE_BOTH || feedbackMode == FEEDBACK_MODE_VIBRATION) {
          digitalWrite(VIB1_PIN, feedbackState);
          digitalWrite(VIB2_PIN, feedbackState);
        }
        fbTimer = currentMillis; 
      }
      if (currentMillis - fbTimer > 10000) { fbState = FB_IDLE; }
      break;
    case FB_I2C_ERROR:
      if (currentMillis - fbTimer < 50) { 
        if (feedbackMode == FEEDBACK_MODE_BOTH || feedbackMode == FEEDBACK_MODE_BUZZER) {
          digitalWrite(FEEDBACK_PIN, HIGH); 
        }
        // Vibration motors ON when feedback is ON (SAME phase)
        if (feedbackMode == FEEDBACK_MODE_BOTH || feedbackMode == FEEDBACK_MODE_VIBRATION) {
          digitalWrite(VIB1_PIN, HIGH);
          digitalWrite(VIB2_PIN, HIGH);
        }
      }
      else { 
        if (feedbackMode == FEEDBACK_MODE_BOTH || feedbackMode == FEEDBACK_MODE_BUZZER) {
          digitalWrite(FEEDBACK_PIN, LOW); 
        }
        // Vibration motors OFF when feedback is OFF (SAME phase)
        if (feedbackMode == FEEDBACK_MODE_BOTH || feedbackMode == FEEDBACK_MODE_VIBRATION) {
          digitalWrite(VIB1_PIN, LOW);
          digitalWrite(VIB2_PIN, LOW);
        }
        fbState = FB_IDLE; 
      }
      break;
  }
}

// Calibration
static void calibrateSensors() {
  const uint16_t samples = 1000;
  int32_t axSum = 0, aySum = 0, azSum = 0;
  int32_t gxSum = 0, gySum = 0, gzSum = 0;
  Serial.println("Calibrating... keep cane stationary and level");
  fbState = FB_CALIBRATING;
  fbTimer = millis();
  for (uint16_t i = 0; i < samples; i++) {
    if (i % 100 == 0) Serial.print('.');
    int16_t ax, ay, az, gx, gy, gz;
    if (!readMPUData(ax, ay, az, gx, gy, gz)) { handleI2CError(); i--; continue; }
    axSum += ax; aySum += ay; azSum += az;
    gxSum += gx; gySum += gy; gzSum += gz;
    delay(5);
  }
  gyroOffsets[0] = gxSum / samples;
  gyroOffsets[1] = gySum / samples;
  gyroOffsets[2] = gzSum / samples;
  const int16_t targetZ = 4096;
  accelOffsets[0] = axSum / samples;
  accelOffsets[1] = aySum / samples;
  accelOffsets[2] = (azSum / samples) - targetZ;
  Serial.println("\nCalibration complete");
  Serial.printf("Accel Offsets: X:%d Y:%d Z:%d\n", accelOffsets[0], accelOffsets[1], accelOffsets[2]);
  Serial.printf("Gyro Offsets: X:%d Y:%d Z:%d\n", gyroOffsets[0], gyroOffsets[1], gyroOffsets[2]);
  saveCalibrationToEEPROM();
  fbState = FB_CALIB_COMPLETE;
  fbTimer = millis();
}

// Step detection
static void detectSteps(uint32_t now) {
  float az = smoothAz;
  switch (stepState) {
    case STEP_IDLE:
      if (az > STEP_THRESHOLD_HIGH && motionEnergy > MOTION_THRESH) {
        stepState = STEP_RISING;
        stepStartTime = now;
        stepPeakValue = az;
      }
      break;
    case STEP_RISING:
      if (az > stepPeakValue) { stepPeakValue = az; }
      else if (az < stepPeakValue - 0.1f) { stepState = STEP_PEAK; }
      if (now - stepStartTime > STEP_MAX_PERIOD) { stepState = STEP_IDLE; }
      break;
    case STEP_PEAK:
      if (az < STEP_THRESHOLD_LOW) { stepState = STEP_FALLING; }
      break;
    case STEP_FALLING:
      if (az < STEP_THRESHOLD_LOW && now - stepStartTime > STEP_MIN_PERIOD) {
        stepCount++;
        dailySteps++;
        totalSteps++;
        
        // Save step data every 10 steps to avoid excessive EEPROM writes
        if (dailySteps % 10 == 0) {
          saveStepDataToEEPROM();
        }
        
        Serial.printf("Step detected! Daily: %lu, Total: %lu (Peak: %.2fg)\n", 
                      dailySteps, totalSteps, stepPeakValue);
        
        // Step detection feedback - short vibration (respect feedback mode)
        extern SensorData sensorData;
        uint8_t feedbackMode = sensorData.feedbackMode;
        if (feedbackMode == FEEDBACK_MODE_BOTH || feedbackMode == FEEDBACK_MODE_VIBRATION) {
          digitalWrite(VIB1_PIN, HIGH);
          digitalWrite(VIB2_PIN, HIGH);
          delay(50);
          digitalWrite(VIB1_PIN, LOW);
          digitalWrite(VIB2_PIN, LOW);
        }
        
        stepState = STEP_IDLE;
      }
      break;
  }
}
// Fall detection
static void detectFalls(uint32_t now) {
  static uint32_t lastActivityTime = 0;
  float accelMag = sqrt(smoothAx*smoothAx + smoothAy*smoothAy + smoothAz*smoothAz);
  if (accelMag > FALL_IMPACT_THRESHOLD && motionEnergy > MOTION_THRESH) { impactTime = millis(); }
  if (accelMag < FALL_FREE_THRESHOLD) {
    if (fallState == FALL_NONE) { fallState = FALL_DETECTED; fallStartTime = millis(); }
  } else if (fallState == FALL_DETECTED) { fallState = FALL_NONE; }
  if (fallState == FALL_DETECTED && impactTime > fallStartTime) {
    if (motionEnergy < MOTION_THRESH * 0.3f) {
      if (lastActivityTime == 0) { lastActivityTime = millis(); }
      else if (millis() - lastActivityTime > FALL_INACTIVITY_TIME) {
        fallState = FALL_CONFIRMED;
        Serial.println("\n!!! FALL CONFIRMED - SENDING ALERT !!!");
        fbState = FB_FALL_ALERT;
        fbTimer = millis();
      }
    } else { lastActivityTime = 0; }
  }
  if (fallState != FALL_NONE && motionEnergy > MOTION_THRESH) {
    if (millis() - impactTime > FALL_INACTIVITY_TIME * 2) {
      Serial.println("Fall alarm reset");
      fallState = FALL_NONE;
      lastActivityTime = 0;
    }
  }
}
// Slope warning
static void detectSlope() {
  static bool wasWarned = false;
  float absPitch = fabs(smoothPitch);
  float absRoll = fabs(smoothRoll);
  if (!slopeWarningActive && (absPitch > SLOPE_THRESHOLD_HIGH || absRoll > SLOPE_THRESHOLD_HIGH)) {
    slopeWarningActive = true;
    wasWarned = true;
    Serial.println("SLOPE WARNING: Steep terrain ahead!");
  } else if (slopeWarningActive && absPitch < SLOPE_THRESHOLD_LOW && absRoll < SLOPE_THRESHOLD_LOW) {
    slopeWarningActive = false;
    Serial.println("Slope warning cleared");
  }
  static uint32_t lastWarnTime = 0;
  if (slopeWarningActive && millis() - lastWarnTime > 5000) {
    Serial.println("SLOPE WARNING: Still on steep terrain");
    lastWarnTime = millis();
  }
}

void IMU_init() {
  Wire.begin(I2C_SDA, I2C_SCL, 400000);
  pinMode(CALIB_BUTTON_PIN, INPUT_PULLUP);
  pinMode(FEEDBACK_PIN, OUTPUT);
  digitalWrite(FEEDBACK_PIN, LOW);
  
  // Initialize vibration motors
  pinMode(VIB1_PIN, OUTPUT);
  pinMode(VIB2_PIN, OUTPUT);
  digitalWrite(VIB1_PIN, LOW);
  digitalWrite(VIB2_PIN, LOW);
  configureMPU();
  filter.begin(100);
  delay(500);
  int16_t ax, ay, az, gx, gy, gz;
  if (readMPUData(ax, ay, az, gx, gy, gz)) {
    for (uint8_t i = 0; i < 5; i++) {
      axBuf[i] = ax; ayBuf[i] = ay; azBuf[i] = az;
      gxBuf[i] = gx; gyBuf[i] = gy; gzBuf[i] = gz;
    }
    SensorHealthManager::updateSensorHealth("mpu6050", SENSOR_OK, "MPU6050 initialized");
  } else {
    SensorHealthManager::updateSensorHealth("mpu6050", SENSOR_INIT_FAILED, nullptr, "MPU6050 init failed");
  }
  if (loadCalibrationFromEEPROM()) {
    Serial.println("Loaded calibration from EEPROM");
    Serial.printf("Accel Offsets: X:%d Y:%d Z:%d\n", accelOffsets[0], accelOffsets[1], accelOffsets[2]);
    Serial.printf("Gyro Offsets: X:%d Y:%d Z:%d\n", gyroOffsets[0], gyroOffsets[1], gyroOffsets[2]);
  } else {
    Serial.println("No valid calibration data. Starting calibration...");
    calibrateSensors();
  }
  
  // Load step data from EEPROM
  if (loadStepDataFromEEPROM()) {
    Serial.println("Loaded step data from EEPROM");
  } else {
    Serial.println("No valid step data found - starting fresh");
    dailySteps = 0;
    totalSteps = 0;
    lastResetDay = 0;
    lastResetMonth = 0;
    lastResetYear = 0;
  }
  
  // Display time source status
  Serial.print("Time source: ");
  Serial.println(IMU_getTimeSource());
  
  nextMicros = micros() + SAMPLE_US;
#ifdef SC_DEBUG_IMU
  Serial.println("\nSmart Cane System Ready");
  Serial.println("Features: Robust step detection, Advanced fall detection, Slope warning, Recalibration");
  Serial.println("Step counting with daily reset at 23:59 PKT");
  Serial.println("BTN3 (Blue) functions:");
  Serial.println("  Short press: Recalibrate sensors");
  Serial.println("  3s press: Reset daily steps");
  Serial.println("  10s press: Factory reset (clears all data)");
#endif
}

void IMU_update(SensorData* data) {
  uint32_t now = micros();
  if (now < nextMicros) return;
  nextMicros += SAMPLE_US;
  updateFeedback();
  static uint32_t buttonPressTime = 0;
  static bool buttonActive = false;
  static bool factoryResetDone = false;
  static bool stepResetDone = false;
  if (digitalRead(CALIB_BUTTON_PIN) == LOW) {
    if (!buttonActive) { 
      buttonActive = true; 
      buttonPressTime = millis(); 
      factoryResetDone = false; 
      stepResetDone = false;
    }
    else if (millis() - buttonPressTime >= 10000 && !factoryResetDone) {
      factoryResetEEPROM(); 
      factoryResetDone = true; 
      Serial.println("EEPROM Factory Reset");
      digitalWrite(FEEDBACK_PIN, HIGH); 
      delay(1000); 
      digitalWrite(FEEDBACK_PIN, LOW);
    }
    else if (millis() - buttonPressTime >= 3000 && !stepResetDone && !factoryResetDone) {
      resetDailySteps();
      stepResetDone = true;
      Serial.println("Daily steps manually reset");
      digitalWrite(FEEDBACK_PIN, HIGH);
      delay(500);
      digitalWrite(FEEDBACK_PIN, LOW);
    }
  } else if (buttonActive) {
    buttonActive = false;
    if (!factoryResetDone && !stepResetDone && millis() - buttonPressTime < 3000) {
      calibrateSensors();
    }
    factoryResetDone = false;
    stepResetDone = false;
  }
  int16_t axr, ayr, azr, gxr, gyr, gzr;
  if (!readMPUData(axr, ayr, azr, gxr, gyr, gzr)) { handleI2CError(); return; }
  i2cErrorCount = 0;
  axr -= accelOffsets[0]; ayr -= accelOffsets[1]; azr -= accelOffsets[2];
  gxr -= gyroOffsets[0]; gyr -= gyroOffsets[1]; gzr -= gyroOffsets[2];
  float ax_g = axr / 4096.0f;
  float ay_g = ayr / 4096.0f;
  float az_g = azr / 4096.0f;
  float gx_dps = gxr / 32.8f;
  float gy_dps = gyr / 32.8f;
  float gz_dps = gzr / 32.8f;
  axBuf[bufIdx] = axr; ayBuf[bufIdx] = ayr; azBuf[bufIdx] = azr;
  gxBuf[bufIdx] = gxr; gyBuf[bufIdx] = gyr; gzBuf[bufIdx] = gzr;
  bufIdx = (bufIdx + 1) % 5;
  float ax = median5(axBuf) / 4096.0f;
  float ay = median5(ayBuf) / 4096.0f;
  float az = median5(azBuf) / 4096.0f;
  float gx = median5(gxBuf) / 32.8f;
  float gy = median5(gyBuf) / 32.8f;
  float gz = median5(gzBuf) / 32.8f;
  smoothAx = SMOOTH_ALPHA * ax + (1 - SMOOTH_ALPHA) * smoothAx;
  smoothAy = SMOOTH_ALPHA * ay + (1 - SMOOTH_ALPHA) * smoothAy;
  smoothAz = SMOOTH_ALPHA * az + (1 - SMOOTH_ALPHA) * smoothAz;
  smoothGx = SMOOTH_ALPHA * gx + (1 - SMOOTH_ALPHA) * smoothGx;
  smoothGy = SMOOTH_ALPHA * gy + (1 - SMOOTH_ALPHA) * smoothGy;
  smoothGz = SMOOTH_ALPHA * gz + (1 - SMOOTH_ALPHA) * smoothGz;
  float accelDelta = abs(ax - smoothAx) + abs(ay - smoothAy) + abs(az - smoothAz);
  float gyroDelta = abs(gx - smoothGx) + abs(gy - smoothGy) + abs(gz - smoothGz);
  motionEnergy = 0.95f * motionEnergy + 0.05f * (accelDelta + gyroDelta * 0.1f);
  filter.updateIMU(
    smoothGx * DEG_TO_RAD,
    smoothGy * DEG_TO_RAD,
    smoothGz * DEG_TO_RAD,
    smoothAx, smoothAy, smoothAz
  );
  float roll = filter.getRoll();
  float pitch = filter.getPitch();
  float yaw = filter.getYaw();
  float currentAlpha = (motionEnergy > MOTION_THRESH) ? FAST_ALPHA : SMOOTH_ALPHA;
  smoothRoll = currentAlpha * roll + (1 - currentAlpha) * smoothRoll;
  smoothPitch = currentAlpha * pitch + (1 - currentAlpha) * smoothPitch;
  smoothYaw = currentAlpha * yaw + (1 - currentAlpha) * smoothYaw;
  detectSteps(now);
  detectFalls(now);
  detectSlope();
  
  // Check for daily step reset
  static uint32_t lastResetCheck = 0;
  if (now - lastResetCheck >= 60000000) { // Check every minute
    lastResetCheck = now;
    if (shouldResetDailySteps()) {
      resetDailySteps();
    }
  }
  
  static uint32_t lastPrint = 0;
  if (now - lastPrint >= 100000) {
    lastPrint = now;
    char stepIndicator = (stepState != STEP_IDLE) ? 'S' : ' ';
    char fallIndicator = (fallState != FALL_NONE) ? 'F' : ' ';
    char slopeIndicator = slopeWarningActive ? 'W' : ' ';
    Serial.printf("[%c%c%c] Daily:%lu Total:%lu | R:%5.1f P:%5.1f | Motion:%4.2f | Time:%s\n",
      stepIndicator, fallIndicator, slopeIndicator,
      dailySteps, totalSteps, smoothRoll, smoothPitch, motionEnergy, IMU_getTimeSource().c_str());
    Serial.print("Pitch: [");
    int pitchPos = map(constrain(smoothPitch, -30, 30), -30, 30, 0, 20);
    for (int i = 0; i < 20; i++) {
      Serial.print(i == pitchPos ? '^' : (i == 10 ? '|' : ' '));
    }
    Serial.print("] ");
    Serial.print(smoothPitch, 1);
    Serial.println("°");
  }
  if (data) {
    data->imuPitch = smoothPitch;
    data->imuRoll = smoothRoll;
    data->imuYaw = smoothYaw;
    data->dailySteps = dailySteps;
    data->totalSteps = totalSteps;
    data->lastResetDay = lastResetDay;
    data->lastResetMonth = lastResetMonth;
    data->lastResetYear = lastResetYear;
  }
}

// Getter functions for BLE access
FallState IMU_getFallState() {
  return fallState;
}

float IMU_getMotionEnergy() {
  return motionEnergy;
}

bool IMU_getSlopeWarningActive() {
  return slopeWarningActive;
}