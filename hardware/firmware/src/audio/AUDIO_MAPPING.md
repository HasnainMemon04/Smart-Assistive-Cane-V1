# Audio File Mapping Reference

This document provides the complete mapping between serial print statements and their corresponding audio files for the Smart Cane audio feedback system.

## Serial Statement to Audio File Mappings

### System Connectivity (serial_001.wav - serial_012.wav)

| Serial Number | Audio File | Serial Statement | Description |
|---------------|------------|------------------|-------------|
| 001 | serial_001.wav | "WiFi connected" | WiFi connection established |
| 002 | serial_002.wav | "WiFi connection failed" | WiFi connection attempt failed |
| 003 | serial_003.wav | "WiFi status" | WiFi status inquiry |
| 004 | serial_004.wav | "Connected to [SSID]" | WiFi network connection confirmation |
| 005 | serial_005.wav | "Signal strength: [value] dBm" | WiFi signal strength reading |
| 006 | serial_006.wav | "WiFi functionality disabled" | WiFi feature disabled |
| 007 | serial_007.wav | "Switching to offline mode" | Offline mode activation |
| 008 | serial_008.wav | "Cloud functionality disabled" | Cloud features disabled |
| 009 | serial_009.wav | "Cloud connection status" | Cloud connectivity status |
| 011 | serial_011.wav | "Status: Offline mode" | Current offline status |
| 012 | serial_012.wav | "Resetting cloud connection" | Cloud connection reset |

### Light Sensor Calibration (serial_014.wav - serial_020.wav)

| Serial Number | Audio File | Serial Statement | Description |
|---------------|------------|------------------|-------------|
| 014 | serial_014.wav | "Light sensor calibration reset" | Calibration data cleared |
| 016 | serial_016.wav | "AddLocation command received" | Location addition command |
| 017 | serial_017.wav | "Light sensor calibration" | Calibration process |
| 018 | serial_018.wav | "Calibration complete" | Calibration finished successfully |
| 019 | serial_019.wav | "Calibration in progress" | Calibration currently running |
| 020 | serial_020.wav | "Calibration failed" | Calibration process failed |

### Vibration Motor Testing (serial_021.wav - serial_024.wav)

| Serial Number | Audio File | Serial Statement | Description |
|---------------|------------|------------------|-------------|
| 021 | serial_021.wav | "Testing vibration motors" | Motor test initiation |
| 022 | serial_022.wav | "Vibration test complete" | Motor test finished |
| 023 | serial_023.wav | "Vibration motor 1 test" | First motor test |
| 024 | serial_024.wav | "Vibration motor 2 test" | Second motor test |

### BLE Status (serial_025.wav - serial_029.wav)

| Serial Number | Audio File | Serial Statement | Description |
|---------------|------------|------------------|-------------|
| 025 | serial_025.wav | "BLE connected" | Bluetooth connection established |
| 026 | serial_026.wav | "BLE disconnected" | Bluetooth connection lost |
| 027 | serial_027.wav | "BLE status" | Bluetooth status inquiry |
| 028 | serial_028.wav | "BLE device name: [name]" | Device name announcement |
| 029 | serial_029.wav | "BLE MAC address: [address]" | MAC address announcement |

### ToF Sensor Modes (serial_030.wav - serial_033.wav)

| Serial Number | Audio File | Serial Statement | Description |
|---------------|------------|------------------|-------------|
| 030 | serial_030.wav | "ToF mode" | Time-of-Flight mode status |
| 031 | serial_031.wav | "Radar mode" | Radar scanning mode |
| 032 | serial_032.wav | "Simple mode" | Simple distance mode |
| 033 | serial_033.wav | "Distance reading: [value] cm" | Distance measurement |

### GPS Functionality (serial_034.wav - serial_038.wav)

| Serial Number | Audio File | Serial Statement | Description |
|---------------|------------|------------------|-------------|
| 034 | serial_034.wav | "GPS status" | GPS system status |
| 035 | serial_035.wav | "GPS connected" | GPS signal acquired |
| 036 | serial_036.wav | "GPS disconnected" | GPS signal lost |
| 037 | serial_037.wav | "[number] satellites" | Satellite count |
| 038 | serial_038.wav | "GPS coordinates: [lat], [lon]" | Position coordinates |

### Sensor Reset (serial_039.wav - serial_042.wav)

| Serial Number | Audio File | Serial Statement | Description |
|---------------|------------|------------------|-------------|
| 039 | serial_039.wav | "Sensor reset" | General sensor reset |
| 040 | serial_040.wav | "All sensors reset" | Complete system reset |
| 041 | serial_041.wav | "IMU reset" | Inertial measurement unit reset |
| 042 | serial_042.wav | "ToF reset" | Time-of-Flight sensor reset |

### Feedback Modes (serial_043.wav - serial_046.wav)

| Serial Number | Audio File | Serial Statement | Description |
|---------------|------------|------------------|-------------|
| 043 | serial_043.wav | "Feedback mode" | Current feedback mode |
| 044 | serial_044.wav | "Buzzer mode" | Audio buzzer feedback |
| 045 | serial_045.wav | "Vibration mode" | Haptic vibration feedback |
| 046 | serial_046.wav | "Both mode" | Combined audio and haptic |

### System Diagnostics (serial_202.wav - serial_206.wav)

| Serial Number | Audio File | Serial Statement | Description |
|---------------|------------|------------------|-------------|
| 202 | serial_202.wav | "Running full system diagnostic" | Comprehensive system check |
| 203 | serial_203.wav | "Diagnostic complete" | Diagnostic process finished |
| 204 | serial_204.wav | "Checking sensor connectivity" | Sensor connection test |
| 205 | serial_205.wav | "All sensors connected successfully" | All sensors operational |
| 206 | serial_206.wav | "Some sensors failed connectivity check" | Sensor connection issues |

### System Status Display (serial_207.wav - serial_221.wav)

| Serial Number | Audio File | Serial Statement | Description |
|---------------|------------|------------------|-------------|
| 207 | serial_207.wav | "Smart Cane System Status" | System status header |
| 208 | serial_208.wav | "Smart Assistive Cane v1.0" | System version |
| 209 | serial_209.wav | "Uptime: [time]" | System runtime |
| 210 | serial_210.wav | "Free heap: [bytes] bytes" | Available memory |
| 211 | serial_211.wav | "BLE: Connected" | Bluetooth status |
| 212 | serial_212.wav | "Audio System: [status]" | Audio system status |
| 213 | serial_213.wav | "Feedback Mode: [mode]" | Current feedback mode |
| 214 | serial_214.wav | "ToF Mode: [mode]" | Distance sensor mode |
| 215 | serial_215.wav | "Current Room: [room]" | Location information |
| 216 | serial_216.wav | "Daily Steps: [count]" | Step counter reading |
| 217 | serial_217.wav | "Temperature: [value]°C" | Temperature reading |
| 218 | serial_218.wav | "Humidity: [value]%" | Humidity reading |
| 219 | serial_219.wav | "Light: [value] lux" | Light level reading |
| 220 | serial_220.wav | "Distance: [value] cm" | Distance measurement |
| 221 | serial_221.wav | "GPS: [status]" | GPS status |

### Available Commands (serial_222.wav - serial_240.wav)

| Serial Number | Audio File | Serial Statement | Description |
|---------------|------------|------------------|-------------|
| 222 | serial_222.wav | "Available commands:" | Command list header |
| 223 | serial_223.wav | "wifi - WiFi management" | WiFi command description |
| 224 | serial_224.wav | "reconnect - Reconnect WiFi" | Reconnection command |
| 225 | serial_225.wav | "resetwifi - Reset WiFi settings" | WiFi reset command |
| 226 | serial_226.wav | "offline - Switch to offline mode" | Offline mode command |
| 227 | serial_227.wav | "resetcal - Reset calibration" | Calibration reset |
| 228 | serial_228.wav | "vibrate - Test vibration" | Vibration test command |
| 229 | serial_229.wav | "rooms - List saved rooms" | Room management |
| 230 | serial_230.wav | "clearrooms - Clear all rooms" | Room data clearing |
| 231 | serial_231.wav | "autoreg - Auto room registration" | Automatic room detection |
| 232 | serial_232.wav | "addlocation - Add new location" | Location addition |
| 233 | serial_233.wav | "addroom - Add new room" | Room addition |
| 234 | serial_234.wav | "Button controls:" | Button control header |
| 235 | serial_235.wav | "Hold BTN1 - [function]" | Button 1 function |
| 236 | serial_236.wav | "Hold BTN2 - [function]" | Button 2 function |
| 237 | serial_237.wav | "Hold BTN3 - [function]" | Button 3 function |
| 238 | serial_238.wav | "BTN3 short press - [function]" | Button 3 short press |
| 239 | serial_239.wav | "BTN3 hold 3s - [function]" | Button 3 medium hold |
| 240 | serial_240.wav | "BTN3 hold 10s - [function]" | Button 3 long hold |

### ToF Sensor Operations (serial_243.wav - serial_249.wav)

| Serial Number | Audio File | Serial Statement | Description |
|---------------|------------|------------------|-------------|
| 243 | serial_243.wav | "ToF sensor reset" | Sensor reset initiation |
| 244 | serial_244.wav | "ToF sensor reset complete" | Reset process finished |
| 245 | serial_245.wav | "Failed to initialize sensor" | Sensor initialization error |
| 246 | serial_246.wav | "VL53L1X Smart Cane System initialized" | Sensor ready |
| 248 | serial_248.wav | "Modes: Simple / Radar" | Available sensor modes |
| 249 | serial_249.wav | "ToF sensor appears stuck" | Sensor malfunction warning |

### Radar Mode Operations (serial_269.wav - serial_274.wav)

| Serial Number | Audio File | Serial Statement | Description |
|---------------|------------|------------------|-------------|
| 269 | serial_269.wav | "Switching to radar mode" | Radar mode activation |
| 270 | serial_270.wav | "Switching to simple mode" | Simple mode activation |
| 271 | serial_271.wav | "Real-time radar task started" | Radar scanning initiated |
| 272 | serial_272.wav | "Radar sweep completed" | Scan cycle finished |
| 273 | serial_273.wav | "Real-time radar scanning started" | Continuous scanning |
| 274 | serial_274.wav | "Radar scan results: [data]" | Scan results announcement |

## Digit and Unit Files

### Number Files (/audio/digits/)

| Filename | Content | Usage |
|----------|---------|-------|
| num0.wav | "zero" | Number 0 |
| num1.wav | "one" | Number 1 |
| num2.wav | "two" | Number 2 |
| num3.wav | "three" | Number 3 |
| num4.wav | "four" | Number 4 |
| num5.wav | "five" | Number 5 |
| num6.wav | "six" | Number 6 |
| num7.wav | "seven" | Number 7 |
| num8.wav | "eight" | Number 8 |
| num9.wav | "nine" | Number 9 |
| num10.wav | "ten" | Number 10 |
| num20.wav | "twenty" | Number 20 |
| num30.wav | "thirty" | Number 30 |
| num40.wav | "forty" | Number 40 |
| num50.wav | "fifty" | Number 50 |
| num60.wav | "sixty" | Number 60 |
| num70.wav | "seventy" | Number 70 |
| num80.wav | "eighty" | Number 80 |
| num90.wav | "ninety" | Number 90 |
| hundred.wav | "hundred" | Hundred multiplier |
| point.wav | "point" | Decimal point |

### Unit Files (/audio/digits/)

| Filename | Content | Usage |
|----------|---------|-------|
| centimeters.wav | "centimeters" | Distance unit |
| meters.wav | "meters" | Distance unit |
| celsius.wav | "degrees celsius" | Temperature unit |
| fahrenheit.wav | "degrees fahrenheit" | Temperature unit |
| lux.wav | "lux" | Light intensity unit |
| percent.wav | "percent" | Percentage unit |
| volts.wav | "volts" | Voltage unit |
| milliamps.wav | "milliamps" | Current unit |
| seconds.wav | "seconds" | Time unit |
| minutes.wav | "minutes" | Time unit |
| hours.wav | "hours" | Time unit |

## Critical Alert Files (/audio/critical/)

| Filename | Content | Usage |
|----------|---------|-------|
| system_ready.wav | "System ready" | Startup completion |
| obstacle_ahead.wav | "Obstacle ahead" | Immediate danger warning |
| distance_sensor_failed.wav | "Distance sensor failed" | Hardware malfunction |
| low_battery.wav | "Low battery" | Power warning |
| emergency_alert.wav | "Emergency alert" | Critical situation |
| gps_no_signal.wav | "GPS signal lost" | Navigation warning |

## Environmental Files (/audio/environmental/)

| Filename | Content | Usage |
|----------|---------|-------|
| dark_environment.wav | "Dark environment" | Low light condition |
| dim_light.wav | "Dim lighting" | Moderate light |
| bright_light.wav | "Bright lighting" | High light condition |
| wet_surface.wav | "Wet surface detected" | Surface condition |
| uneven_ground.wav | "Uneven ground" | Terrain warning |

## Navigation Files (/audio/navigation/)

| Filename | Content | Usage |
|----------|---------|-------|
| turn_left.wav | "Turn left" | Direction instruction |
| turn_right.wav | "Turn right" | Direction instruction |
| go_straight.wav | "Go straight" | Direction instruction |
| stairs_ahead.wav | "Stairs ahead" | Obstacle warning |
| destination_reached.wav | "Destination reached" | Navigation complete |
| satellites_connected.wav | "Satellites connected" | GPS status |

## Usage Examples

### Serial Statement Mapping
```cpp
// When this serial statement is printed:
Serial.println("WiFi connected");

// The audio system will play:
audioManager.announceSerialStatement("WiFi connected");
// Which maps to: /audio/serial/serial_001.wav
```

### Number Announcement
```cpp
// Announce distance reading of 45.2 cm
audioManager.announceDistanceReading(45.2);
// Plays: "forty" + "five" + "point" + "two" + "centimeters"
```

### Direct Audio File Playback
```cpp
// Play specific audio file
audioManager.playAudioFile("/audio/critical/obstacle_ahead.wav");
```

---

**Note**: This mapping system provides comprehensive audio feedback for all Smart Cane operations, ensuring accessibility and clear communication of system status and environmental information to users.