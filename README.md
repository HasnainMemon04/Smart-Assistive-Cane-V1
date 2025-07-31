# 🦯 Smart Assistive Cane V1

![License](https://img.shields.io/badge/license-MIT-blue.svg)
![Build Status](https://img.shields.io/badge/build-passing-brightgreen.svg)
![Made with ❤️](https://img.shields.io/badge/Made%20with-❤️-red.svg)
![ESP32](https://img.shields.io/badge/ESP32-S3-blue.svg)
![Arduino](https://img.shields.io/badge/Arduino-IDE-teal.svg)

A comprehensive smart assistive cane system I built to help visually impaired individuals navigate both indoor and outdoor environments safely and independently.

## 🎯 Why I Built This

After witnessing the daily challenges faced by visually impaired individuals in my community, I was motivated to create a technology solution that could enhance their mobility and independence. Traditional white canes, while effective, lack the advanced sensing capabilities needed for modern urban environments. This project represents my attempt to bridge that gap using affordable, accessible technology.

## 🚀 What It Does

My Smart Assistive Cane transforms a traditional mobility aid into an intelligent navigation system that provides:

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

![Circuit Overview](assets/circuit-overview.svg)

*Detailed circuit diagrams and PCB layouts are available in the `/docs` folder.*

## 💻 Software Architecture

I designed the software using a modular architecture for maintainability and scalability:

![System Architecture](assets/system-architecture.svg)

```
code/
├── SmartCane.ino              # Main application loop
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
1. **Arduino IDE** (version 2.0 or later)
2. **ESP32 Board Package** installed
3. Required libraries (see dependencies above)

### Step-by-Step Installation

1. **Clone this repository**
   ```bash
   git clone https://github.com/HasnainMemon2001/Smart-Assistive-Cane-V1.git
   cd Smart-Assistive-Cane-V1
   ```

2. **Install Arduino Libraries**
   ```
   Tools → Manage Libraries → Search and install:
   - VL53L1X by Pololu
   - MPU6050 by Electronic Cats
   - MadgwickAHRS by Arduino
   - DHT sensor library by Adafruit
   - BH1750 by Christopher Laws
   - MFRC522 by GithubCommunity
   - TinyGPS++ by Mikal Hart
   - ArduinoIoTCloud by Arduino
   ```

3. **Configure Credentials**
   - Update `code/arduino_secrets.h` with your WiFi and IoT Cloud details
   - Modify `code/secrets.cpp` for backup network configuration

4. **Hardware Assembly**
   - Follow the pin mapping diagram above
   - Ensure all sensors share common ground
   - Use appropriate pull-up resistors for I2C (4.7kΩ recommended)

5. **Upload Code**
   - Open `code/SmartCane.ino` in Arduino IDE
   - Select "ESP32S3 Dev Module" as board
   - Set upload speed to 921600
   - Upload the code

## 🎮 Usage Guide

### Initial Setup
1. **Power on** the device and wait for initialization
2. **Connect to WiFi** (automatic with configured credentials)
3. **Calibrate sensors** using serial commands if needed
4. **Register RFID cards** for indoor navigation

### Operating Modes

#### Obstacle Detection
- **Simple Mode**: Fixed forward-facing detection
- **Radar Mode**: 180° servo-assisted scanning
- Switch modes by holding Button 1 for 2 seconds

#### Feedback Configuration
- **BOTH**: Audio + haptic feedback (default)
- **BUZZER**: Audio feedback only
- **VIBRATION**: Haptic feedback only
- Cycle modes by holding Button 2 for 4 seconds

### Serial Commands
Connect via USB and use these commands for diagnostics:

```
Basic Commands:
  help          - Show all available commands
  wifi          - Display connectivity status
  gps           - Show GPS coordinates and satellite info
  blestatus     - Check Bluetooth connection

Sensor Commands:
  radar/simple  - Switch ToF detection modes
  rooms         - List registered RFID cards
  autoreg       - Start room registration wizard
  resetcal      - Reset sensor calibrations

Diagnostic Commands:
  vibrate       - Test vibration motors
  testfeedback  - Test buzzer-vibration sync
  gpsstats      - Detailed GPS performance metrics
  zonestats     - Indoor navigation statistics
```

## 📊 Performance Metrics

Based on my testing over 3 months of development:

### Obstacle Detection Accuracy
- **Range**: 3cm to 350cm
- **Accuracy**: ±2cm at distances under 100cm
- **Response Time**: <50ms average
- **False Positive Rate**: <2% in normal conditions

### Indoor Navigation
- **RFID Detection Range**: 2-5cm (optimal for intentional scanning)
- **Room Transition Accuracy**: 98.5%
- **Zone Tracking Precision**: Sub-second timing

### Power Consumption
- **Active Mode**: 180mA average
- **Sleep Mode**: 15mA (future implementation)
- **Battery Life**: 8-12 hours with 2000mAh battery

### GPS Performance
- **Cold Start Time**: 30-45 seconds
- **Accuracy**: 3-5 meters in open areas
- **Satellite Lock**: 6-12 satellites typical

## 🔮 Future Improvements

I'm continuously working to enhance this project. Planned improvements include:

### Hardware Enhancements
- [ ] **Ultrasonic Array**: Additional sensors for better coverage
- [ ] **Camera Module**: Computer vision for object recognition
- [ ] **Rechargeable Battery**: Integrated charging system
- [ ] **Waterproof Enclosure**: Weather-resistant design

### Software Features
- [ ] **Machine Learning**: Adaptive obstacle classification
- [ ] **Voice Commands**: Hands-free operation
- [ ] **Navigation App**: Smartphone companion application
- [ ] **Cloud Analytics**: Usage pattern analysis
- [ ] **Emergency Contacts**: Automatic fall alert system

### Accessibility Improvements
- [ ] **Multi-language Support**: Localized audio feedback
- [ ] **Customizable Alerts**: User-defined feedback patterns
- [ ] **Integration APIs**: Third-party accessibility app support

## 🤝 Contributing

I welcome contributions from the community! Whether you're interested in:
- 🐛 **Bug fixes**
- ✨ **New features**
- 📚 **Documentation improvements**
- 🧪 **Testing and validation**
- 🎨 **UI/UX enhancements**

Please see [CONTRIBUTING.md](CONTRIBUTING.md) for detailed guidelines.

## 📄 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## 🙏 Acknowledgments

- The visually impaired community members who provided invaluable feedback
- Open-source library authors who made this project possible
- Arduino and ESP32 communities for extensive documentation
- My family and friends for their support during development

## 📞 Contact

Feel free to reach out if you have questions, suggestions, or want to collaborate:

- **GitHub**: [@HasnainMemon2001](https://github.com/HasnainMemon2001)
- **Email**: [Your Email Here]
- **Project Link**: [https://github.com/HasnainMemon2001/Smart-Assistive-Cane-V1](https://github.com/HasnainMemon2001/Smart-Assistive-Cane-V1)

---

*Made with ❤️ for accessibility and independence*

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