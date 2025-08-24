# 🦯 Smart Assistive Cane V1

A comprehensive smart assistive cane system designed to help visually impaired individuals navigate both indoor and outdoor environments safely and independently.

## 🎯 Why This Project Exists

After witnessing the daily challenges faced by visually impaired individuals in our community, this project was created to enhance their mobility and independence. Traditional white canes, while effective, lack the advanced sensing capabilities needed for modern urban environments. This project bridges that gap using affordable, accessible technology.

## 🚀 What It Does

The Smart Assistive Cane transforms a traditional mobility aid into an intelligent navigation system that provides:

- **Real-time obstacle detection** with haptic and audio feedback
- **Indoor room identification** using RFID technology
- **Fall detection and emergency alerts** through motion analysis
- **Environmental awareness** including temperature, humidity, and lighting conditions
- **GPS navigation** for outdoor wayfinding
- **Step counting and activity tracking** for health monitoring
- **Smartphone connectivity** for remote monitoring and data logging

## ✨ Key Features

### 🔍 Sensing Capabilities

- **Obstacle Detection**: VL53L1X ToF sensor with 3.5m range and adaptive filtering
- **Motion Tracking**: MPU6050 IMU with Madgwick filter for precise orientation
- **Environmental Monitoring**: DHT22 for temperature/humidity, BH1750 for light levels
- **Indoor Navigation**: MFRC522 RFID reader for room identification
- **GPS Tracking**: NEO-6M module with Kalman filtering for accurate positioning

### 🎛️ User Interface

- **Dual Feedback System**: Configurable buzzer and vibration motor alerts
- **Button Controls**: Long-press detection for mode switching
- **LED Indicators**: Visual status feedback
- **Serial Commands**: 50+ diagnostic and configuration commands

### 📱 Connectivity

- **Bluetooth Low Energy**: Real-time data streaming to mobile apps
- **WiFi Integration**: Arduino IoT Cloud connectivity with retry logic
- **Offline Mode**: Graceful degradation when connectivity is unavailable

## 🔧 Hardware Components

| Component | Specification | Quantity | Purpose |
|-----------|---------------|----------|----------|
| ESP32-S3-N16R8 | 240MHz dual-core, 16MB Flash, 8MB PSRAM | 1 | Main controller |
| VL53L1X | Time-of-Flight sensor, 4m range | 1 | Obstacle detection |
| MPU6050 | 6-axis IMU, 16-bit ADC | 1 | Motion tracking |
| DHT22 | Temperature/humidity sensor | 1 | Environmental monitoring |
| BH1750 | Digital light sensor, 65535 lux range | 1 | Light level detection |
| MFRC522 | 13.56MHz RFID reader | 1 | Indoor navigation |
| NEO-6M | GPS module, 50 channels | 1 | Outdoor positioning |
| SG90 | Micro servo motor | 1 | Radar scanning mode |
| Vibration Motors | 3V DC motors | 2 | Haptic feedback |
| Piezo Buzzer | 5V active buzzer | 1 | Audio alerts |
| Push Buttons | Momentary switches | 4 | User input |
| LEDs | 3mm standard LEDs | 2 | Status indication |

## 📐 Circuit Diagram & Wiring

### Pin Configuration

```
ESP32-S3 Pin Mapping:
├── I2C Bus (Sensors)
│   ├── SDA: GPIO 8
│   └── SCL: GPIO 9
├── SPI Bus (RFID)
│   ├── MOSI: GPIO 40
│   ├── MISO: GPIO 48
│   ├── SCK: GPIO 21
│   └── CS: GPIO 10
├── UART (GPS)
│   ├── RX: GPIO 17
│   └── TX: GPIO 18
├── Feedback System
│   ├── Buzzer: GPIO 16
│   ├── Vibration 1: GPIO 38
│   └── Vibration 2: GPIO 39
├── User Interface
│   ├── Button 1: GPIO 1
│   ├── Button 2: GPIO 2
│   ├── Button 3: GPIO 42
│   └── Button 4: GPIO 41
└── Status LEDs
    ├── LED 1: GPIO 43
    └── LED 2: GPIO 44
```

Detailed circuit diagrams and PCB layouts are available in the `/Arduino IDE ESP32/docs` folder.

## 💻 Software Architecture

The software uses a modular architecture for maintainability and scalability:

```
Arduino IDE ESP32/SmartCaneESP32N16R8/
├── SmartCaneESP32N16R8.ino    # Main application loop
├── SensorData.h               # Unified data structure
├── Pins.h                     # Hardware pin definitions
├── Sensor Modules/
│   ├── EnvMonitor.cpp/.h      # Temperature & humidity
│   ├── LightSensor.cpp/.h     # Ambient light detection
│   ├── IMU.cpp/.h             # Motion & fall detection
│   ├── ToF.cpp/.h             # Obstacle detection
│   ├── RFID.cpp/.h            # Indoor navigation
│   └── GPSModule.cpp/.h       # Outdoor positioning
├── Communication/
│   ├── BLEManager.cpp/.h      # Bluetooth connectivity
│   └── ConnectivityManager.cpp/.h # WiFi & cloud
├── User Interface/
│   └── FeedbackManager.cpp/.h # Haptic & audio feedback
└── Configuration/
    ├── arduino_secrets.h      # IoT Cloud credentials
    ├── secrets.cpp/.h         # WiFi configuration
    └── thingProperties.h      # Cloud variable definitions
```

### Core Libraries Used

- **VL53L1X**: Time-of-Flight sensor communication
- **MPU6050**: IMU data processing
- **MadgwickAHRS**: Sensor fusion algorithm
- **DHT**: Temperature/humidity sensor
- **BH1750**: Light sensor interface
- **MFRC522**: RFID card reading
- **TinyGPS++**: GPS data parsing
- **ArduinoIoTCloud**: Cloud connectivity
- **BLEDevice**: Bluetooth Low Energy

## 🛠️ Installation & Setup

### Prerequisites

- Arduino IDE 2.0 or later
- ESP32 board package installed
- Required libraries (see `libraries.txt`)

### Hardware Setup

1. **Assemble the circuit** following the pin configuration above
2. **Connect power supply** (5V, 2A recommended)
3. **Mount sensors** on the cane structure
4. **Test all connections** before powering on

### Software Setup

1. **Clone this repository**:
   ```bash
   git clone https://github.com/HasnainMemon04/Smart-Assistive-Cane-V1.git
   cd Smart-Assistive-Cane-V1
   ```

2. **Install required libraries**:
   - Open Arduino IDE
   - Go to Tools → Manage Libraries
   - Install libraries listed in `Arduino IDE ESP32/libraries.txt`

3. **Configure credentials**:
   - Copy `arduino_secrets_template.h` to `arduino_secrets.h`
   - Fill in your WiFi and IoT Cloud credentials

4. **Upload the code**:
   - Open `Arduino IDE ESP32/SmartCaneESP32N16R8/SmartCaneESP32N16R8.ino`
   - Select ESP32-S3 board
   - Upload to your device

## 📱 Mobile App

The companion Flutter mobile app provides:

- Real-time sensor data visualization
- GPS tracking and route history
- Emergency contact notifications
- Device configuration and settings
- Health metrics and activity tracking

### App Setup

1. **Navigate to APP directory**:
   ```bash
   cd APP
   ```

2. **Install dependencies**:
   ```bash
   flutter pub get
   ```

3. **Run the app**:
   ```bash
   flutter run
   ```

## 📊 Performance Metrics

- **Obstacle Detection Range**: 0.04m - 4.0m
- **Detection Accuracy**: 95%+ in optimal conditions
- **Battery Life**: 8-12 hours continuous use
- **GPS Accuracy**: ±3-5 meters
- **Response Time**: <100ms for critical alerts
- **BLE Range**: Up to 10 meters

## 🔒 Security Features

- Encrypted BLE communication
- Secure cloud data transmission
- Local data backup and recovery
- Privacy-focused data handling

## 🤝 Contributing

We welcome contributions! Please see [CONTRIBUTING.md](CONTRIBUTING.md) for guidelines.

## 📄 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## 🙏 Acknowledgments

- Visually impaired community for feedback and testing
- Open-source hardware and software communities
- Contributors and supporters of this project

## 📞 Support

For questions, issues, or suggestions:

- Create an [issue](https://github.com/HasnainMemon04/Smart-Assistive-Cane-V1/issues)
- Contact: [your-email@example.com]

---

**Made with ❤️ for accessibility and independence**