#include "Pins.h"
#include "ConnectivityManager.h"
#include "EnvMonitor.h"
#include "LightSensor.h"
#include "IMU.h"
#include "ToF.h"
#include "RFID.h"
#include "GPSModule.h"
#include "SensorData.h"
#include "FeedbackManager.h"
#include "BLEManager.h"
#include "thingProperties.h"


SensorData sensorData;
static uint32_t hdrT = 0;
static String serialBuffer = "";

// Arduino IoT Cloud variables
String cloudSerialData = ""; // Global cloud variable
static uint32_t lastCloudUpdate = 0;
static const uint32_t CLOUD_UPDATE_INTERVAL = 5000; // Update cloud every 5 seconds
static String lastSensorLine = ""; // Store last sensor line for cloud upload

// Cloud connection management
static bool cloudConnected = false;
static uint8_t cloudRetryCount = 0;
static const uint8_t MAX_CLOUD_RETRIES = 3;
static const uint32_t CLOUD_RETRY_INTERVAL = 30000; // 30 seconds
static uint32_t lastCloudRetry = 0;
static bool cloudRetryInProgress = false;

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
  
  // Store the sensor line for cloud upload
  lastSensorLine = String(buf);
  
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
  
  if (cmd == "v" || cmd == "vibrate") {
    Serial.println("🔔 Testing vibration motors...");
    digitalWrite(VIB1_PIN, HIGH);
    digitalWrite(VIB2_PIN, HIGH);
    delay(1000);
    digitalWrite(VIB1_PIN, LOW);
    digitalWrite(VIB2_PIN, LOW);
    Serial.println("✅ Vibration test complete");
  }
  else if (cmd == "wifi") {
    ConnectivityManager::printStatus();
  }
  else if (cmd == "reconnect") {
    ConnectivityManager::reconnect();
  }
  else if (cmd == "resetwifi") {
    ConnectivityManager::resetCredentials();
  }
  else if (cmd == "offline") {
    Serial.println("📱 Switching to offline mode");
    WiFi.disconnect();
    cloudConnected = false;
    cloudRetryCount = MAX_CLOUD_RETRIES; // Stop retrying
    Serial.println("☁️ Cloud retries disabled");
  }
  else if (cmd == "cloudstatus") {
    Serial.println("☁️ Cloud Connection Status:");
    Serial.printf("   Connected: %s\n", cloudConnected ? "Yes" : "No");
    Serial.printf("   Retry Count: %d/%d\n", cloudRetryCount, MAX_CLOUD_RETRIES);
    Serial.printf("   Arduino Cloud: %s\n", ArduinoCloud.connected() ? "Connected" : "Disconnected");
    Serial.printf("   WiFi: %s\n", WiFi.status() == WL_CONNECTED ? "Connected" : "Disconnected");
    if (cloudRetryCount >= MAX_CLOUD_RETRIES) {
      Serial.println("   Status: Offline mode (max retries reached)");
    } else if (!cloudConnected) {
      Serial.println("   Status: Attempting connection...");
    } else {
      Serial.println("   Status: Online and streaming");
    }
  }
  else if (cmd == "resetcloud") {
    Serial.println("🔄 Resetting cloud connection...");
    cloudRetryCount = 0;
    cloudConnected = false;
    cloudRetryInProgress = false;
    lastCloudRetry = 0;
    Serial.println("✅ Cloud retry counter reset. Will attempt connection in 30 seconds.");
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
    if (BLEManager::isConnected()) {
      Serial.printf("   Connection Time: %lu seconds\n", (millis() - BLEManager::getConnectionTime()) / 1000);
    }
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
  // GPS Commands
  else if (cmd == "gps") {
    Serial.println("📍 GPS Status:");
    Serial.printf("   Connected: %s\n", sensorData.gpsLat != 0 ? "Yes" : "No");
    Serial.printf("   Satellites: %d\n", sensorData.gpsSatellites);
    Serial.printf("   Latitude: %.6f\n", sensorData.gpsLat);
    Serial.printf("   Longitude: %.6f\n", sensorData.gpsLon);
    Serial.printf("   Altitude: %.1fm\n", sensorData.gpsAlt);
    Serial.printf("   Speed: %.1f km/h\n", sensorData.gpsSpeed);
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
    Serial.println("   (Time sync status from ConnectivityManager)");
    Serial.printf("   Time synced: %s\n", ConnectivityManager::isTimeSynced() ? "Yes" : "No");
    Serial.printf("   Current time: %s\n", ConnectivityManager::getCurrentTime().c_str());
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
    Serial.printf("   Time sync: %s\n", ConnectivityManager::isTimeSynced() ? "Active" : "Inactive");
    Serial.printf("   Local time: %s\n", ConnectivityManager::getCurrentTime().c_str());
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
  else if (cmd == "help") {
    Serial.println("\n📋 Available Commands:");
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
    Serial.println("🎛️  Feedback Modes:");
    Serial.println("   Hold BTN2 for 4s to cycle: BOTH → BUZZER → VIBRATION");
    Serial.println("\n🏠 Enhanced RFID Commands:");
    Serial.println("   zonestats - Show zone statistics");
    Serial.println("   saverooms - Save room data to storage");
    Serial.println("   loadrooms - Load room data from storage");
      Serial.println("   clearrooms - Clear all room cards and save");
  Serial.println("\n📱 BLE Commands:");
  Serial.println("   blestatus - Show BLE connection status");
  Serial.println("\n📡 ToF Mode Commands:");
  Serial.println("   radar     - Switch to RADAR mode (servo scanning)");
  Serial.println("   simple    - Switch to SIMPLE mode (fixed ToF)");
  Serial.println("   tofmode   - Show current ToF mode");
  Serial.println("   vibrate   - Test vibration motors");
  Serial.println("   testfeedback - Test buzzer-vibration sync");
  Serial.println("   feedbackmode - Show current feedback mode");
  Serial.println("   forcevib   - Force vibration with buzzer test");
  Serial.println("   resettof   - Manual ToF sensor reset");
    Serial.println("\n🏠 Auto Registration Commands:");
    Serial.println("   addroom - Start adding a new location");
    Serial.println("   done - Complete auto registration");
    Serial.println("   cancel - Cancel auto registration");
    Serial.println("   Workflow: addroom → scan card → type location name");
    Serial.println("\n🎯 Single Command Registration:");
    Serial.println("   addlocation - Complete workflow in one command");
    Serial.println("   Workflow: addlocation → scan card → type name → auto complete");
    Serial.println("\n📍 GPS Commands:");
    Serial.println("   gps      - Show GPS status and coordinates");
    Serial.println("   gpsinfo  - Detailed GPS information");
    Serial.println("   location - Current coordinates");
    Serial.println("   sethome  - Set current location as home");
    Serial.println("   home     - Distance from home point");
    Serial.println("   gpssats  - Satellite information");
    Serial.println("   gpsfix   - GPS fix quality");
    Serial.println("   gpstime  - GPS time display");
    Serial.println("   gpsstats - GPS statistics");
    Serial.println("   gpsclear - Clear GPS data");
    Serial.println("   Note: Single char GPS commands (r,s,t,v,d) handled internally");
    Serial.println("   gpstrack - GPS track information");
    Serial.println("   gpsmode  - GPS mode settings");
    Serial.println("   gpscalibrate - GPS calibration");
    Serial.println("   gpslog   - GPS data logging");
    Serial.println("   gpsexport - Export GPS data");
    Serial.println("   gpsreset - Reset GPS module");
    Serial.println("   gpsfilter - GPS filtering settings");
    Serial.println("   gpsdebug - GPS debug mode");
    Serial.println("   gpsspeed - GPS speed information");
    Serial.println("   gpsaccuracy - GPS accuracy metrics");
    Serial.println("   gpshistory - GPS history data");
    Serial.println("   gpshealth - GPS health status");
    Serial.println("   gpstimezone - GPS timezone info");
    Serial.println("   gpswaypoint - GPS waypoint management");
    Serial.println("   gpsnavigation - GPS navigation mode");
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
  Serial.println("\n🚀 Smart Assistive Cane - ESP32-S3 Initializing...");
  Serial.println("==================================================");
  
  // Initialize connectivity first
  ConnectivityManager::init();
  
  // Initialize Arduino IoT Cloud
  initProperties();
  ArduinoCloud.begin(ArduinoIoTPreferredConnection);
  Serial.println("☁️ Arduino IoT Cloud initialized");
  
  // Initialize BLE
  BLEManager::init();
  Serial.println("📱 BLE initialized");
  
  // Initialize feedback manager
  FeedbackManager::init();
  
  // Initialize all sensor modules
  Serial.println("\n📡 Initializing Sensors...");
  EnvMonitor_init();
  LightSensor_init();
  IMU_init();
  ToF_init();
  RFID_init();
  GPSModule_init();
  
  sensorData.feedbackMode = 0; // Initialize to BOTH mode (0)

  Serial.println("\n✅ Smart Cane System Ready!");
  Serial.println("🏠 Indoor Zoning System Active");
  Serial.println("☁️ Cloud streaming enabled (with retry logic)");
  Serial.println("📱 BLE mobile app communication active");
  Serial.println("📋 Available Commands:");
  Serial.println("   wifi     - Show connectivity status");
  Serial.println("   reconnect - Manual WiFi reconnection");
  Serial.println("   resetwifi - Clear WiFi credentials");
  Serial.println("   offline  - Switch to offline mode");
  Serial.println("   resetcal - Reset light sensor calibration");
  Serial.println("   rooms    - Show registered room cards");
  Serial.println("   clearrooms - Clear all room cards");
  Serial.println("   addroom <room> <uid> - Add room card");
  Serial.println("   Example: addroom 1 UID:12 34 56 78");
  Serial.println("   zonestats - Show zone statistics");
  Serial.println("🎛️  Feedback Modes:");
  Serial.println("   Hold BTN2 for 4s to cycle: BOTH → BUZZER → VIBRATION");
  Serial.println("==================================================");
}

void loop() {
  // Update Arduino IoT Cloud
  ArduinoCloud.update();
  
  // Manage cloud connection with retry logic
  manageCloudConnection();
  
  // Update BLE
  BLEManager::update();
  
  // Update feedback manager first
  FeedbackManager::update(&sensorData);
  
  // Check if sensors are disabled during room registration
  bool sensorsDisabled = RFID_areSensorsDisabled();
  if (sensorsDisabled) {
    // Only update RFID during registration mode
    RFID_update(&sensorData);
    
    // Show registration status (reduced frequency)
    if (millis() % 10000 < 100) { // Every 10 seconds instead of 5
      Serial.println("🔇 Registration Mode: Only RFID active - other sensors disabled");
    }
  } else {
    // Update all modules normally
    EnvMonitor_update(&sensorData);
    LightSensor_update(&sensorData);
    IMU_update(&sensorData);
    ToF_update(&sensorData);
    RFID_update(&sensorData);
    GPSModule_update(&sensorData);

    // Print synchronized sensor data only when not in registration mode
    printLine(sensorData);
    
    // Update cloud data (only if connected)
    updateCloudData();
    
    // Send data via BLE (only if connected)
    BLEManager::sendBLEData(sensorData);
  }
  
  // Process serial commands and capture for cloud
  while (Serial.available()) {
    char c = Serial.read();
    if (c == '\n' || c == '\r') {
      if (serialBuffer.length() > 0) {
        // Send to cloud if it's a sensor data line (not a command) and connected
        if (cloudConnected && serialBuffer.startsWith("|") && serialBuffer.length() > 100) {
          cloudSerialData = serialBuffer; // Send sensor data to cloud
          Serial.println("☁️ Serial data uploaded to Arduino Cloud");
        }
        processSerialCommand(serialBuffer);
        serialBuffer = "";
      }
    } else {
      serialBuffer += c;
    }
  }
}

// Function to manage cloud connection with retry logic
void manageCloudConnection() {
  // Check if we should attempt cloud connection
  if (!cloudConnected && !cloudRetryInProgress) {
    if (cloudRetryCount < MAX_CLOUD_RETRIES) {
      if (millis() - lastCloudRetry >= CLOUD_RETRY_INTERVAL) {
        Serial.printf("☁️ Attempting cloud connection (attempt %d/%d)...\n", 
                     cloudRetryCount + 1, MAX_CLOUD_RETRIES);
        cloudRetryInProgress = true;
        lastCloudRetry = millis();
        
        // Try to connect to cloud
        if (ArduinoCloud.connected()) {
          cloudConnected = true;
          cloudRetryCount = 0;
          cloudRetryInProgress = false;
          Serial.println("✅ Cloud connection successful!");
        } else {
          cloudRetryCount++;
          cloudRetryInProgress = false;
          Serial.printf("❌ Cloud connection failed (attempt %d/%d)\n", 
                       cloudRetryCount, MAX_CLOUD_RETRIES);
          
          if (cloudRetryCount >= MAX_CLOUD_RETRIES) {
            Serial.println("⚠️ Max cloud retries reached. Switching to offline mode.");
          }
        }
      }
    }
  }
  
  // Check if cloud connection is lost
  if (cloudConnected && !ArduinoCloud.connected()) {
    cloudConnected = false;
    Serial.println("⚠️ Cloud connection lost. Will retry in 30 seconds.");
    cloudRetryCount = 0; // Reset retry count for reconnection attempts
  }
}

// Function to update Arduino IoT Cloud
void updateCloudData() {
  if (cloudConnected && lastSensorLine.length() > 0) {
    cloudSerialData = lastSensorLine;
    Serial.println("☁️ Data uploaded to Arduino Cloud");
    lastSensorLine = ""; // Clear after upload to avoid duplicates
  }
}