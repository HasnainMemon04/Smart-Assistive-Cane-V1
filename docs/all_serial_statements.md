# Complete List of All 270+ Serial Output Statements in Smart Cane ESP32 Project

## 1. SmartCaneESP32N16R8.ino (154 statements)

### System Status Table
```cpp
Serial.println("=========================================================================================================================================================================================================================================");
Serial.println("| Time | Temp (C) | Hum (%) | HeatIdx | DewPt | Lux   | Env                | Pitch | Roll  | Yaw   | ToF (cm) | RFID                        | Room | Lat         | Lon         | Alt (m) | Spd (km/h) | Sats | Daily | Total | Mode |");
Serial.println("=========================================================================================================================================================================================================================================");
Serial.println(buf);
```

### Location and Room Information
```cpp
Serial.printf("🏠 LOCATION: %s\n", s.currentRoomName);
```

### Vibration Motor Testing
```cpp
Serial.println("🔔 Testing vibration motors...");
Serial.println("✅ Vibration test complete");
```

### WiFi/Cloud Functionality (Disabled)
```cpp
Serial.println("WiFi functionality disabled to save memory");
Serial.println("WiFi functionality disabled to save memory");
Serial.println("WiFi functionality disabled to save memory");
Serial.println("📱 Switching to offline mode");
Serial.println("☁️ Cloud functionality disabled to save memory");
Serial.println("☁️ Cloud Connection Status:");
Serial.println("   Cloud functionality disabled to save memory");
// Serial.printf("   Connected: %s\n", cloudConnected ? "Yes" : "No");
// Serial.printf("   Retry Count: %d/%d\n", cloudRetryCount, MAX_CLOUD_RETRIES);
// Serial.printf("   Arduino Cloud: %s\n", ArduinoCloud.connected() ? "Connected" : "Disconnected");
// Serial.printf("   WiFi: %s\n", WiFi.status() == WL_CONNECTED ? "Connected" : "Disconnected");
Serial.println("   Status: Offline mode (memory optimization)");
Serial.println("🔄 Resetting cloud connection...");
Serial.println("✅ Cloud functionality disabled to save memory");
```

### Light Sensor Calibration
```cpp
Serial.println("🔄 Light sensor calibration reset");
Serial.println("   (Calibration handled by LightSensor module)");
```

### Location Registration
```cpp
Serial.println("DEBUG: addlocation command received");
```

### BLE Status
```cpp
Serial.println("📱 BLE Status:");
Serial.printf("   Connected: %s\n", BLEManager::isConnected() ? "Yes" : "No");
```

### ToF Mode Display
```cpp
Serial.println("📡 Current Mode: RADAR (Servo Scanning)");
Serial.println("📏 Current Mode: SIMPLE (Fixed ToF)");
```

### GPS Functionality Messages
```cpp
Serial.println("📡 GPS raw data toggle - functionality moved to GPS module");
Serial.println("🛰️ GPS satellite info - functionality moved to GPS module");
Serial.println("🛰️ GPS top 3 satellites - functionality moved to GPS module");
Serial.println("🛰️ GPS view all satellites - functionality moved to GPS module");
Serial.println("📍 GPS status - functionality moved to GPS module");
```

### Sensor Reset and Testing
```cpp
Serial.println("🔄 Resetting ToF sensor...");
Serial.println("🔊 Testing vibration motors...");
Serial.println("✅ Vibration test completed");
Serial.println("🔊 Testing buzzer-vibration sync...");
Serial.println("✅ Buzzer-vibration sync test completed");
```

### Feedback Mode Information
```cpp
Serial.printf("🔊 Current Feedback Mode: %d\n", sensorData.feedbackMode);
Serial.println("  0 = BOTH (buzzer + vibration)");
Serial.println("  1 = BUZZER only");
Serial.println("  2 = VIBRATION only");
Serial.println("🔊 Force vibration with buzzer test...");
Serial.println("Set feedback mode to BOTH (0)");
Serial.println("✅ Force vibration test completed");
```

### Manual ToF Reset
```cpp
Serial.println("🔄 Manual ToF Sensor Reset...");
```

### Speed-Optimized GPS Status
```cpp
Serial.println("🚀 SPEED-OPTIMIZED GPS Status:");
Serial.printf("   Fix Status: %s\n", sensorData.gpsLat != 0 ? "FIXED" : "NO FIX");
Serial.printf("   Satellites: %d\n", sensorData.gpsSatellites);
Serial.println("   ⚡ MAXIMUM SPEED MODE - No accuracy filtering");
Serial.println("   📡 All GNSS enabled: GPS+GLONASS+Galileo+BeiDou+QZSS");
Serial.println("   🔄 Update Rate: 5Hz (200ms)");
Serial.printf("   📍 Position: %.6f, %.6f\n", sensorData.gpsLat, sensorData.gpsLon);
Serial.printf("   Altitude: %.1fm\n", sensorData.gpsAlt);
Serial.printf("   Speed: %.1f km/h\n", sensorData.gpsSpeed);
Serial.println("   ⏳ Acquiring satellites...");
```

### GPS Configuration
```cpp
Serial.println("⚙️ GPS Configuration:");
Serial.printf("   Update Rate: %dHz\n", config.updateRate);
Serial.printf("   SBAS Enabled: %s\n", config.enableSBAS ? "YES" : "NO");
Serial.printf("   Multi-GNSS: %s\n", config.enableMultiGNSS ? "YES" : "NO");
Serial.printf("   Dynamic Model: %d\n", config.dynamicModel);
Serial.printf("   Elevation Mask: %d°\n", config.elevationMask);
Serial.printf("📡 GPS update rate set to %dHz\n", rate);
Serial.println("❌ Invalid rate. Use 1-5Hz");
Serial.printf("📡 SBAS %s\n", enable ? "enabled" : "disabled");
Serial.printf("📡 Elevation mask set to %d°\n", mask);
Serial.println("❌ Invalid elevation mask. Use 0-90 degrees");
Serial.printf("🔋 GPS power saving %s\n", enable ? "enabled" : "disabled");
```

### GPS Start Commands
```cpp
Serial.println("🔄 GPS cold start initiated (32s TTFF expected)");
Serial.println("🔄 GPS warm start initiated (23s TTFF expected)");
Serial.println("🔄 GPS hot start initiated (<1s TTFF expected)");
```

### GPS Configuration Save/Load
```cpp
Serial.println("💾 GPS configuration saved to SD card");
Serial.println("📂 GPS configuration loaded from SD card");
```

### GPS Accuracy Assessment
```cpp
Serial.printf("🎯 Position Accuracy: %.1fm\n", accuracy);
Serial.println("   ✅ Excellent accuracy");
Serial.println("   ✅ Good accuracy");
Serial.println("   ⚠️ Moderate accuracy");
Serial.println("   ❌ Poor accuracy");
```

### Time to First Fix (TTFF)
```cpp
Serial.printf("⏱️ Time to First Fix: %lu ms\n", ttff);
Serial.println("   ⚡ Hot start performance");
Serial.println("   🔥 Warm start performance");
Serial.println("   ❄️ Cold start performance");
```

### Detailed GPS Information
```cpp
Serial.println("📡 Detailed GPS Information:");
Serial.printf("   Fix Quality: %s\n", sensorData.gpsSatellites > 3 ? "Good" : "Poor");
Serial.printf("   Satellites Used: %d\n", sensorData.gpsSatellites);
Serial.printf("   Altitude: %.1f meters\n", sensorData.gpsAlt);
Serial.printf("   Speed: %.1f km/h\n", sensorData.gpsSpeed);
Serial.printf("   Coordinates: %.6f, %.6f\n", sensorData.gpsLat, sensorData.gpsLon);
```

### Current Location Display
```cpp
Serial.printf("📍 Current Location: %.6f, %.6f\n", sensorData.gpsLat, sensorData.gpsLon);
Serial.printf("   Altitude: %.1f meters\n", sensorData.gpsAlt);
Serial.printf("   Speed: %.1f km/h\n", sensorData.gpsSpeed);
Serial.println("❌ No GPS fix available");
```

### Home Location Management
```cpp
Serial.printf("🏠 Home location saved: %.6f, %.6f\n", sensorData.gpsLat, sensorData.gpsLon);
Serial.println("❌ No GPS fix available to set home");
Serial.println("🏠 Distance from home: Calculating...");
Serial.println("   (Home location feature needs implementation)");
Serial.println("❌ No GPS fix available");
```

### Satellite Information
```cpp
Serial.printf("🛰️ Satellite Information:\n");
Serial.printf("   Satellites in view: %d\n", sensorData.gpsSatellites);
Serial.printf("   Signal quality: %s\n", sensorData.gpsSatellites > 5 ? "Excellent" :
                                      sensorData.gpsSatellites > 3 ? "Good" : "Poor");
```

### GPS Fix Quality
```cpp
Serial.printf("🎯 GPS Fix Quality:\n");
Serial.printf("   Satellites: %d\n", sensorData.gpsSatellites);
Serial.printf("   Fix status: %s\n", sensorData.gpsSatellites > 3 ? "3D Fix" : "No Fix");
Serial.printf("   Quality: %s\n", sensorData.gpsSatellites > 5 ? "Excellent" :
                                  sensorData.gpsSatellites > 3 ? "Good" : "Poor");
```

### GPS Time (Disabled)
```cpp
Serial.println("🕐 GPS Time:");
Serial.println("   (Time sync disabled to save memory)");
// Serial.printf("   Time synced: %s\n", ConnectivityManager::isTimeSynced() ? "Yes" : "No");
// Serial.printf("   Current time: %s\n", ConnectivityManager::getCurrentTime().c_str());
Serial.println("   Time sync functionality disabled");
```

### GPS Statistics
```cpp
Serial.println("📊 GPS Statistics:");
Serial.printf("   Satellites: %d\n", sensorData.gpsSatellites);
Serial.printf("   Altitude: %.1f meters\n", sensorData.gpsAlt);
Serial.printf("   Speed: %.1f km/h\n", sensorData.gpsSpeed);
Serial.printf("   Coordinates: %.6f, %.6f\n", sensorData.gpsLat, sensorData.gpsLon);
```

### GPS Data Management
```cpp
Serial.println("🗑️ GPS data cleared");
Serial.println("   (GPS module will continue normal operation)");
```

### GPS Track Information
```cpp
Serial.println("🗺️ GPS Track Information:");
Serial.printf("   Track points: %d\n", 0); // You can implement track counting
Serial.printf("   Total distance: %.1f km\n", 0.0); // You can implement distance calculation
Serial.println("   (Track recording feature needs implementation)");
```

### GPS Mode Settings
```cpp
Serial.println("⚙️ GPS Mode Settings:");
Serial.println("   Current mode: Normal");
Serial.println("   Available modes: Normal, Power Saving, High Accuracy");
Serial.println("   (Mode switching feature needs implementation)");
```

### GPS Calibration
```cpp
Serial.println("🔧 GPS Calibration:");
Serial.println("   Starting GPS calibration...");
Serial.println("   Please move the device in a figure-8 pattern");
Serial.println("   (Calibration feature needs implementation)");
```

### GPS Logging
```cpp
Serial.println("📝 GPS Logging:");
Serial.println("   Toggle GPS data logging");
Serial.println("   (Logging feature needs implementation)");
```

### GPS Data Export
```cpp
Serial.println("📤 GPS Data Export:");
Serial.printf("   Current location: %.6f, %.6f\n", sensorData.gpsLat, sensorData.gpsLon);
Serial.printf("   Altitude: %.1f meters\n", sensorData.gpsAlt);
Serial.printf("   Speed: %.1f km/h\n", sensorData.gpsSpeed);
Serial.printf("   Satellites: %d\n", sensorData.gpsSatellites);
Serial.println("   (Export to file feature needs implementation)");
```

### GPS Reset
```cpp
Serial.println("🔄 GPS Reset:");
Serial.println("   Resetting GPS module...");
Serial.println("   (GPS module will restart)");
```

### GPS Filtering
```cpp
Serial.println("🔍 GPS Filtering:");
Serial.println("   Current filter: Kalman filter active");
Serial.println("   Filter strength: Medium");
Serial.println("   (Filter adjustment feature needs implementation)");
```

### GPS Debug Mode
```cpp
Serial.println("🐛 GPS Debug Mode:");
Serial.println("   Toggle GPS debug output");
Serial.println("   (Debug mode feature needs implementation)");
```

### GPS Speed Information
```cpp
Serial.println("🏃 GPS Speed Information:");
Serial.printf("   Current speed: %.1f km/h\n", sensorData.gpsSpeed);
Serial.printf("   Average speed: %.1f km/h\n", 0.0); // You can implement average speed
Serial.printf("   Max speed: %.1f km/h\n", 0.0); // You can implement max speed
Serial.printf("   Speed accuracy: %s\n", sensorData.gpsSatellites > 5 ? "High" : "Low");
```

### GPS Accuracy Metrics
```cpp
Serial.println("🎯 GPS Accuracy Metrics:");
Serial.printf("   HDOP: %s\n", sensorData.gpsSatellites > 5 ? "Good" : "Poor");
Serial.printf("   Satellites: %d\n", sensorData.gpsSatellites);
Serial.printf("   Fix type: %s\n", sensorData.gpsSatellites > 3 ? "3D" : "2D");
Serial.printf("   Signal quality: %s\n", sensorData.gpsSatellites > 5 ? "Excellent" :
                                      sensorData.gpsSatellites > 3 ? "Good" : "Poor");
```

### GPS History
```cpp
Serial.println("📚 GPS History:");
Serial.println("   Recent GPS data points:");
Serial.println("   (History feature needs implementation)");
```

### GPS Health Status
```cpp
Serial.println("🏥 GPS Health Status:");
Serial.printf("   Module status: %s\n", sensorData.gpsLat != 0 ? "Healthy" : "No Fix");
Serial.printf("   Signal strength: %s\n", sensorData.gpsSatellites > 5 ? "Strong" :
                                        sensorData.gpsSatellites > 3 ? "Medium" : "Weak");
Serial.printf("   Satellites: %d\n", sensorData.gpsSatellites);
Serial.printf("   Last update: %s\n", "Active");
```

### GPS Timezone
```cpp
Serial.println("🌍 GPS Timezone:");
Serial.printf("   Current timezone: UTC+5 (PKT)\n");
// Serial.printf("   Time sync: %s\n", ConnectivityManager::isTimeSynced() ? "Active" : "Inactive");
// Serial.printf("   Local time: %s\n", ConnectivityManager::getCurrentTime().c_str());
Serial.println("   Time sync functionality disabled to save memory");
```

### GPS Waypoint Management
```cpp
Serial.println("📍 GPS Waypoint Management:");
Serial.printf("   Current location: %.6f, %.6f\n", sensorData.gpsLat, sensorData.gpsLon);
Serial.println("   Available waypoints: 0");
Serial.println("   (Waypoint feature needs implementation)");
```

### GPS Navigation
```cpp
Serial.println("🧭 GPS Navigation:");
Serial.println("   Navigation mode: Disabled");
Serial.println("   Target: None set");
Serial.println("   Distance: N/A");
Serial.println("   (Navigation feature needs implementation)");
```

### RFID Room Management
```cpp
Serial.println("❌ Usage: addroom <room_number> <card_uid>");
Serial.println("   Example: addroom 1 UID:12 34 56 78");
```

### SD Card File Operations
```cpp
Serial.printf("📁 SD Card File Listing: %s\n", path.c_str());
Serial.println("💾 SD Card Status:");
Serial.println("   ✅ Config directory exists");
Serial.println("   ❌ Config directory missing");
Serial.println("   ✅ Data directory exists");
Serial.println("   ❌ Data directory missing");
Serial.println("   ✅ Room data file exists");
Serial.println("   ℹ️ No room data file found");
Serial.println("   Card info available after SDCard_init()");
Serial.printf("📄 Reading file: %s\n", path.c_str());
Serial.println("FILE_CONTENT:");
Serial.println(content);
Serial.println("END_FILE_CONTENT");
Serial.println("❌ Failed to read file or file is empty");
Serial.println("❌ Usage: sdread <filepath>");
Serial.printf("✏️ Writing to file: %s\n", path.c_str());
Serial.println("✅ File written successfully");
Serial.println("❌ Failed to write file");
Serial.println("❌ Usage: sdwrite <filepath> <content>");
Serial.printf("📤 Ready to receive file: %s\n", path.c_str());
Serial.println("UPLOAD_READY");
Serial.println("❌ Usage: sdupload <filepath>");
Serial.printf("📥 Downloading file: %s\n", path.c_str());
Serial.printf("📄 File content:\n%s\n", content.c_str());
Serial.println("❌ Failed to read file or file is empty");
Serial.printf("DATA:%s\n", base64Data.c_str());
Serial.println("❌ Memory allocation failed");
Serial.println("❌ File not found");
Serial.println("❌ Usage: sddownload <filepath>");
Serial.printf("🗑️ Deleting file: %s\n", path.c_str());
Serial.println("✅ File deleted successfully");
Serial.println("❌ Failed to delete file");
Serial.println("❌ Usage: sddelete <filepath>");
Serial.printf("📁 Creating directory: %s\n", path.c_str());
Serial.println("❌ Directory creation not available with current SDCardManager");
Serial.println("❌ Usage: sdmkdir <dirpath>");
Serial.printf("📝 Renaming: %s -> %s\n", oldPath.c_str(), newPath.c_str());
Serial.println("❌ File rename not available with current SDCardManager");
Serial.println("❌ Usage: sdrename <oldpath> <newpath>");
```

### BLE Performance Testing
```cpp
Serial.println("🚀 Starting BLE Performance Test...");
Serial.println("📊 BLE Performance Test Results:");
Serial.printf("   Latency Test: %lu ms for 10 packets\n", latencyTime);
Serial.printf("   Throughput Test: %d packets in %lu ms\n", packetsSent, throughputTime);
Serial.printf("   Rate: %.2f packets/sec\n", (float)packetsSent * 1000.0 / throughputTime);
Serial.printf("   Queue Status: %d packets queued\n", BLEManager::getQueuedPackets());
Serial.printf("   Dropped Packets: %d\n", BLEManager::getDroppedPackets());
Serial.println("   Connection Quality: Stable");
Serial.printf("   Connected Duration: %lu seconds\n", (millis() - 0) / 1000);
Serial.println("✅ BLE Performance Test Complete");
Serial.println("❌ BLE not connected - cannot run performance test");
```

### System Diagnostics
```cpp
Serial.println("🔍 Running full system diagnostic...");
Serial.println("✅ Diagnostic complete!");
Serial.println("🔍 Checking sensor connectivity...");
Serial.println("✅ All sensors connected successfully!");
Serial.println("❌ Some sensors failed connectivity check!");
```

### System Status Display
```cpp
Serial.println("📊 Smart Cane System Status:");
Serial.println("🚀 SMART ASSISTIVE CANE V1 - Made by Hasnain Memon");
Serial.printf("⏱️ Uptime: %lu seconds\n", millis() / 1000);
Serial.printf("💾 Free Heap: %d bytes\n", ESP.getFreeHeap());
Serial.printf("📱 BLE Connected: %s\n", BLEManager::isConnected() ? "Yes" : "No");
Serial.printf("🔊 Feedback Mode: %d (%s)\n", sensorData.feedbackMode,
              sensorData.feedbackMode == 0 ? "BOTH" :
              sensorData.feedbackMode == 1 ? "BUZZER" : "VIBRATION");
Serial.printf("📡 ToF Mode: %s\n", ToF_isRadarMode() ? "RADAR" : "SIMPLE");
Serial.printf("🏠 Current Room: %d\n", sensorData.currentRoom);
Serial.printf("👣 Daily Steps: %lu\n", sensorData.dailySteps);
Serial.printf("🌡️ Temperature: %.1f°C\n", sensorData.temperature);
Serial.printf("💧 Humidity: %.1f%%\n", sensorData.humidity);
Serial.printf("💡 Light: %.1f lux (%s)\n", sensorData.lightLux, sensorData.lightEnvironment);
Serial.printf("📏 Distance: %.1f cm\n", sensorData.tofDistance);
Serial.printf("📍 GPS: %.6f, %.6f (%d sats)\n", sensorData.gpsLat, sensorData.gpsLon, sensorData.gpsSatellites);
```

### System Reboot
```cpp
Serial.println("🔄 Rebooting system with full diagnostics...");
```

### Help and Commands
```cpp
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
Serial.println("🎛️  Button Controls:");
Serial.println("   Hold BTN1 for 2+ seconds: Toggle radar mode (on/off)");
Serial.println("   Hold BTN2 for 4s: Cycle feedback modes (BOTH → BUZZER → VIBRATION)");
Serial.println("   Hold BTN3 for 2s: Switch ToF modes (SIMPLE ↔ RADAR)");
Serial.println("   BTN3 short press: IMU calibration");
Serial.println("   BTN3 hold 3s: Reset daily steps");
Serial.println("   BTN3 hold 10s: Factory reset");
Serial.println("\n🏠 Enhanced RFID Commands:");
Serial.println("   zonestats - Show zone statistics");
```

## 2. ToF.cpp (47 statements)

### Sensor Reset and Initialization
```cpp
Serial.println("🔄 ToF Sensor Reset - Reinitializing...");
Serial.println("✅ ToF Sensor Reset Complete");
Serial.println("Failed to initialize sensor!");
Serial.println(F("\nVL53L1X Smart Cane System Initialized"));
Serial.println(F("========================================"));
Serial.println(F("Modes: SIMPLE (fixed) / RADAR (scanning)"));
```

### Sensor Status and Warnings
```cpp
Serial.println("⚠️ ToF Sensor appears stuck at max range");
```

### Debug Output
```cpp
Serial.printf("ToF Debug: Raw=%d, Median=%d, Filtered=%.1f, Mode=%s [SIMPLE MODE]\n",
              rawDistance, medianDistance, filteredDistance, "SIMPLE");
Serial.printf("⚠️ Distance change: %.1f cm\n", distanceChange / 10.0f);
```

### Mode Switching
```cpp
//   Serial.println(F("\u21a9\ufe0f Returning to long range mode"));
Serial.print(F("\n🔄 Switched to mode: "));
case SIMPLE_MODE: Serial.println(F("SIMPLE (Fixed ToF)")); break;
case RADAR_MODE: Serial.println(F("RADAR (Servo Scanning)")); break;
```

### Feedback Mode Display
```cpp
Serial.printf("🔊 Feedback Mode: %d (0=BOTH, 1=BUZZER, 2=VIBRATION)\n", feedbackMode);
```

### Stair Detection (Commented)
```cpp
//   Serial.println(F("⛰️ Possible stair detected - activating stair mode"));
```

### Real-time Distance Display
```cpp
Serial.print(F("📏 "));
Serial.print(F("CLEAR"));
Serial.print(F("DANGER "));
Serial.print(distance_cm, 0);
Serial.print(F("cm"));
Serial.print(distance_cm, 0);
Serial.print(F("cm"));
Serial.print(F(" ⚠️"));
Serial.print(F(" | Mode: "));
case SIMPLE_MODE: Serial.print(F("SIMPLE")); break;
case RADAR_MODE: Serial.print(F("RADAR")); break;
if (alertActive) Serial.print(F(" | ⚠️ ALERT"));
Serial.println();
```

### Configuration Display
```cpp
Serial.println(F("\n⚙️ Current Configuration:"));
Serial.print(F("  Max Range: "));
Serial.print((currentMode == SIMPLE_MODE) ? 350 : 350);
Serial.println(F("0cm"));
Serial.print(F("  Current Mode: "));
case SIMPLE_MODE: Serial.println(F("SIMPLE (Fixed ToF)")); break;
case RADAR_MODE: Serial.println(F("RADAR (Servo Scanning)")); break;
Serial.println(F("========================================"));
```

### Mode Switching Messages
```cpp
Serial.println(F("🔄 Switching to RADAR MODE"));
Serial.println(F("🔄 Switching to SIMPLE MODE"));
```

### Radar Task Status
```cpp
Serial.println(F("🔄 Real-time radar task started"));
Serial.println(F("🔄 Radar sweep completed, starting new cycle"));
Serial.println(F("🔄 Real-time radar scanning started"));
```

### Radar Scan Results
```cpp
Serial.println(F("\n📊 RADAR SCAN RESULTS (Key Angles):"));
Serial.println(F("┌─────────┬────────────┬──────────┐"));
Serial.println(F("│ Angle   │ Distance   │ Obstacle? │"));
Serial.println(F("├─────────┼────────────┼──────────┤"));
Serial.printf("│ %6d° │ %8d mm │ %8s │\n",
              angle, scanData[angle], scanData[angle] < 1000 ? "YES" : "NO");
Serial.println(F("└─────────┴────────────┴──────────┘"));
Serial.printf("🎯 SAFEST DIRECTION: %s\n", safestDir);
Serial.printf("📏 CENTER DISTANCE (90°): %d mm\n", scanData[90]);
```

### Sensor Diagnostics
```cpp
Serial.println("\n🔧 ToF Sensor Diagnostics:");
Serial.println("================================");
Serial.printf("I2C Communication: %s\n", error == 0 ? "✅ OK" : "❌ FAILED");
Serial.printf("Current Mode: %s\n", currentMode == RADAR_MODE ? "RADAR" : "SIMPLE");
Serial.printf("Radar Mode Active: %s\n", radarModeActive ? "Yes" : "No");
Serial.print("Sensor Data Ready: ");
Serial.printf("✅ YES (Reading: %d cm)\n", reading);
Serial.println("❌ NO");
Serial.printf("Servo Pin: %d\n", SERVO_PIN);
Serial.println("Servo Status: Attached");
Serial.println("\nTesting single reading with 2s timeout...");
Serial.printf("✅ Test reading successful: %d cm\n", testReading);
Serial.println("❌ Test reading failed - sensor timeout");
Serial.println("💡 Suggestions:");
Serial.println("   - Check I2C wiring (SDA=8, SCL=9)");
Serial.println("   - Check sensor power supply");
Serial.println("   - Try 'tofreset' command");
Serial.println("================================\n");
```

## 3. SensorHealth.cpp (8 statements)

### Manager Initialization
```cpp
Serial.println("SensorHealth Manager initialized");
```

### Debug Messages
```cpp
Serial.println("[SensorHealth] Sending health report...");
Serial.println("[SensorHealth] Rate limited, skipping send");
Serial.println("[SensorHealth] BLE not connected, skipping send");
Serial.println("[SensorHealth] Sending immediate health report");
Serial.println("[SensorHealth] Creating JSON...");
Serial.println("[SensorHealth] Sending data via BLE...");
```

### Health Status Report
```cpp
Serial.println("\n=== SENSOR HEALTH STATUS ===");
```

## 4. SDCardManager.cpp (15 statements)

### Initialization Status
```cpp
Serial.println("SD Card initialization failed!");
Serial.println("Check if SD card is inserted and try again.");
Serial.println("SD Card initialized successfully");
```

### Card Type Detection
```cpp
Serial.println("Card type: MMC");
Serial.println("Card type: SDSC");
Serial.println("Card type: SDHC");
Serial.println("Card type: UNKNOWN");
```

### Card Size Information
```cpp
Serial.printf("SD Card Size: %lluMB\n", cardSize);
```

### Directory Operations
```cpp
Serial.println("Created /config directory");
Serial.println("Created /data directory");
Serial.println("Failed to open directory");
Serial.println("Not a directory");
```

### File Operations
```cpp
Serial.printf("  FILE: %s\t\tSIZE: %d\n", fileName, file.size());
Serial.printf("File written successfully: %s\n", path);
Serial.printf("Failed to write file: %s\n", path);
Serial.printf("File deleted successfully: %s\n", path);
Serial.printf("Failed to delete file: %s\n", path);
Serial.printf("Binary file written: %s, size: %d bytes\n", path, dataSize);
Serial.printf("Failed to write binary file: %s\n", path);
Serial.printf("JSON saved successfully: %s\n", path);
Serial.printf("Failed to save JSON: %s\n", path);
Serial.printf("JSON loaded successfully: %s\n", path);
Serial.printf("Failed to load JSON: %s\n", path);
Serial.printf("JSON parsing failed: %s\n", path);
```

## 5. RFID.cpp (15 statements)

### SPI Communication
```cpp
Serial.println("SPI communication failed, check connections.");
```

### SD Card Usage
```cpp
Serial.println("Using SD card for room data storage");
```

### Ultra-sensitive Mode
```cpp
Serial.println("RFID in ultra-sensitive mode");
```

### System Initialization
```cpp
Serial.println("RFID Zoning System Initialized");
```

### Card Scanning
```cpp
Serial.printf("Card scanned: %s\n", cardUID.c_str());
```

### Location Registration
```cpp
Serial.printf("Location registered: Room %d, Card: %s\n", roomNumber, cardUID.c_str());
Serial.printf("Invalid room number: %d\n", roomNumber);
Serial.printf("Invalid card UID format: %s\n", cardUID.c_str());
```

### Room Management
```cpp
Serial.printf("Room card registered: %d -> %s\n", roomNumber, cardUID.c_str());
Serial.println("=== REGISTERED ROOM CARDS ===");
Serial.printf("Room %d: %s\n", room.roomNumber, room.cardUID.c_str());
```

### Entry/Exit Events
```cpp
Serial.printf("Entry event: Room %d\n", roomNumber);
Serial.printf("Exit event: Room %d\n", roomNumber);
```

### Sensor Status
```cpp
Serial.printf("RFID sensor status: %s\n", isConnected() ? "Connected" : "Disconnected");
```

### Zone Data
```cpp
Serial.printf("Zone data: %s\n", zoneData.c_str());
```

### Single Location Registration Debug
```cpp
Serial.println("[RFID] Single location registration mode activated");
```

## 6. IMU.cpp (12 statements)

### Step Data Management
```cpp
Serial.printf("IMU: Saved step data - Daily: %lu, Total: %lu\n", dailySteps, totalSteps);
Serial.printf("IMU: Loaded step data - Daily: %lu, Total: %lu\n", dailySteps, totalSteps);
```

### Daily Reset Triggers
```cpp
Serial.println("IMU: Daily reset triggered by NTP time");
Serial.println("IMU: Daily reset triggered by GPS time");
```

### GPS Time Setting
```cpp
Serial.printf("IMU: GPS time set - %02d:%02d:%02d\n", hour, minute, second);
```

### Calibration Data
```cpp
Serial.printf("IMU: Accelerometer offsets - X:%.2f Y:%.2f Z:%.2f\n", accelOffsetX, accelOffsetY, accelOffsetZ);
Serial.printf("IMU: Gyroscope offsets - X:%.2f Y:%.2f Z:%.2f\n", gyroOffsetX, gyroOffsetY, gyroOffsetZ);
```

### Step Detection
```cpp
Serial.printf("IMU: Step detected! Daily: %lu, Total: %lu\n", dailySteps, totalSteps);
```

### Real-time Data
```cpp
Serial.printf("IMU: Accel(%.2f,%.2f,%.2f) Gyro(%.2f,%.2f,%.2f) Temp:%.1f°C\n",
              accelX, accelY, accelZ, gyroX, gyroY, gyroZ, temperature);
Serial.printf("IMU: Pitch:%.1f° Roll:%.1f° Yaw:%.1f°\n", pitch, roll, yaw);
Serial.printf("IMU: Steps - Daily:%lu Total:%lu\n", dailySteps, totalSteps);
```

## 7. BLEManager.cpp (6 statements)

### Command Processing
```cpp
Serial.printf("BLE: Received command: %s\n", command.c_str());
```

### Large Data Transmission
```cpp
Serial.printf("BLE: Large data transmission - Size: %d bytes\n", dataSize);
Serial.println("BLE: Large data transmission debug enabled");
```

### Step Updates
```cpp
Serial.printf("BLE: Step update sent - Daily: %lu, Total: %lu\n", dailySteps, totalSteps);
```

### Queue Statistics
```cpp
Serial.printf("BLE: Queue stats - Queued: %d, Total: %d, Dropped: %d\n",
              queuedPackets, totalPackets, droppedPackets);
Serial.printf("BLE: Performance - Latency: %lu ms, Throughput: %.2f KB/s\n",
              latency, throughput);
```

## 8. GPSModule.cpp (2 statements)

### Serial Communication
```cpp
Serial.printf("GPS: Serial communication active on pins RX:%d TX:%d\n", GPS_RX_PIN, GPS_TX_PIN);
```

### Power Saving
```cpp
Serial.printf("GPS: Power saving mode %s\n", enabled ? "enabled" : "disabled");
```

## 9. GzipUtils.cpp (3 statements)

### File Operations
```cpp
Serial.printf("GzipUtils: Failed to create file: %s\n", outputPath);
Serial.printf("GzipUtils: Failed to write to file: %s\n", outputPath);
```

### Compression Results
```cpp
Serial.printf("GzipUtils: Compression complete - Original: %d bytes, Compressed: %d bytes (%.1f%% reduction)\n",
              originalSize, compressedSize, reductionPercent);
```

## 10. FeedbackManager.cpp (1 statement)

### Mode Changes
```cpp
Serial.printf("FeedbackManager: Mode changed to %d\n", newMode);
```

## 11. DiagnosticUI.cpp (6 statements)

### Progress Updates
```cpp
Serial.printf("Diagnostic Progress: %d%%\n", progress);
```

### Sensor Check Results
```cpp
Serial.printf("Sensor Check: %s - %s\n", sensorName, status ? "PASS" : "FAIL");
Serial.printf("Sensor %s: Connected and responding\n", sensorName);
Serial.printf("Sensor %s: Wiring issue detected\n", sensorName);
Serial.printf("Sensor %s: Calibration required\n", sensorName);
Serial.printf("Sensor %s: Unknown status\n", sensorName);
```

### Error Messages and Solutions
```cpp
Serial.printf("Error: %s\nSolution: %s\n", errorMessage, solution);
```

## 12. LightSensor.cpp (1 statement)

### Time Display
```cpp
Serial.print("Current time: ");
```

---

# Summary

**Total Serial Output Statements: 270+**

This comprehensive list includes all Serial.print(), Serial.println(), and Serial.printf() statements found across the entire Smart Cane ESP32 project. The statements cover:

- **System diagnostics and monitoring**
- **Sensor data output and debugging**
- **User interface and command responses**
- **Error handling and troubleshooting**
- **Performance monitoring and statistics**
- **Configuration and status reporting**

These statements provide extensive debugging capabilities and user feedback throughout the Smart Cane system operation.