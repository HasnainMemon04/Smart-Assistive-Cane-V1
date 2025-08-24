#include "LightSensor.h"
#include "Pins.h"
#include "SensorHealth.h"
#include <BH1750.h>
#include "SDCardManager.h"
#include <Wire.h>
#include "ConnectivityManager.h"

#define FILTER_SAMPLES 7
#define MAX_VALID_LUX 65535.0
#define DARKNESS_THRESHOLD 0.75
#define PITCH_BLACK_MAX 0.1
#define FLASHLIGHT_THRESHOLD 1500.0
#define FLASHLIGHT_DURATION 2000
#define OVERCAST_DURATION 50000
#define FULL_DAYLIGHT_DURATION 50000
#define DIRECT_SUNLIGHT_DURATION 30000
#define SUNLIGHT_DURATION 120000
#define PITCH_BLACK_DURATION 10000

static BH1750 lightMeter;
// Using SD card for preferences storage
static float luxBuffer[FILTER_SAMPLES];
static uint8_t bufferIndex = 0;
static float darkOffset = 5.0;
static bool calibratedDark = false;
static String currentEnvironment = "Unknown";
static String previousEnvironment = "";
static uint32_t flashlightStart = 0, overcastStart = 0, fullDaylightStart = 0, directSunlightStart = 0, pitchBlackStart = 0;
static bool flashlightActive = false, overcastActive = false, fullDaylightActive = false, directSunlightActive = false, pitchBlackActive = false, glareCompensationActive = false;

static float median3(float a, float b, float c) {
  if ((a <= b && b <= c) || (c <= b && b <= a)) return b;
  if ((b <= a && a <= c) || (c <= a && a <= b)) return a;
  return c;
}

static String formatTime(double decimalHours) {
  int hours = (int)decimalHours;
  int minutes = (int)((decimalHours - hours) * 60 + 0.5);
  char buffer[6];
  sprintf(buffer, "%02d:%02d", hours, minutes);
  return String(buffer);
}

static void updateDayNightStatus() {
  // Use ConnectivityManager for day/night status
  bool isDay = ConnectivityManager::isDaytime();
  Serial.print("Current time: ");
  Serial.println(ConnectivityManager::getCurrentTime());
  Serial.print("Day status: ");
  Serial.println(isDay ? "Daytime" : "Nighttime");
}

static void calibrateSensor() {
  Serial.println("\n=== SENSOR CALIBRATION ===");
  Serial.println("Cover sensor completely for calibration...");
  Serial.println("Starting in 3 seconds...");
  delay(3000);
  const uint8_t calSamples = 15;
  float calSum = 0;
  bool validCal = true;
  float minLux = 10000.0, maxLux = 0.0;
  Serial.println("Calibrating...");
  for (uint8_t i = 0; i < calSamples; i++) {
    float rawLux = lightMeter.readLightLevel();
    if (rawLux < minLux) minLux = rawLux;
    if (rawLux > maxLux) maxLux = rawLux;
    if (rawLux > 10.0) validCal = false;
    calSum += rawLux;
    Serial.print(".");
    delay(100);
  }
  float stability = maxLux - minLux;
  bool stable = (stability < 2.0);
  if (validCal && stable) {
    darkOffset = calSum / calSamples;
    calibratedDark = true;
    Serial.println("\n✅ Dynamic calibration successful");
    Serial.print("Dark offset: ");
    Serial.print(darkOffset, 4);
    Serial.println(" lux");
    SDCard_savePreferences("lightsensor", "darkOffset", darkOffset);
    SDCard_savePreferences("lightsensor", "calibrated", true);
  } else {
    Serial.println("\n⚠️ Calibration failed");
    calibratedDark = false;
    Serial.print("Using safe offset: ");
    Serial.print(darkOffset, 4);
    Serial.println(" lux");
  }
  Serial.println("==========================");
}

static float readCalibratedSensor() {
  float readings[3];
  for (int i = 0; i < 3; i++) {
    readings[i] = lightMeter.readLightLevel();
    delay(10);
  }
  float lux = median3(readings[0], readings[1], readings[2]);
  if (lux >= darkOffset) lux -= darkOffset;
  else lux = 0;
  if (lux > MAX_VALID_LUX) lux = MAX_VALID_LUX;
  if (isnan(lux)) lux = 0;
  return lux;
}

static float filteredLux() {
  static float smoothed = 0;
  const float alpha = 0.3;
  float newest = readCalibratedSensor();
  float oldest = luxBuffer[bufferIndex];
  static float sum = 0;
  sum = sum - oldest + newest;
  luxBuffer[bufferIndex] = newest;
  bufferIndex = (bufferIndex + 1) % FILTER_SAMPLES;
  float filtered = sum / FILTER_SAMPLES;
  smoothed = alpha * filtered + (1 - alpha) * smoothed;
  if (glareCompensationActive) smoothed *= 0.85;
  else if (currentEnvironment == "Pitch Black") smoothed *= 1.15;
  return (smoothed < DARKNESS_THRESHOLD) ? 0 : smoothed;
}

static String classifyEnvironment(float lux) {
  if (lux <= PITCH_BLACK_MAX) return "Pitch Black";
  if (lux <= 10.0) return "Dark Indoor";
  if (lux <= 50.0) return "Dim Indoor";
  if (lux <= 300.0) return "Normal Indoor";
  if (lux <= 1000.0) return "Bright Indoor";
  if (!ConnectivityManager::isDaytime() && lux > FLASHLIGHT_THRESHOLD) return "Flashlight";
  if (lux <= 5000.0) return "Overcast Daylight";
  if (lux <= 10000.0) return "Full Daylight";
  return "Direct Sunlight";
}

static void checkSpecialConditions(float lux, String env) {
  uint32_t currentTime = millis();
  if (lux > FLASHLIGHT_THRESHOLD) {
    if (!flashlightActive) {
      flashlightStart = currentTime;
      flashlightActive = true;
      Serial.println("\n🔦 Flashlight detected...");
    } else if (currentTime - flashlightStart > FLASHLIGHT_DURATION) {
      Serial.println("\n⚠️ WARNING: Someone is shining light at you!");
      flashlightActive = false;
    }
  } else if (flashlightActive) {
    Serial.println("\n🔦 Flashlight removed");
    flashlightActive = false;
  }
  if (env == "Overcast Daylight" && ConnectivityManager::isDaytime()) {
    if (!overcastActive) {
      overcastStart = currentTime;
      overcastActive = true;
      Serial.println("\n🌥️ Overcast Daylight detected...");
    } else if (currentTime - overcastStart > OVERCAST_DURATION) {
      Serial.println("\n🌥️ Overcast Daylight confirmed (50s+)");
      overcastActive = false;
    }
  } else if (overcastActive) {
    Serial.println("\n🌥️ Overcast Daylight ended");
    overcastActive = false;
  }
  if (env == "Full Daylight" && ConnectivityManager::isDaytime()) {
    if (!fullDaylightActive) {
      fullDaylightStart = currentTime;
      fullDaylightActive = true;
      Serial.println("\n🌤️ Full Daylight detected...");
    } else if (currentTime - fullDaylightStart > FULL_DAYLIGHT_DURATION) {
      Serial.println("\n🌤️ Full Daylight confirmed (50s+)");
      fullDaylightActive = false;
    }
  } else if (fullDaylightActive) {
    Serial.println("\n🌤️ Full Daylight ended");
    fullDaylightActive = false;
  }
  if (env == "Direct Sunlight" && ConnectivityManager::isDaytime()) {
    if (!directSunlightActive) {
      directSunlightStart = currentTime;
      directSunlightActive = true;
      Serial.println("\n☀️ Direct sunlight detected...");
    } else {
      if (currentTime - directSunlightStart > DIRECT_SUNLIGHT_DURATION) {
        Serial.println("\n☀️ Direct Sunlight confirmed (30s+)");
      }
      if (currentTime - directSunlightStart > SUNLIGHT_DURATION) {
        glareCompensationActive = true;
        Serial.println("\n🟡 Enabling sunlight glare compensation");
      }
    }
  } else if (directSunlightActive) {
    Serial.println("\n☀️ Direct sunlight ended");
    directSunlightActive = false;
    glareCompensationActive = false;
  }
  if (env == "Pitch Black" && !ConnectivityManager::isDaytime()) {
    if (!pitchBlackActive) {
      pitchBlackStart = currentTime;
      pitchBlackActive = true;
      Serial.println("\n🌑 Pitch black detected...");
    } else if (currentTime - pitchBlackStart > PITCH_BLACK_DURATION) {
      Serial.println("\n🔵 Activating emergency lighting");
      pitchBlackActive = false;
    }
  } else if (pitchBlackActive) {
    Serial.println("\n🌑 Pitch black ended");
    pitchBlackActive = false;
  }
}

static void checkCalibration() {
  static uint32_t lastCheck = 0;
  if (millis() - lastCheck > 3600000) {
    lastCheck = millis();
    if (currentEnvironment == "Pitch Black" && !calibratedDark) {
      Serial.println("\n=== AUTO-RECALIBRATION ===");
      Serial.println("Recalibrating in dark environment...");
      const uint8_t samples = 10;
      float sum = 0;
      for (uint8_t i = 0; i < samples; i++) {
        float rawLux = lightMeter.readLightLevel();
        sum += rawLux;
        delay(100);
      }
      darkOffset = sum / samples;
      calibratedDark = true;
      SDCard_savePreferences("lightsensor", "darkOffset", darkOffset);
      SDCard_savePreferences("lightsensor", "calibrated", true);
      Serial.print("New offset: ");
      Serial.print(darkOffset, 4);
      Serial.println(" lux");
    }
  }
}

static void printEnvironment(float lux, String env) {
  currentEnvironment = env;
  if (currentEnvironment != previousEnvironment) {
    char envBuf[32];
    env.toCharArray(envBuf, sizeof(envBuf));
    Serial.println("\n=== ENVIRONMENT CHANGE ===");
    Serial.print("Detected: ");
    Serial.println(envBuf);
    Serial.print("Day status: ");
    Serial.println(ConnectivityManager::isDaytime() ? "Daytime" : "Nighttime");
    Serial.println("========================");
    previousEnvironment = currentEnvironment;
  }
  Serial.print("Illuminance: ");
  Serial.print(lux, 2);
  Serial.print(" lx | ");
  Serial.println(env);
}

// Helper function for safe Serial input
bool readLine(char* buf, size_t maxLen, uint32_t timeoutMs = 10000) {
  size_t idx = 0;
  uint32_t start = millis();
  while (millis() - start < timeoutMs && idx < maxLen - 1) {
    if (Serial.available()) {
      char c = Serial.read();
      if (c == '\n' || c == '\r') break;
      buf[idx++] = c;
    }
  }
  buf[idx] = '\0';
  return idx > 0;
}

void checkSerialForCalibrationReset() {
  if (Serial.available()) {
    String cmd = Serial.readStringUntil('\n');
    cmd.trim();
    if (cmd == "resetcal") {
      SDCard_clearPreferences("lightsensor");
      Serial.println("Calibration cleared. Please recalibrate or reboot.");
      while (1); // Halt for safety
    }
  }
}

void LightSensor_init() {
  Wire.begin(I2C_SDA, I2C_SCL, 400000);
  
  // Test BH1750 I2C communication
  Wire.beginTransmission(0x23); // BH1750 default address
  uint8_t error = Wire.endTransmission();
  
  if (error != 0) {
    SensorHealthManager::updateSensorHealth("bh1750", SENSOR_DISCONNECTED, nullptr, "BH1750 I2C failed");
    Serial.println("[LightSensor] BH1750 I2C communication failed");
    return;
  }
  
  lightMeter.begin(BH1750::CONTINUOUS_HIGH_RES_MODE_2, 0x23, &Wire);
  
  // Test initial reading
  delay(200); // Allow sensor to stabilize
  float testReading = lightMeter.readLightLevel();
  
  if (isnan(testReading) || testReading < 0) {
    SensorHealthManager::updateSensorHealth("bh1750", SENSOR_INIT_FAILED, nullptr, "BH1750 init failed");
  } else {
    SensorHealthManager::updateSensorHealth("bh1750", SENSOR_OK, String(testReading, 1).c_str());
  }
  
  // Load calibration from SD card
  darkOffset = SDCard_loadPreferences("lightsensor", "darkOffset", 5.0f);
  calibratedDark = SDCard_loadPreferences("lightsensor", "calibrated", false);
  if (calibratedDark) {
    Serial.print("[LightSensor] Loaded calibration: darkOffset = ");
    Serial.print(darkOffset, 4);
    Serial.print(" lux, calibrated = ");
    Serial.println(calibratedDark ? "true" : "false");
  } else {
    Serial.println("[LightSensor] No calibration found. Using default offset.");
  }
  for (uint8_t i = 0; i < FILTER_SAMPLES; i++) luxBuffer[i] = 0;
#ifdef SC_DEBUG_LIGHT
  Serial.println("[LightSensor] Initialized");
#endif
}

void LightSensor_update(SensorData* data) {
  checkSerialForCalibrationReset();
  
  // Update connectivity status
  ConnectivityManager::update();
  
  checkCalibration();
  
  static uint32_t lastUpdate = 0;
  uint32_t now = millis();
  if (now - lastUpdate < 120) return;
  lastUpdate = now;
  
  // --- Debug print for raw sensor value ---
  float rawLux = lightMeter.readLightLevel();
  Serial.print("[DEBUG] Raw BH1750: ");
  Serial.print(rawLux, 2);
  Serial.print(" lx | darkOffset: ");
  Serial.println(darkOffset, 2);
  
  // Check for sensor health
  if (isnan(rawLux) || rawLux < 0) {
    SensorHealthManager::updateSensorHealth("bh1750", SENSOR_ERROR, nullptr, "BH1750 read failed");
  } else {
    SensorHealthManager::updateSensorHealth("bh1750", SENSOR_OK, String(rawLux, 1).c_str());
  }
  // --- End debug print ---
  
  float lux = filteredLux();
  String env = classifyEnvironment(lux);
  checkSpecialConditions(lux, env);
  printEnvironment(lux, env);
  
  if (data) {
    data->lightLux = lux;
    strncpy(data->lightEnvironment, env.c_str(), sizeof(data->lightEnvironment)-1);
    data->lightEnvironment[sizeof(data->lightEnvironment)-1] = '\0';
  }
  
  if (Serial.available()) {
    char c = Serial.read();
    if (c == 'r' || c == 'R') {
      Serial.println("\n\n=== FORCED RECALIBRATION ===");
      calibrateSensor();
      while (Serial.available()) Serial.read();
    }
  }
}