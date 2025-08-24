# Smart Assistive Cane - ESP32-S3

A comprehensive smart cane system designed for visually impaired users, featuring obstacle detection, indoor navigation, fall detection, and environmental monitoring.

## 🚀 Features

### Core Functionality
- **Obstacle Detection**: VL53L1X Time-of-Flight sensor with adaptive filtering
- **Indoor Zoning**: RFID-based room identification and navigation
- **Fall Detection**: IMU-based motion analysis and alert system
- **Step Counting**: Daily and total step tracking with automatic reset
- **Environmental Monitoring**: Temperature, humidity, and light level sensing
- **GPS Navigation**: Outdoor location tracking and navigation

### Connectivity
- **WiFi Integration**: Background connection with retry logic
- **Bluetooth Low Energy**: Mobile app communication
- **Arduino IoT Cloud**: Remote monitoring and data logging
- **Time Synchronization**: NTP and GPS time sync

### User Interface
- **Haptic Feedback**: Vibration motors for tactile alerts
- **Audio Feedback**: Piezo buzzer with configurable patterns
- **Audio/Haptic Feedback**: Buzzer and vibration for user feedback
- **Button Controls**: Mode switching and calibration

## 📋 Hardware Requirements

### Main Controller
- ESP32-S3-N16R8 DevKit

### Sensors
- **DHT22**: Temperature and humidity
- **BH1750**: Light sensor with environment classification
- **MPU6050**: IMU for motion detection and fall detection
- **VL53L1X**: Time-of-Flight distance sensor
- **MFRC522**: RFID reader for indoor zoning
- **NEO-6M**: GPS module for outdoor navigation

### Actuators
- **Vibration Motors**: Haptic feedback
- **Piezo Buzzer**: Audio alerts
- **Servo Motor**: Radar mode scanning (ToF)

### Interface
- **Push Buttons**: Mode switching and calibration
- **Audio/Haptic Feedback**: Buzzer and vibration alerts

## 🔧 Installation

1. **Clone the repository**
   ```bash
   git clone https://github.com/HasnainMemon2001/Smart-Assistive-Cane-V1.git
   cd Smart-Assistive-Cane-V1
   ```

2. **Install required libraries**
   - DHT sensor library
   - BH1750 library
   - MPU6050 library
   - VL53L1X library
   - MFRC522 library
   - TinyGPS++ library
   - Arduino IoT Cloud library

3. **Configure WiFi credentials**
   - Update `arduino_secrets.h` with your WiFi details
   - Update `secrets.cpp` with backup network credentials

4. **Upload to ESP32-S3**
   - Open `SmartCane.ino` in Arduino IDE
   - Select ESP32-S3 board
   - Upload the code

## 🎛️ Usage

### Serial Commands
- `help` - Show all available commands
- `wifi` - Show connectivity status
- `rooms` - Show registered room cards
- `radar` - Switch ToF to radar mode
- `simple` - Switch ToF to simple mode
- `gps` - Show GPS status
- `blestatus` - Show BLE connection status

### Button Controls
- **BTN1 (hold 2+ seconds)**: Activate radar mode (while held, reverts to simple when released)
- **BTN2 (4s hold)**: Cycle feedback modes (BOTH → BUZZER → VIBRATION)
- **BTN3 (2s hold)**: Switch ToF modes (SIMPLE ↔ RADAR)
- **BTN3 (short press)**: IMU calibration
- **BTN3 (3s hold)**: Reset daily steps
- **BTN3 (10s hold)**: Factory reset

### Feedback Modes
- **BOTH**: Audio and haptic feedback
- **BUZZER**: Audio feedback only
- **VIBRATION**: Haptic feedback only

## 🏠 Indoor Zoning System

### Room Registration
1. Use `autoreg` to start guided room setup
2. Scan RFID cards for each room
3. Enter room names when prompted
4. Use `done` to complete setup

### Room Navigation
- Scan room card to enter that room
- Scan same card again to return to lobby
- System tracks zone statistics and time spent

## 📡 Sensor Modules

### Environment Monitor
- Temperature and humidity monitoring
- Heat index calculation
- Dew point analysis
- Indoor/outdoor assessment

### Light Sensor
- Environment classification (Dark, Normal, Bright, etc.)
- Automatic calibration
- Special condition detection (flashlight, direct sunlight)

### IMU (Motion Detection)
- Step counting with daily reset
- Fall detection with alert system
- Slope warning for steep terrain
- Motion energy analysis

### ToF (Obstacle Detection)
- Adaptive filtering for stable readings
- Radar mode with servo scanning
- Distance-based alert levels
- Error detection and recovery

### RFID (Indoor Navigation)
- Room identification and tracking
- Zone statistics and analytics
- Persistent storage of room data
- Auto-registration system

### GPS (Outdoor Navigation)
- Location tracking and coordinates
- Speed and altitude monitoring
- Satellite information and fix quality
- Time synchronization

## ☁️ Cloud Integration

### Arduino IoT Cloud
- Real-time sensor data streaming
- Remote monitoring capabilities
- Automatic retry logic for reliability
- Offline mode support

### BLE Mobile App Support
- Real-time sensor data transmission
- Zone and room information
- Step count and activity tracking

## 🔧 Configuration

### Pin Mapping
All pin definitions are in `Pins.h`:
- I2C: SDA=8, SCL=9
- SPI: MOSI=40, MISO=48, SCK=21
- UART: GPS_RX=17, GPS_TX=18
- Feedback: VIB1=38, VIB2=39, BUZZER=16

### WiFi Configuration
Update credentials in:
- `arduino_secrets.h` for Arduino IoT Cloud
- `secrets.cpp` for ConnectivityManager

## 📊 Data Structure

The `SensorData` struct contains all sensor readings:
- Environmental data (temperature, humidity, light)
- Motion data (pitch, roll, yaw, steps)
- Location data (GPS coordinates, room info)
- System status (feedback mode, connectivity)

## 🛠️ Development

### Project Structure
```
SmartCane/
├── SmartCane.ino          # Main application
├── Pins.h                 # Pin definitions
├── SensorData.h           # Data structures
├── ConnectivityManager/   # WiFi and time sync
├── EnvMonitor/           # Temperature/humidity
├── LightSensor/          # Light sensing
├── IMU/                  # Motion detection
├── ToF/                  # Obstacle detection
├── RFID/                 # Indoor zoning
├── GPSModule/            # GPS navigation
├── FeedbackManager/       # Haptic/audio feedback
├── BLEManager/           # Bluetooth communication
└── thingProperties.h     # Arduino IoT Cloud
```

### Adding New Features
1. Create new module files (.cpp/.h)
2. Add initialization in `setup()`
3. Add update call in `loop()`
4. Update `SensorData` struct if needed
5. Add serial commands in `processSerialCommand()`

## 📝 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## 🤝 Contributing

1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Test thoroughly
5. Submit a pull request

## 📞 Support

For questions or issues, please open an issue on GitHub or contact the development team.

---

**Smart Assistive Cane V1** - Empowering independence through intelligent assistance.