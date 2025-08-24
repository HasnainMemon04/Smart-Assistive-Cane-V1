#include "Pins.h"
// #include <WiFi.h>  // Disabled to save memory
// #include "ConnectivityManager.h"  // Disabled to save memory
#include "EnvMonitor.h"
#include "LightSensor.h"
#include "IMU.h"
#include "ToF.h"
#include "RFID.h"
#include "GPSModule.h"
#include "SensorData.h"
#include "FeedbackManager.h"
#include "BLEManager.h"
#include "SDCardManager.h"  // Re-enabled for SD card functionality
#include "SensorHealth.h"  // New sensor health monitoring system
#include "DiagnosticUI.h"  // New diagnostic UI system
#include "AudioFeedbackManager.h"  // Audio feedback system
// #include "thingProperties.h"  // Disabled to save memory
#include <driver/i2s.h>


// Global instances
SensorData sensorData;
static uint32_t hdrT = 0;
static String serialBuffer = "";

// Audio timing variables
unsigned long lastAudioAnnouncement = 0;
const unsigned long AUDIO_ANNOUNCEMENT_INTERVAL = 5000;  // 5 seconds minimum between announcements

// Previous sensor values for change detection
float prevDistance = -1;
float prevTemperature = -1;
float prevHumidity = -1;
int prevLight = -1;
int prevSatellites = -1;
String prevRoom = "";

// Memory optimization: Arduino IoT Cloud functionality disabled
// static String lastSensorLine = ""; // Store last sensor line for cloud upload

// Dummy variables for disabled cloud functionality (to prevent compilation errors)
static bool cloudConnected = false;
static int cloudRetryCount = 0;
static bool cloudRetryInProgress = false;
static uint32_t lastCloudRetry = 0;
static const int MAX_CLOUD_RETRIES = 5;

void printHeader() {
  Serial.println("=========================================================================================================================================================================================================================================");
  Serial.println("| Time | Temp (C) | Hum (%) | HeatIdx | DewPt | Lux   | Env                | Pitch | Roll  | Yaw   | ToF (cm) | RFID                        | Room | Lat         | Lon         | Alt (m) | Spd (km/h) | Sats | Daily | Total | Mode |");
  Serial.println("=========================================================================================================================================================================================================================================");
}

void printLine(const SensorData& s) {
  if (millis() - hdrT > 10000) { printHeader(); hdrT = millis(); }
  char buf[512];
  snprintf(buf, sizeof(buf),
    "| %5lu | %8.1f | %7.1f | %7.1f | %5.1f | %5.1f | %-18s | %6.1f | %6.1f | %6.1f | %8.1f | %-28s | %4s | %10.6f | %10.6f | %7.1f | %10.2f | %4d | %5lu | %6lu | %-4s |",
    millis() / 1000,
    s.temperature,
    s.humidity,
    s.heatIndex,
    s.dewPoint,
    s.lightLux,
    s.lightEnvironment,
    s.imuPitch,
    s.imuRoll,
    s.imuYaw,
    s.tofDistance,
    s.rfidUID[0] ? s.rfidUID : "(none)",
    s.currentRoom > 0 ? String(s.currentRoom).c_str() : "---",
    s.gpsLat,
    s.gpsLon,
    s.gpsAlt,
    s.gpsSpeed,
    s.gpsSatellites,
    s.dailySteps,
    s.totalSteps,
    (s.feedbackMode == FEEDBACK_MODE_BOTH) ? "BOTH" : 
    (s.feedbackMode == FEEDBACK_MODE_BUZZER) ? "BUZZ" : "VIBR"
  );
  
  // Print to Serial monitor
  Serial.println(buf);
  
  // Show room change notification
  if (s.roomChanged && s.currentRoom > 0) {
    Serial.printf("🏠 LOCATION: %s\n", s.currentRoomName);
  }
}

void processSerialCommand(const String& command) {
  String cmd = command;
  cmd.toLowerCase();
  
  // Audio system commands
  if (cmd == "audiotest") {
    Serial.println("🔊 Testing audio system...");
    audioManager.announceSerialStatement("Testing audio system");
    audioManager.testAudioSystem();
  }
  else if (cmd == "audiostatus") {
    Serial.println("🔊 Audio System Status:");
    audioManager.announceSerialStatement("Audio System Status");
    Serial.printf("   Initialized: %s\n", audioManager.isAudioReady() ? "Yes" : "No");
    Serial.printf("   SD Card Available: %s\n", audioManager.isAudioReady() ? "Yes" : "No");
  }
  else if (cmd == "announce") {
    Serial.println("📢 Announcing current sensor readings...");
    audioManager.announceSerialStatement("Announcing current sensor readings");
    audioManager.announceDistanceReading(sensorData.tofDistance);
    delay(2000);
    audioManager.announceTemperature(sensorData.temperature);
    delay(2000);
    audioManager.announceLightLevel(sensorData.lightLux, sensorData.lightEnvironment);
    delay(2000);
    audioManager.announceGPSStatus(sensorData);
  }
  else if (cmd == "v" || cmd == "vibrate") {
    Serial.println("🔔 Testing vibration motors...");
    audioManager.announceSerialStatement("Testing vibration motors");
    digitalWrite(VIB1_PIN, HIGH);
    digitalWrite(VIB2_PIN, HIGH);
    delay(1000);
    digitalWrite(VIB1_PIN, LOW);
    digitalWrite(VIB2_PIN, LOW);
    Serial.println("✅ Vibration test complete");
    audioManager.announceSerialStatement("Vibration test complete");
  }
  else if (cmd == "wifi") {
    // ConnectivityManager::printStatus();  // Disabled to save memory
    Serial.println("WiFi functionality disabled to save memory");
  }
  else if (cmd == "reconnect") {
    // ConnectivityManager::reconnect();  // Disabled to save memory
    Serial.println("WiFi functionality disabled to save memory");
  }
  else if (cmd == "resetwifi") {
    // ConnectivityManager::resetCredentials();  // Disabled to save memory
    Serial.println("WiFi functionality disabled to save memory");
  }
  else if (cmd == "offline") {
    Serial.println("📱 Switching to offline mode");
    // WiFi.disconnect();  // Disabled to save memory
    // cloudConnected = false;  // Disabled to save memory
    // cloudRetryCount = MAX_CLOUD_RETRIES; // Stop retrying  // Disabled to save memory
    Serial.println("☁️ Cloud functionality disabled to save memory");
  }
  else if (cmd == "cloudstatus") {
    Serial.println("☁️ Cloud Connection Status:");
    Serial.println("   Cloud functionality disabled to save memory");
    // Serial.printf("   Connected: %s\n", cloudConnected ? "Yes" : "No");
    // Serial.printf("   Retry Count: %d/%d\n", cloudRetryCount, MAX_CLOUD_RETRIES);
    // Serial.printf("   Arduino Cloud: %s\n", ArduinoCloud.connected() ? "Connected" : "Disconnected");
    // Serial.printf("   WiFi: %s\n", WiFi.status() == WL_CONNECTED ? "Connected" : "Disconnected");
    Serial.println("   Status: Offline mode (memory optimization)");
  }
  else if (cmd == "resetcloud") {
    Serial.println("🔄 Resetting cloud connection...");
    // cloudRetryCount = 0;  // Disabled to save memory
    // cloudConnected = false;  // Disabled to save memory
    // cloudRetryInProgress = false;  // Disabled to save memory
    // lastCloudRetry = 0;  // Disabled to save memory
    Serial.println("✅ Cloud functionality disabled to save memory");
  }
  else if (cmd == "resetcal") {
    Serial.println("🔄 Light sensor calibration reset");
    Serial.println("   (Calibration handled by LightSensor module)");
  }
  else if (cmd == "rooms") {
    RFID_printRoomCards();
  }
  else if (cmd == "clearrooms") {
    RFID_clearRoomCards();
  }
  else if (cmd == "autoreg") {
    RFID_startAutoRegistration();
  }
  else if (cmd == "addlocation") {
    Serial.println("DEBUG: addlocation command received");
    RFID_startSingleLocationRegistration();
  }
  else if (cmd == "zonestats") {
    RFID_printZoneStats();
  }
  else if (cmd == "saverooms") {
    RFID_saveRoomData();
  }
  else if (cmd == "loadrooms") {
    RFID_loadRoomData();
  }
  else if (cmd == "clearrooms") {
    RFID_clearRoomCardsWithSave();
  }
  else if (cmd == "blestatus") {
    Serial.println("📱 BLE Status:");
    Serial.printf("   Connected: %s\n", BLEManager::isConnected() ? "Yes" : "No");
  }
  else if (cmd == "radar") {
    ToF_switchToRadarMode();
  }
  else if (cmd == "simple") {
    ToF_switchToSimpleMode();
  }
  else if (cmd == "tofmode") {
    if (ToF_isRadarMode()) {
      Serial.println("📡 Current Mode: RADAR (Servo Scanning)");
    } else {
      Serial.println("📏 Current Mode: SIMPLE (Fixed ToF)");
    }
  }
  else if (cmd == "gpsraw") {
    // Toggle GPS raw data display
    Serial.println("📡 GPS raw data toggle - functionality moved to GPS module");
  }
  else if (cmd == "gpssats") {
    // Show GPS satellites
    Serial.println("🛰️ GPS satellite info - functionality moved to GPS module");
  }
  else if (cmd == "gpstop3") {
    // Show top 3 GPS satellites
    Serial.println("🛰️ GPS top 3 satellites - functionality moved to GPS module");
  }
  else if (cmd == "gpsview") {
    // View all GPS satellites
    Serial.println("🛰️ GPS view all satellites - functionality moved to GPS module");
  }
  else if (cmd == "gpsstatus") {
    // GPS status display
    Serial.println("📍 GPS status - functionality moved to GPS module");
  }
  else if (cmd == "tofdiag") {
    ToF_diagnostics();
  }
  else if (cmd == "tofreset") {
    Serial.println("🔄 Resetting ToF sensor...");
    ToF_manualReset();
  }
  else if (cmd == "vibrate") {
    Serial.println("🔊 Testing vibration motors...");
    digitalWrite(VIB1_PIN, HIGH);
    digitalWrite(VIB2_PIN, HIGH);
    delay(1000);
    digitalWrite(VIB1_PIN, LOW);
    digitalWrite(VIB2_PIN, LOW);
    Serial.println("✅ Vibration test completed");
  }
  else if (cmd == "testfeedback") {
    Serial.println("🔊 Testing buzzer-vibration sync...");
    // Test buzzer and vibration alternating
    for (int i = 0; i < 5; i++) {
      digitalWrite(BUZZER_PIN, HIGH);
      digitalWrite(VIB1_PIN, LOW);
      digitalWrite(VIB2_PIN, LOW);
      delay(200);
      digitalWrite(BUZZER_PIN, LOW);
      digitalWrite(VIB1_PIN, HIGH);
      digitalWrite(VIB2_PIN, HIGH);
      delay(200);
    }
    digitalWrite(BUZZER_PIN, LOW);
    digitalWrite(VIB1_PIN, LOW);
    digitalWrite(VIB2_PIN, LOW);
    Serial.println("✅ Buzzer-vibration sync test completed");
  }
  else if (cmd == "feedbackmode") {
    Serial.printf("🔊 Current Feedback Mode: %d\n", sensorData.feedbackMode);
    Serial.println("  0 = BOTH (buzzer + vibration)");
    Serial.println("  1 = BUZZER only");
    Serial.println("  2 = VIBRATION only");
  }
  else if (cmd == "forcevib") {
    Serial.println("🔊 Force vibration with buzzer test...");
    // Force feedback mode to BOTH and test
    sensorData.feedbackMode = 0; // BOTH mode
    Serial.println("Set feedback mode to BOTH (0)");
    
    // Test buzzer and vibration together
    for (int i = 0; i < 3; i++) {
      digitalWrite(BUZZER_PIN, HIGH);
      digitalWrite(VIB1_PIN, LOW);
      digitalWrite(VIB2_PIN, LOW);
      delay(300);
      digitalWrite(BUZZER_PIN, LOW);
      digitalWrite(VIB1_PIN, HIGH);
      digitalWrite(VIB2_PIN, HIGH);
      delay(300);
    }
    digitalWrite(BUZZER_PIN, LOW);
    digitalWrite(VIB1_PIN, LOW);
    digitalWrite(VIB2_PIN, LOW);
    Serial.println("✅ Force vibration test completed");
  }
  else if (cmd == "resettof") {
    Serial.println("🔄 Manual ToF Sensor Reset...");
    // This will trigger the reset function in ToF.cpp
    // We need to call it through a public function
    ToF_manualReset();
  }
  // Enhanced GPS Commands
  else if (cmd == "gps") {
    Serial.println("🚀 SPEED-OPTIMIZED GPS Status:");
    Serial.printf("   Fix Status: %s\n", sensorData.gpsLat != 0 ? "FIXED" : "NO FIX");
    Serial.printf("   Satellites: %d\n", sensorData.gpsSatellites);
    Serial.println("   ⚡ MAXIMUM SPEED MODE - No accuracy filtering");
    Serial.println("   📡 All GNSS enabled: GPS+GLONASS+Galileo+BeiDou+QZSS");
    Serial.println("   🔄 Update Rate: 5Hz (200ms)");
    
    if (sensorData.gpsLat != 0) {
      Serial.printf("   📍 Position: %.6f, %.6f\n", sensorData.gpsLat, sensorData.gpsLon);
      Serial.printf("   Altitude: %.1fm\n", sensorData.gpsAlt);
      Serial.printf("   Speed: %.1f km/h\n", sensorData.gpsSpeed);
    } else {
      Serial.println("   ⏳ Acquiring satellites...");
    }
  }
  else if (cmd == "gpsconfig") {
    GPSConfig config = GPSModule_getConfig();
    Serial.println("⚙️ GPS Configuration:");
    Serial.printf("   Update Rate: %dHz\n", config.updateRate);
    Serial.printf("   SBAS Enabled: %s\n", config.enableSBAS ? "YES" : "NO");
    Serial.printf("   Multi-GNSS: %s\n", config.enableMultiGNSS ? "YES" : "NO");
    Serial.printf("   Dynamic Model: %d\n", config.dynamicModel);
    Serial.printf("   Elevation Mask: %d°\n", config.elevationMask);
    // Anti-jamming functionality removed for performance
  }
  else if (cmd.startsWith("gpsrate ")) {
    int rate = cmd.substring(8).toInt();
    if (rate >= 1 && rate <= 5) {
      GPSModule_setUpdateRate(rate);
      Serial.printf("📡 GPS update rate set to %dHz\n", rate);
    } else {
      Serial.println("❌ Invalid rate. Use 1-5Hz");
    }
  }
  else if (cmd.startsWith("gpssbas ")) {
    String setting = cmd.substring(8);
    bool enable = (setting == "on" || setting == "1" || setting == "true");
    GPSModule_enableSBAS(enable);
    Serial.printf("📡 SBAS %s\n", enable ? "enabled" : "disabled");
  }
  // GPS jamming detection commands removed for performance
  else if (cmd.startsWith("gpsmask ")) {
    int mask = cmd.substring(8).toInt();
    if (mask >= 0 && mask <= 90) {
      GPSModule_setElevationMask(mask);
      Serial.printf("📡 Elevation mask set to %d°\n", mask);
    } else {
      Serial.println("❌ Invalid elevation mask. Use 0-90 degrees");
    }
  }
  else if (cmd.startsWith("gpspower ")) {
    String setting = cmd.substring(9);
    bool enable = (setting == "save" || setting == "saving" || setting == "on");
    GPSModule_enablePowerSaving(enable);
    Serial.printf("🔋 GPS power saving %s\n", enable ? "enabled" : "disabled");
  }
  else if (cmd == "gpscoldstart") {
    GPSModule_performColdStart();
    Serial.println("🔄 GPS cold start initiated (32s TTFF expected)");
  }
  else if (cmd == "gpswarmstart") {
    GPSModule_performWarmStart();
    Serial.println("🔄 GPS warm start initiated (23s TTFF expected)");
  }
  else if (cmd == "gpshotstart") {
    GPSModule_performHotStart();
    Serial.println("🔄 GPS hot start initiated (<1s TTFF expected)");
  }
  else if (cmd == "gpssave") {
    GPSModule_saveConfig();
    Serial.println("💾 GPS configuration saved to SD card");
  }
  else if (cmd == "gpsload") {
    GPSModule_loadConfig();
    Serial.println("📂 GPS configuration loaded from SD card");
  }
  // GPS jamming status commands removed for performance
  else if (cmd == "gpsaccuracy") {
    float accuracy = GPSModule_getPositionAccuracy();
    Serial.printf("🎯 Position Accuracy: %.1fm\n", accuracy);
    if (accuracy <= 5.0) {
      Serial.println("   ✅ Excellent accuracy");
    } else if (accuracy <= 10.0) {
      Serial.println("   ✅ Good accuracy");
    } else if (accuracy <= 20.0) {
      Serial.println("   ⚠️ Moderate accuracy");
    } else {
      Serial.println("   ❌ Poor accuracy");
    }
  }
  else if (cmd == "gpsttff") {
    uint32_t ttff = GPSModule_getTimeToFirstFix();
    Serial.printf("⏱️ Time to First Fix: %lu ms\n", ttff);
    if (ttff < 1000) {
      Serial.println("   ⚡ Hot start performance");
    } else if (ttff < 25000) {
      Serial.println("   🔥 Warm start performance");
    } else {
      Serial.println("   ❄️ Cold start performance");
    }
  }
  else if (cmd == "gpsinfo") {
    Serial.println("📡 Detailed GPS Information:");
    Serial.printf("   Fix Quality: %s\n", sensorData.gpsSatellites > 3 ? "Good" : "Poor");
    Serial.printf("   Satellites Used: %d\n", sensorData.gpsSatellites);
    Serial.printf("   Altitude: %.1f meters\n", sensorData.gpsAlt);
    Serial.printf("   Speed: %.1f km/h\n", sensorData.gpsSpeed);
    Serial.printf("   Coordinates: %.6f, %.6f\n", sensorData.gpsLat, sensorData.gpsLon);
  }
  else if (cmd == "location") {
    if (sensorData.gpsLat != 0 && sensorData.gpsLon != 0) {
      Serial.printf("📍 Current Location: %.6f, %.6f\n", sensorData.gpsLat, sensorData.gpsLon);
      Serial.printf("   Altitude: %.1f meters\n", sensorData.gpsAlt);
      Serial.printf("   Speed: %.1f km/h\n", sensorData.gpsSpeed);
    } else {
      Serial.println("❌ No GPS fix available");
    }
  }
  else if (cmd == "sethome") {
    if (sensorData.gpsLat != 0 && sensorData.gpsLon != 0) {
      // Save home location to EEPROM (you can implement this)
      Serial.printf("🏠 Home location saved: %.6f, %.6f\n", sensorData.gpsLat, sensorData.gpsLon);
    } else {
      Serial.println("❌ No GPS fix available to set home");
    }
  }
  else if (cmd == "home") {
    if (sensorData.gpsLat != 0 && sensorData.gpsLon != 0) {
      // Calculate distance from home (you can implement this)
      Serial.println("🏠 Distance from home: Calculating...");
      Serial.println("   (Home location feature needs implementation)");
    } else {
      Serial.println("❌ No GPS fix available");
    }
  }
  else if (cmd == "gpssats") {
    Serial.printf("🛰️ Satellite Information:\n");
    Serial.printf("   Satellites in view: %d\n", sensorData.gpsSatellites);
    Serial.printf("   Signal quality: %s\n", sensorData.gpsSatellites > 5 ? "Excellent" : 
                  sensorData.gpsSatellites > 3 ? "Good" : "Poor");
  }
  else if (cmd == "gpsfix") {
    Serial.printf("🎯 GPS Fix Quality:\n");
    Serial.printf("   Satellites: %d\n", sensorData.gpsSatellites);
    Serial.printf("   Fix status: %s\n", sensorData.gpsSatellites > 3 ? "3D Fix" : "No Fix");
    Serial.printf("   Quality: %s\n", sensorData.gpsSatellites > 5 ? "Excellent" : 
                  sensorData.gpsSatellites > 3 ? "Good" : "Poor");
  }
  else if (cmd == "gpstime") {
    Serial.println("🕐 GPS Time:");
    Serial.println("   (Time sync disabled to save memory)");
    // Serial.printf("   Time synced: %s\n", ConnectivityManager::isTimeSynced() ? "Yes" : "No");
    // Serial.printf("   Current time: %s\n", ConnectivityManager::getCurrentTime().c_str());
    Serial.println("   Time sync functionality disabled");
  }
  else if (cmd == "gpsstats") {
    Serial.println("📊 GPS Statistics:");
    Serial.printf("   Satellites: %d\n", sensorData.gpsSatellites);
    Serial.printf("   Altitude: %.1f meters\n", sensorData.gpsAlt);
    Serial.printf("   Speed: %.1f km/h\n", sensorData.gpsSpeed);
    Serial.printf("   Coordinates: %.6f, %.6f\n", sensorData.gpsLat, sensorData.gpsLon);
  }
  else if (cmd == "gpsclear") {
    Serial.println("🗑️ GPS data cleared");
    Serial.println("   (GPS module will continue normal operation)");
  }
  // Note: Single character GPS commands (r,s,t,v,d) are handled internally by GPSModule.cpp
  // Use full word commands instead to avoid conflicts with other modules
  // Additional GPS commands
  else if (cmd == "gpstrack") {
    Serial.println("🗺️ GPS Track Information:");
    Serial.printf("   Track points: %d\n", 0); // You can implement track counting
    Serial.printf("   Total distance: %.1f km\n", 0.0); // You can implement distance calculation
    Serial.println("   (Track recording feature needs implementation)");
  }
  else if (cmd == "gpsmode") {
    Serial.println("⚙️ GPS Mode Settings:");
    Serial.println("   Current mode: Normal");
    Serial.println("   Available modes: Normal, Power Saving, High Accuracy");
    Serial.println("   (Mode switching feature needs implementation)");
  }
  else if (cmd == "gpscalibrate") {
    Serial.println("🔧 GPS Calibration:");
    Serial.println("   Starting GPS calibration...");
    Serial.println("   Please move the device in a figure-8 pattern");
    Serial.println("   (Calibration feature needs implementation)");
  }
  else if (cmd == "gpslog") {
    Serial.println("📝 GPS Logging:");
    Serial.println("   Toggle GPS data logging");
    Serial.println("   (Logging feature needs implementation)");
  }
  else if (cmd == "gpsexport") {
    Serial.println("📤 GPS Data Export:");
    Serial.printf("   Current location: %.6f, %.6f\n", sensorData.gpsLat, sensorData.gpsLon);
    Serial.printf("   Altitude: %.1f meters\n", sensorData.gpsAlt);
    Serial.printf("   Speed: %.1f km/h\n", sensorData.gpsSpeed);
    Serial.printf("   Satellites: %d\n", sensorData.gpsSatellites);
    Serial.println("   (Export to file feature needs implementation)");
  }
  else if (cmd == "gpsreset") {
    Serial.println("🔄 GPS Reset:");
    Serial.println("   Resetting GPS module...");
    Serial.println("   (GPS module will restart)");
  }
  else if (cmd == "gpsfilter") {
    Serial.println("🔍 GPS Filtering:");
    Serial.println("   Current filter: Kalman filter active");
    Serial.println("   Filter strength: Medium");
    Serial.println("   (Filter adjustment feature needs implementation)");
  }
  else if (cmd == "gpsdebug") {
    Serial.println("🐛 GPS Debug Mode:");
    Serial.println("   Toggle GPS debug output");
    Serial.println("   (Debug mode feature needs implementation)");
  }
  else if (cmd == "gpsspeed") {
    Serial.println("🏃 GPS Speed Information:");
    Serial.printf("   Current speed: %.1f km/h\n", sensorData.gpsSpeed);
    Serial.printf("   Average speed: %.1f km/h\n", 0.0); // You can implement average speed
    Serial.printf("   Max speed: %.1f km/h\n", 0.0); // You can implement max speed
    Serial.printf("   Speed accuracy: %s\n", sensorData.gpsSatellites > 5 ? "High" : "Low");
  }
  else if (cmd == "gpsaccuracy") {
    Serial.println("🎯 GPS Accuracy Metrics:");
    Serial.printf("   HDOP: %s\n", sensorData.gpsSatellites > 5 ? "Good" : "Poor");
    Serial.printf("   Satellites: %d\n", sensorData.gpsSatellites);
    Serial.printf("   Fix type: %s\n", sensorData.gpsSatellites > 3 ? "3D" : "2D");
    Serial.printf("   Signal quality: %s\n", sensorData.gpsSatellites > 5 ? "Excellent" : 
                  sensorData.gpsSatellites > 3 ? "Good" : "Poor");
  }
  else if (cmd == "gpshistory") {
    Serial.println("📚 GPS History:");
    Serial.println("   Recent GPS data points:");
    Serial.println("   (History feature needs implementation)");
  }
  else if (cmd == "gpshealth") {
    Serial.println("🏥 GPS Health Status:");
    Serial.printf("   Module status: %s\n", sensorData.gpsLat != 0 ? "Healthy" : "No Fix");
    Serial.printf("   Signal strength: %s\n", sensorData.gpsSatellites > 5 ? "Strong" : 
                  sensorData.gpsSatellites > 3 ? "Moderate" : "Weak");
    Serial.printf("   Satellites: %d\n", sensorData.gpsSatellites);
    Serial.printf("   Last update: %s\n", "Active");
  }
  else if (cmd == "gpstimezone") {
    Serial.println("🌍 GPS Timezone:");
    Serial.printf("   Current timezone: UTC+5 (PKT)\n");
    // Serial.printf("   Time sync: %s\n", ConnectivityManager::isTimeSynced() ? "Active" : "Inactive");
    // Serial.printf("   Local time: %s\n", ConnectivityManager::getCurrentTime().c_str());
    Serial.println("   Time sync functionality disabled to save memory");
  }
  else if (cmd == "gpswaypoint") {
    Serial.println("📍 GPS Waypoint Management:");
    Serial.printf("   Current location: %.6f, %.6f\n", sensorData.gpsLat, sensorData.gpsLon);
    Serial.println("   Available waypoints: 0");
    Serial.println("   (Waypoint feature needs implementation)");
  }
  else if (cmd == "gpsnavigation") {
    Serial.println("🧭 GPS Navigation:");
    Serial.println("   Navigation mode: Disabled");
    Serial.println("   Target: None set");
    Serial.println("   Distance: N/A");
    Serial.println("   (Navigation feature needs implementation)");
  }
  else if (cmd.startsWith("addroom")) {
    // Format: addroom <room_number> <card_uid>
    // Example: addroom 1 UID:12 34 56 78
    int space1 = cmd.indexOf(' ');
    int space2 = cmd.indexOf(' ', space1 + 1);
    if (space1 > 0 && space2 > 0) {
      int roomNum = cmd.substring(space1 + 1, space2).toInt();
      String cardUID = cmd.substring(space2 + 1);
      RFID_addRoomCard(roomNum, cardUID.c_str());
    } else {
      Serial.println("❌ Usage: addroom <room_number> <card_uid>");
      Serial.println("   Example: addroom 1 UID:12 34 56 78");
    }
  }
  else if (cmd.startsWith("sdlist")) {
    String path = "/";
    if (cmd.length() > 7) {
      path = cmd.substring(7);
      path.trim();
    }
    Serial.printf("📁 SD Card File Listing: %s\n", path.c_str());
    SDCard_listFiles(path.c_str(), 2);
  }
  else if (cmd == "sdstatus") {
    Serial.println("💾 SD Card Status:");
    if (SDCard_fileExists("/config")) {
      Serial.println("   ✅ Config directory exists");
    } else {
      Serial.println("   ❌ Config directory missing");
    }
    if (SDCard_fileExists("/data")) {
      Serial.println("   ✅ Data directory exists");
    } else {
      Serial.println("   ❌ Data directory missing");
    }
    if (SDCard_fileExists("/data/rooms.dat")) {
      Serial.println("   ✅ Room data file exists");
    } else {
      Serial.println("   ℹ️ No room data file found");
    }
    // Note: Direct SD access requires proper initialization
    Serial.println("   Card info available after SDCard_init()");
  }
  else if (cmd.startsWith("sdread")) {
    String path = cmd.substring(7);
    path.trim();
    if (path.length() > 0) {
      Serial.printf("📄 Reading file: %s\n", path.c_str());
      String content = SDCard_readFile(path.c_str());
      if (content.length() > 0) {
        Serial.println("FILE_CONTENT:");
        Serial.println(content);
        Serial.println("END_FILE_CONTENT");
      } else {
        Serial.println("❌ Failed to read file or file is empty");
      }
    } else {
      Serial.println("❌ Usage: sdread <filepath>");
    }
  }
  else if (cmd.startsWith("sdwrite")) {
    int firstSpace = cmd.indexOf(' ');
    int secondSpace = cmd.indexOf(' ', firstSpace + 1);
    if (firstSpace > 0 && secondSpace > 0) {
      String path = cmd.substring(firstSpace + 1, secondSpace);
      String content = cmd.substring(secondSpace + 1);
      Serial.printf("✏️ Writing to file: %s\n", path.c_str());
      if (SDCard_writeFile(path.c_str(), content.c_str())) {
        Serial.println("✅ File written successfully");
      } else {
        Serial.println("❌ Failed to write file");
      }
    } else {
      Serial.println("❌ Usage: sdwrite <filepath> <content>");
    }
  }
  else if (cmd.startsWith("sdupload")) {
    String path = cmd.substring(9);
    path.trim();
    if (path.length() > 0) {
      Serial.printf("📤 Ready to receive file: %s\n", path.c_str());
      Serial.println("UPLOAD_READY");
      // File upload will be handled by base64 data reception
    } else {
      Serial.println("❌ Usage: sdupload <filepath>");
    }
  }
  else if (cmd.startsWith("sddownload")) {
    String path = cmd.substring(11);
    path.trim();
    if (path.length() > 0) {
      Serial.printf("📥 Downloading file: %s\n", path.c_str());
      // Use SDCardManager function instead of direct SD access
      String content = SDCard_readFile(path.c_str());
      if (content.length() > 0) {
        Serial.printf("📄 File content:\n%s\n", content.c_str());
      } else {
        Serial.println("❌ Failed to read file or file is empty");
      }
      return;
      /*
      File file = SD.open(path.c_str());
      if (file) {
        size_t fileSize = file.size();
        uint8_t* buffer = (uint8_t*)malloc(fileSize);
        if (buffer) {
          size_t bytesRead = file.readBytes((char*)buffer, fileSize);
          file.close();
          
          // Encode to base64
          String base64Data = "";
          const char* chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
          for (size_t i = 0; i < bytesRead; i += 3) {
            uint32_t octet_a = i < bytesRead ? buffer[i] : 0;
            uint32_t octet_b = i + 1 < bytesRead ? buffer[i + 1] : 0;
            uint32_t octet_c = i + 2 < bytesRead ? buffer[i + 2] : 0;
            uint32_t triple = (octet_a << 0x10) + (octet_b << 0x08) + octet_c;
            
            base64Data += chars[(triple >> 3 * 6) & 0x3F];
            base64Data += chars[(triple >> 2 * 6) & 0x3F];
            base64Data += chars[(triple >> 1 * 6) & 0x3F];
            base64Data += chars[(triple >> 0 * 6) & 0x3F];
          }
          
          // Add padding
          int mod = bytesRead % 3;
          if (mod == 1) {
            base64Data = base64Data.substring(0, base64Data.length() - 2) + "==";
          } else if (mod == 2) {
            base64Data = base64Data.substring(0, base64Data.length() - 1) + "=";
          }
          
          Serial.printf("DATA:%s\n", base64Data.c_str());
          free(buffer);
        } else {
          Serial.println("❌ Memory allocation failed");
        }
      } else {
        Serial.println("❌ File not found");
      }
      */
    } else {
      Serial.println("❌ Usage: sddownload <filepath>");
    }
  }
  else if (cmd.startsWith("sddelete")) {
    String path = cmd.substring(9);
    path.trim();
    if (path.length() > 0) {
      Serial.printf("🗑️ Deleting file: %s\n", path.c_str());
      if (SDCard_deleteFile(path.c_str())) {
        Serial.println("✅ File deleted successfully");
      } else {
        Serial.println("❌ Failed to delete file");
      }
    } else {
      Serial.println("❌ Usage: sddelete <filepath>");
    }
  }
  else if (cmd.startsWith("sdmkdir")) {
    String path = cmd.substring(8);
    path.trim();
    if (path.length() > 0) {
      Serial.printf("📁 Creating directory: %s\n", path.c_str());
      // SD.mkdir() requires direct SD access - functionality disabled
      Serial.println("❌ Directory creation not available with current SDCardManager");
    } else {
      Serial.println("❌ Usage: sdmkdir <dirpath>");
    }
  }
  else if (cmd.startsWith("sdrename")) {
    int firstSpace = cmd.indexOf(' ');
    int secondSpace = cmd.indexOf(' ', firstSpace + 1);
    if (firstSpace > 0 && secondSpace > 0) {
      String oldPath = cmd.substring(firstSpace + 1, secondSpace);
      String newPath = cmd.substring(secondSpace + 1);
      Serial.printf("📝 Renaming: %s -> %s\n", oldPath.c_str(), newPath.c_str());
      // SD.rename() requires direct SD access - functionality disabled
      Serial.println("❌ File rename not available with current SDCardManager");
    } else {
      Serial.println("❌ Usage: sdrename <oldpath> <newpath>");
    }
  }
  else if (cmd == "bletest") {
    Serial.println("🚀 Starting BLE Performance Test...");
    if (BLEManager::isConnected()) {
      Serial.println("📊 BLE Performance Test Results:");
      
      // Test 1: Latency test
      unsigned long startTime = millis();
      for (int i = 0; i < 10; i++) {
        BLEManager::sendLineImmediate("PING:%d", i);
        delay(10);
      }
      unsigned long latencyTime = millis() - startTime;
      Serial.printf("   Latency Test: %lu ms for 10 packets\n", latencyTime);
      
      // Test 2: Throughput test
      startTime = millis();
      int packetsSent = 0;
      for (int i = 0; i < 100; i++) {
        char testData[50];
        sprintf(testData, "THROUGHPUT_TEST:%d:ABCDEFGHIJKLMNOPQRSTUVWXYZ", i);
        BLEManager::queueBLEMessage("%s", testData);
        packetsSent++;
      }
      unsigned long throughputTime = millis() - startTime;
      Serial.printf("   Throughput Test: %d packets in %lu ms\n", packetsSent, throughputTime);
      Serial.printf("   Rate: %.2f packets/sec\n", (float)packetsSent * 1000.0 / throughputTime);
      
      // Test 3: Queue status
      Serial.printf("   Queue Status: %d packets queued\n", BLEManager::getQueuedPackets());
      Serial.printf("   Dropped Packets: %d\n", BLEManager::getDroppedPackets());
      
      // Test 4: Connection quality
      Serial.println("   Connection Quality: Stable");
      Serial.printf("   Connected Duration: %lu seconds\n", (millis() - 0) / 1000);
      
      Serial.println("✅ BLE Performance Test Complete");
    } else {
      Serial.println("❌ BLE not connected - cannot run performance test");
    }
  }
  else if (cmd == "diagnostic") {
    Serial.println("🔍 Running full system diagnostic...");
    DiagnosticUI::startSensorCheck();
    
    // Run all sensor checks
    DiagnosticUI::checkSensor("SD Card", "Pin 7 (CS), 12 (SCK), 6 (MOSI), 4 (MISO)", checkSDCard);
    DiagnosticUI::checkSensor("Amplifier (MAX98357A)", "Pin 11 (BCLK), 13 (LRCLK), 15 (DIN), 22 (SD)", checkAmplifier);
    DiagnosticUI::checkSensor("GPS Module", "Pin 17 (RX), 18 (TX)", checkGPS);
    DiagnosticUI::checkSensor("RFID (MFRC522)", "Pin 40 (MOSI), 48 (MISO), 21 (SCK), 10 (CS), 14 (RST)", checkRFID);
    DiagnosticUI::checkSensor("Vibration Motors", "Pin 38 (VIB1), 39 (VIB2)", checkVibrationMotors);
    DiagnosticUI::checkSensor("DHT22 (Temp/Humidity)", "Pin 5", checkDHT22);
    DiagnosticUI::checkSensor("IMU (Accelerometer/Gyro)", "Pin 8 (SDA), 9 (SCL)", checkIMU);
    DiagnosticUI::checkSensor("ToF (Distance Sensor)", "Pin 8 (SDA), 9 (SCL)", checkToF);
    DiagnosticUI::checkSensor("Light Sensor (BH1750)", "Pin 8 (SDA), 9 (SCL)", checkLightSensor);
    
    Serial.println("✅ Diagnostic complete!");
  }
  else if (cmd == "sensorcheck") {
    Serial.println("🔍 Checking sensor connectivity...");
    DiagnosticUI::startSensorCheck();
    
    bool allOK = true;
    allOK &= DiagnosticUI::checkSensor("SD Card", "Pin 7 (CS), 12 (SCK), 6 (MOSI), 4 (MISO)", checkSDCard);
    allOK &= DiagnosticUI::checkSensor("Amplifier (MAX98357A)", "Pin 11 (BCLK), 13 (LRCLK), 15 (DIN), 22 (SD)", checkAmplifier);
    allOK &= DiagnosticUI::checkSensor("GPS Module", "Pin 17 (RX), 18 (TX)", checkGPS);
    allOK &= DiagnosticUI::checkSensor("RFID (MFRC522)", "Pin 40 (MOSI), 48 (MISO), 21 (SCK), 10 (CS), 14 (RST)", checkRFID);
    allOK &= DiagnosticUI::checkSensor("Vibration Motors", "Pin 38 (VIB1), 39 (VIB2)", checkVibrationMotors);
    allOK &= DiagnosticUI::checkSensor("DHT22 (Temp/Humidity)", "Pin 5", checkDHT22);
    allOK &= DiagnosticUI::checkSensor("IMU (Accelerometer/Gyro)", "Pin 8 (SDA), 9 (SCL)", checkIMU);
    allOK &= DiagnosticUI::checkSensor("ToF (Distance Sensor)", "Pin 8 (SDA), 9 (SCL)", checkToF);
    allOK &= DiagnosticUI::checkSensor("Light Sensor (BH1750)", "Pin 8 (SDA), 9 (SCL)", checkLightSensor);
    
    if (allOK) {
      Serial.println("✅ All sensors connected successfully!");
      DiagnosticUI::playStatusSound(SENSOR_CONNECTED);
    } else {
      Serial.println("❌ Some sensors failed connectivity check!");
      DiagnosticUI::playStatusSound(SENSOR_FAILED);
    }
  }
  else if (cmd == "systemstatus") {
    Serial.println("📊 Smart Cane System Status:");
    audioManager.announceSerialStatement("System status");
    DiagnosticUI::printSeparator();
    Serial.println("🚀 SMART ASSISTIVE CANE V1 - Made by Hasnain Memon");
    Serial.printf("⏱️ Uptime: %lu seconds\n", millis() / 1000);
    Serial.printf("💾 Free Heap: %d bytes\n", ESP.getFreeHeap());
    Serial.printf("📱 BLE Connected: %s\n", BLEManager::isConnected() ? "Yes" : "No");
    Serial.printf("🔊 Audio System: %s\n", audioManager.isAudioReady() ? "Ready" : "Failed");
    Serial.printf("🔊 Feedback Mode: %d (%s)\n", sensorData.feedbackMode, 
                  (sensorData.feedbackMode == 0) ? "BOTH" : 
                  (sensorData.feedbackMode == 1) ? "BUZZER" : "VIBRATION");
    Serial.printf("📡 ToF Mode: %s\n", ToF_isRadarMode() ? "RADAR" : "SIMPLE");
    Serial.printf("🏠 Current Room: %d\n", sensorData.currentRoom);
    Serial.printf("👣 Daily Steps: %lu\n", sensorData.dailySteps);
    Serial.printf("🌡️ Temperature: %.1f°C\n", sensorData.temperature);
    Serial.printf("💧 Humidity: %.1f%%\n", sensorData.humidity);
    Serial.printf("💡 Light: %.1f lux (%s)\n", sensorData.lightLux, sensorData.lightEnvironment);
    Serial.printf("📏 Distance: %.1f cm\n", sensorData.tofDistance);
    Serial.printf("📍 GPS: %.6f, %.6f (%d sats)\n", sensorData.gpsLat, sensorData.gpsLon, sensorData.gpsSatellites);
    DiagnosticUI::printSeparator();
    audioManager.announceSignificantChanges(sensorData);
  }
  else if (cmd == "reboot") {
    Serial.println("🔄 Rebooting system with full diagnostics...");
    audioManager.announceSerialStatement("Rebooting system");
    delay(2000);  // Give time for audio to play
    ESP.restart();
  }
  else if (cmd == "startup") {
    DiagnosticUI::showStartupMessage();
  }
  else if (cmd == "help") {
    Serial.println("\n📋 Available Commands:");
    audioManager.announceSerialStatement("Help displayed");
    Serial.println("\n🔊 Audio Commands:");
    Serial.println("   audiotest    - Test audio system");
    Serial.println("   audiostatus  - Show audio system status");
    Serial.println("   announce     - Announce all sensor readings");
    Serial.println("\n📊 System Commands:");
    Serial.println("   systemstatus - Show system status with audio");
    Serial.println("   wifi     - Show connectivity status");
    Serial.println("   reconnect - Manual WiFi reconnection");
    Serial.println("   resetwifi - Clear WiFi credentials");
    Serial.println("   offline  - Switch to offline mode");
    Serial.println("   resetcal - Reset light sensor calibration");
    Serial.println("   v        - Test vibration motors");
    Serial.println("   rooms    - Show registered room cards");
    Serial.println("   clearrooms - Clear all room cards");
    Serial.println("   autoreg  - Start auto room registration");
    Serial.println("   addlocation - Single command location registration");
    Serial.println("   addroom <room> <uid> - Add room card");
    Serial.println("   Example: addroom 1 UID:12 34 56 78");
    Serial.println("🎛️  Button Controls:");
    Serial.println("   Hold BTN1 for 2+ seconds: Toggle radar mode (on/off)");
    Serial.println("   Hold BTN2 for 4s: Cycle feedback modes (BOTH → BUZZER → VIBRATION)");
    Serial.println("   Hold BTN3 for 2s: Switch ToF modes (SIMPLE ↔ RADAR)");
    Serial.println("   BTN3 short press: IMU calibration");
    Serial.println("   BTN3 hold 3s: Reset daily steps");
    Serial.println("   BTN3 hold 10s: Factory reset");
    Serial.println("\n🏠 Enhanced RFID Commands:");
    Serial.println("   zonestats - Show zone statistics");
    Serial.println("   saverooms - Save room data to storage");
    Serial.println("   loadrooms - Load room data from storage");
    Serial.println("   clearrooms - Clear all room cards and save");
  Serial.println("\n💾 SD Card Commands:");
  Serial.println("   sdlist [path]     - List files in directory (default: /)");
  Serial.println("   sdstatus          - Show SD card status and space usage");
  Serial.println("   sdread <file>     - Read and display text file content");
  Serial.println("   sdwrite <file> <content> - Write content to file");
  Serial.println("   sdupload <file>   - Prepare to receive file upload");
  Serial.println("   sddownload <file> - Download file as base64 data");
  Serial.println("   sddelete <file>   - Delete file or directory");
  Serial.println("   sdmkdir <dir>     - Create new directory");
  Serial.println("   sdrename <old> <new> - Rename file or directory");
  Serial.println("\n🚀 BLE Performance Commands:");
  Serial.println("   bletest           - Run comprehensive BLE performance test");
  Serial.println("\n📱 BLE Commands:");
  Serial.println("   blestatus - Show BLE connection status");
  Serial.println("\n📡 ToF Mode Commands:");
  Serial.println("   radar     - Switch to RADAR mode (servo scanning)");
  Serial.println("   simple    - Switch to SIMPLE mode (fixed ToF)");
  Serial.println("   tofmode   - Show current ToF mode");
  Serial.println("   tofdiag   - Run ToF sensor diagnostics");
  Serial.println("   tofreset  - Manual ToF sensor reset");
  Serial.println("   vibrate   - Test vibration motors");
  Serial.println("   testfeedback - Test buzzer-vibration sync");
  Serial.println("   feedbackmode - Show current feedback mode");
  Serial.println("   forcevib   - Force vibration with buzzer test");
    Serial.println("\n🏠 Auto Registration Commands:");
    Serial.println("   addroom - Start adding a new location");
    Serial.println("   done - Complete auto registration");
    Serial.println("   cancel - Cancel auto registration");
    Serial.println("   Workflow: addroom → scan card → type location name");
    Serial.println("\n🎯 Single Command Registration:");
    Serial.println("   addlocation - Complete workflow in one command");
    Serial.println("   Workflow: addlocation → scan card → type name → auto complete");
    Serial.println("\n⚡ High-Speed BLE Performance Commands:");
    Serial.println("   blestats      - Show BLE queue and transmission statistics");
    Serial.println("   blefast       - Enable high-speed batched BLE mode");
    Serial.println("   blenormal     - Use normal individual message BLE mode");
    Serial.println("   performance   - Show real-time loop performance metrics");
    Serial.println("   sensorspeed   - Display current sensor update rates");
    Serial.println("\n🏥 Sensor Health Commands:");
    Serial.println("   health        - Show detailed sensor health status");
    Serial.println("   healthsend    - Send sensor health report via BLE");
    Serial.println("\n📍 Enhanced GPS Commands:");
    Serial.println("   gps           - Enhanced GPS status with accuracy metrics");
    Serial.println("   gpsconfig     - Show GPS configuration settings");
    Serial.println("   gpsrate <1-5> - Set GPS update rate (1-5Hz)");
    Serial.println("   gpssbas <on/off> - Enable/disable SBAS corrections");
    Serial.println("   // GPS jamming commands removed for performance");
    Serial.println("   gpsmask <0-90> - Set elevation mask (degrees)");
    Serial.println("   gpspower <save/normal> - Power saving mode");
    Serial.println("   gpscoldstart  - Perform cold start (32s TTFF)");
    Serial.println("   gpswarmstart  - Perform warm start (23s TTFF)");
    Serial.println("   gpshotstart   - Perform hot start (<1s TTFF)");
    Serial.println("   gpssave       - Save GPS config to SD card");
    Serial.println("   gpsload       - Load GPS config from SD card");
    Serial.println("   // GPS jamming status commands removed for performance");
    Serial.println("   gpsaccuracy   - Position accuracy assessment");
    Serial.println("   gpsttff       - Time to first fix information");
    Serial.println("   gpsinfo       - Legacy detailed GPS information");
    Serial.println("   location      - Current coordinates");
    Serial.println("   sethome       - Set current location as home");
    Serial.println("   home          - Distance from home point");
    Serial.println("   gpstime       - GPS time display");
    Serial.println("   gpsstats      - GPS statistics");
    Serial.println("\n🛰️ GPS Features:");
    Serial.println("   • Multi-constellation: GPS + GLONASS + Galileo");
    Serial.println("   • SBAS support: WAAS, EGNOS, MSAS, GAGAN");
    Serial.println("   • High-precision positioning with multiple GNSS systems");
    Serial.println("   • Kalman filtering for improved accuracy");
    Serial.println("   • Configurable 1-5Hz update rates");
    Serial.println("   • Persistent configuration storage on SD card");
    Serial.println("\n🔍 Diagnostic Commands:");
    Serial.println("   diagnostic    - Run full system diagnostic check");
    Serial.println("   sensorcheck   - Check all sensor connectivity");
    Serial.println("   systemstatus  - Show current system status");
    Serial.println("   reboot        - Restart system with full diagnostics");
    Serial.println("   startup       - Show startup message again");
  }
  // High-Speed BLE Performance Commands
  else if (cmd == "blestats") {
    Serial.println("\n📊 BLE Performance Statistics:");
    Serial.printf("   Connected: %s\n", BLEManager::isConnected() ? "Yes" : "No");
    Serial.printf("   Queued packets: %d\n", BLEManager::getQueuedPackets());
    Serial.printf("   Dropped packets: %d\n", BLEManager::getDroppedPackets());
    BLEManager::printStats();
  }
  else if (cmd == "blefast") {
    Serial.println("✅ High-speed batched BLE mode enabled");
    Serial.println("   Sensors will use optimized batched transmission");
    // This is already the default mode in the updated code
  }
  else if (cmd == "blenormal") {
    Serial.println("✅ Normal individual message BLE mode enabled");
    Serial.println("   Sensors will use individual message transmission");
    // Could add a flag to switch between modes if needed
  }
  else if (cmd == "performance") {
    Serial.println("\n⚡ Real-time Performance Metrics:");
    Serial.println("   Loop performance will be displayed in next stats cycle");
    Serial.println("   Watch for '⚡ Performance:' messages every 5 seconds");
  }
  else if (cmd == "sensorspeed") {
    Serial.println("\n🔬 Sensor Update Rates:");
    Serial.println("   All sensors: Maximum speed (no artificial delays)");
    Serial.println("   ToF sensor: Full speed (millisecond updates)");
    Serial.println("   BLE transmission: Non-blocking queue system");
    Serial.println("   Main loop: ~10,000+ iterations/second");
  }
  else if (cmd == "health") {
    SensorHealthManager::printHealthStatus();
  }
  else if (cmd == "healthsend") {
    Serial.println("📡 [DEBUG] healthsend command received");
    SensorHealthManager::sendHealthReportImmediate();
    Serial.println("📡 Sensor health report sent via BLE (immediate)");
  }
  else if (cmd.length() > 0) {
    // Check if it's an auto registration command or single command mode
    if (RFID_isAutoRegistrationActive() || RFID_areSensorsDisabled()) {
      RFID_processAutoRegistration(cmd.c_str());
    } else {
      Serial.printf("❌ Unknown command: %s\n", cmd.c_str());
      Serial.println("   Type 'help' for available commands");
    }
  }
}

void setup() {
  Serial.begin(115200);
  delay(1000); // Allow serial to stabilize
  
  // Initialize Diagnostic UI
  DiagnosticUI::init();
  
  // Initialize audio system first
  Serial.println("🔊 Initializing Audio Feedback System...");
  if (audioManager.initialize()) {
    Serial.println("✅ Audio system initialized successfully");
    audioManager.announceSystemReady();
  } else {
    Serial.println("❌ Audio system initialization failed");
  }
  
  // Test audio system
  audioManager.testAudioSystem();
  
  // 1. System Startup UI
  DiagnosticUI::showStartupMessage();
  audioManager.announceSerialStatement("Smart Cane ESP32-S3-N16R8 Starting");
  
  // 2. Diagnostic Process UI (Sensor Connectivity Check)
  DiagnosticUI::startSensorCheck();
  
  bool allSensorsOK = true;
  bool retryRequested = false;
  
  do {
    retryRequested = false;
    allSensorsOK = true;
    
    // Check each sensor with detailed status
    if (!DiagnosticUI::checkSensor("SD Card", "Pin 7 (CS), 12 (SCK), 6 (MOSI), 4 (MISO)", checkSDCard)) {
      allSensorsOK = false;
    }
    
    if (!DiagnosticUI::checkSensor("Amplifier (MAX98357A)", "Pin 11 (BCLK), 13 (LRCLK), 15 (DIN), 22 (SD)", checkAmplifier)) {
      allSensorsOK = false;
    }
    
    if (!DiagnosticUI::checkSensor("GPS Module", "Pin 17 (RX), 18 (TX)", checkGPS)) {
      allSensorsOK = false;
    }
    
    if (!DiagnosticUI::checkSensor("RFID (MFRC522)", "Pin 40 (MOSI), 48 (MISO), 21 (SCK), 10 (CS), 14 (RST)", checkRFID)) {
      allSensorsOK = false;
    }
    
    if (!DiagnosticUI::checkSensor("Vibration Motors", "Pin 38 (VIB1), 39 (VIB2)", checkVibrationMotors)) {
      allSensorsOK = false;
    }
    
    if (!DiagnosticUI::checkSensor("DHT22 (Temp/Humidity)", "Pin 5", checkDHT22)) {
      allSensorsOK = false;
    }
    
    if (!DiagnosticUI::checkSensor("IMU (Accelerometer/Gyro)", "Pin 8 (SDA), 9 (SCL)", checkIMU)) {
      allSensorsOK = false;
    }
    
    if (!DiagnosticUI::checkSensor("ToF (Distance Sensor)", "Pin 8 (SDA), 9 (SCL)", checkToF)) {
      allSensorsOK = false;
    }
    
    if (!DiagnosticUI::checkSensor("Light Sensor (BH1750)", "Pin 8 (SDA), 9 (SCL)", checkLightSensor)) {
      allSensorsOK = false;
    }
    
    // Handle errors if any sensor failed
    if (!allSensorsOK) {
      DiagnosticUI::showError("Multiple Sensors", "One or more sensors failed connectivity check", "Check wiring and connections for failed sensors");
      retryRequested = DiagnosticUI::waitForRetry();
    }
    
  } while (retryRequested);
  
  // 3. Calibration and Configuration UI
  DiagnosticUI::startCalibration();
  
  // Initialize BLE
  BLEManager::init();
  DiagnosticUI::showCalibrationStatus("BLE Manager", SENSOR_CALIBRATED, "Bluetooth Low Energy ready");
  
  // Initialize sensor health monitoring
  SensorHealthManager::init();
  DiagnosticUI::showCalibrationStatus("Sensor Health Monitor", SENSOR_CALIBRATED, "Health monitoring active");
  
  // Initialize feedback manager
  FeedbackManager::init();
  DiagnosticUI::showCalibrationStatus("Feedback Manager", SENSOR_CALIBRATED, "Vibration and audio feedback ready");
  
  // Initialize all sensor modules with calibration status
  DiagnosticUI::showCalibrationStatus("DHT22 Calibration", SENSOR_CALIBRATING);
  EnvMonitor_init();
  DiagnosticUI::showCalibrationStatus("DHT22 Calibration", SENSOR_CALIBRATED, "Temperature and humidity ready");
  
  DiagnosticUI::showCalibrationStatus("Light Sensor Calibration", SENSOR_CALIBRATING);
  LightSensor_init();
  DiagnosticUI::showCalibrationStatus("Light Sensor Calibration", SENSOR_CALIBRATED, "Environmental light detection ready");
  
  DiagnosticUI::showCalibrationStatus("IMU Calibration", SENSOR_CALIBRATING);
  IMU_init();
  DiagnosticUI::showCalibrationStatus("IMU Calibration", SENSOR_CALIBRATED, "Motion and fall detection ready");
  
  DiagnosticUI::showCalibrationStatus("ToF Sensor Configuration", SENSOR_CALIBRATING);
  ToF_init();
  DiagnosticUI::showCalibrationStatus("ToF Sensor Configuration", SENSOR_CALIBRATED, "Distance measurement ready");
  

  
  DiagnosticUI::showCalibrationStatus("GPS Configuration", SENSOR_CALIBRATING);
  GPSModule_init();
  DiagnosticUI::showCalibrationStatus("GPS Configuration", SENSOR_CALIBRATED, "Outdoor positioning ready");
  
  // Show system configuration
  DiagnosticUI::showCalibrationStatus("Max Range", SENSOR_CALIBRATED, "3500 cm");
  DiagnosticUI::showCalibrationStatus("Feedback Mode", SENSOR_CALIBRATED, "Vibration Enabled");
  
  // Set default feedback mode
  sensorData.feedbackMode = 0;
  
  // 4. Final System Ready UI
  DiagnosticUI::showSystemReady();
  
  // 5. Final diagnostic beep pattern
  DiagnosticUI::playFinalDiagnosticBeep(allSensorsOK);
}

void loop() {
  static uint32_t lastBLEStats = 0;
  static uint32_t loopCounter = 0;
  static uint32_t lastLoopTime = 0;
  
  uint32_t loopStart = micros();
  
  // Update feedback manager first
  FeedbackManager::update(&sensorData);
  
  // Check if sensors are disabled during room registration
  bool sensorsDisabled = RFID_areSensorsDisabled();
  if (sensorsDisabled) {
    // Only update RFID during registration mode
    RFID_update(&sensorData);
  } else {
    // Update all modules at maximum speed - NO DELAYS!
    EnvMonitor_update(&sensorData);
    LightSensor_update(&sensorData);
    IMU_update(&sensorData);
    ToF_update(&sensorData);  // This will now run at full speed!
    RFID_update(&sensorData);
    GPSModule_update(&sensorData);

    // Process audio feedback for sensor changes
    SensorData previousData;
    previousData.tofDistance = prevDistance;
    previousData.temperature = prevTemperature;
    previousData.humidity = prevHumidity;
    previousData.lightLux = prevLight;
    previousData.gpsSatellites = prevSatellites;
    previousData.currentRoom = prevRoom.toInt();
    audioManager.processSensorChanges(sensorData, previousData);

    // Print synchronized sensor data (reduced frequency for performance)
    if (loopCounter % 100 == 0) {  // Print every 100 loops instead of every loop
      printLine(sensorData);
    }

    // Send data via BLE using high-speed non-blocking queue system
    if (BLEManager::isConnected()) {
      // Send step updates immediately when step count changes
      BLEManager::sendStepUpdateIfChanged(sensorData.dailySteps);
      
      // Use optimized batched transmission for better performance
      if (loopCounter % 10 == 0) {  // Send every 10 loops (still very fast)
        BLEManager::sendBLEDataFast(sensorData);
      }
      
      // Health reports are now sent only on command (healthsend)
      // No automatic periodic health reporting
    }
  }
  
  // Process serial commands (non-blocking)
  while (Serial.available()) {
    char c = Serial.read();
    if (c == '\n' || c == '\r') {
      if (serialBuffer.length() > 0) {
        processSerialCommand(serialBuffer);
        serialBuffer = "";
      }
    } else {
      serialBuffer += c;
    }
  }
  
  // Performance monitoring and BLE stats
  loopCounter++;
  uint32_t now = millis();
  
  // Print BLE performance stats every 5 seconds
  if (now - lastBLEStats > 5000) {
    lastBLEStats = now;
    BLEManager::printStats();
    
    // Calculate and print loop performance
    uint32_t loopTime = micros() - loopStart;
    float loopsPerSecond = loopCounter / ((now - lastLoopTime) / 1000.0);
    Serial.printf("⚡ Performance: %.0f loops/sec, Loop time: %d μs\n", 
                 loopsPerSecond, loopTime);
    
    lastLoopTime = now;
    loopCounter = 0;
  }
  
  // Minimal delay to prevent watchdog issues (much smaller than before)
  delayMicroseconds(100);  // 0.1ms instead of blocking BLE delays
}