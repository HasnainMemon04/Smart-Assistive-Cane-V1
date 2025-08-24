# Smart Assistive Cane V1

> An intelligent assistive technology system combining ESP32-S3 hardware with Flutter mobile app for enhanced mobility and safety for visually impaired individuals.

## Table of Contents

- [Overview](#overview)
- [Features](#features)
- [Hardware Overview](#hardware-overview)
- [Pin Mapping](#pin-mapping)
- [Bill of Materials](#bill-of-materials)
- [Circuit & Wiring](#circuit--wiring)
- [Software Architecture](#software-architecture)
- [Setup & Installation](#setup--installation)
- [Usage & Operation Modes](#usage--operation-modes)
- [Serial Commands](#serial-commands)
- [Mobile App](#mobile-app)
- [Troubleshooting](#troubleshooting)
- [Development](#development)
- [Roadmap](#roadmap)
- [Contributing](#contributing)
- [License](#license)

## Overview

The Smart Assistive Cane V1 is a comprehensive assistive technology solution designed to enhance mobility, safety, and independence for visually impaired individuals. The system combines advanced sensor technology with intelligent software to provide real-time environmental awareness, navigation assistance, and safety monitoring.

### Key Capabilities
- **Obstacle Detection**: Real-time distance measurement with audio/haptic feedback
- **Fall Detection**: Advanced IMU-based fall detection with emergency alerts
- **Indoor Navigation**: RFID-based room identification and guidance
- **GPS Tracking**: Outdoor positioning and location sharing
- **Environmental Monitoring**: Temperature, humidity, and light level awareness
- **Health Monitoring**: Step counting and activity tracking
- **Smart Connectivity**: BLE integration with mobile app for monitoring and control

## Features

### Core Safety Features
- ✅ **Obstacle Detection** - VL53L1X ToF sensor with 4m range
- ✅ **Fall Detection** - MPU6050 IMU with machine learning algorithms
- ✅ **Emergency Alerts** - Automatic caregiver notifications
- ✅ **Audio Feedback** - MAX98357A amplifier with voice guidance
- ✅ **Haptic Feedback** - Dual vibration motors for tactile alerts

### Navigation & Positioning
- ✅ **Indoor Navigation** - MFRC522 RFID room identification
- ✅ **GPS Tracking** - NEO-6M module for outdoor positioning
- ✅ **Step Counting** - Accurate pedometer functionality
- ✅ **Route Recording** - Track and share walking routes

### Environmental Awareness
- ✅ **Temperature/Humidity** - DHT22 environmental monitoring
- ✅ **Light Detection** - BH1750 ambient light sensor
- ✅ **Weather Integration** - Real-time weather data

### Smart Features
- ✅ **Mobile App Integration** - Flutter-based companion app
- ✅ **Real-time Monitoring** - Live sensor data streaming
- ✅ **Data Logging** - SD card storage for offline data
- ✅ **Caregiver Dashboard** - Remote monitoring capabilities
- ✅ **Voice Commands** - Audio-based system control

## Hardware Overview

### Main Controller
- **ESP32-S3-N16R8 DevKit**: Dual-core processor with 16MB Flash, 8MB PSRAM
- **Operating Voltage**: 3.3V
- **Power Consumption**: 150-300mA (depending on active sensors)
- **Connectivity**: WiFi, Bluetooth 5.0, BLE

### Sensor Suite
| Sensor | Model | Purpose | Interface | Range/Accuracy |
|--------|-------|---------|-----------|----------------|
| ToF Distance | VL53L1X | Obstacle detection | I2C | 4cm - 4m |
| IMU | MPU6050 | Motion/fall detection | I2C | ±2g/±250°/s |
| Environment | DHT22 | Temperature/humidity | Digital | ±0.5°C/±2%RH |
| Light | BH1750 | Ambient light | I2C | 1-65535 lux |
| RFID | MFRC522 | Indoor navigation | SPI | 13.56MHz |
| GPS | NEO-6M | Outdoor positioning | UART | 2.5m accuracy |

### Feedback Systems
| Component | Model | Purpose | Interface | Specifications |
|-----------|-------|---------|-----------|----------------|
| Audio Amp | MAX98357A | Voice feedback | I2S | 3.2W Class D |
| Vibration | Motors x2 | Haptic feedback | Digital | 3V, 80mA |
| Buzzer | Piezo | Audio alerts | PWM | 2-4kHz |
| Servo | SG90 | Radar scanning | PWM | 180° rotation |

### Storage & Interface
- **SD Card**: Data logging and audio file storage
- **Push Buttons**: User interface (2x tactile switches)
- **Status LEDs**: System status indicators
- **USB-C**: Programming and power

## Pin Mapping

### ESP32-S3-N16R8 Pin Assignments

| Function | Pin | Notes |
|----------|-----|-------|
| **I2C Bus** | | |
| SDA | GPIO8 | ToF, IMU, Light sensors |
| SCL | GPIO9 | I2C clock line |
| **SPI - RFID** | | |
| MISO | GPIO13 | MFRC522 data out |
| MOSI | GPIO11 | MFRC522 data in |
| SCK | GPIO12 | MFRC522 clock |
| SS | GPIO10 | MFRC522 chip select |
| RST | GPIO14 | MFRC522 reset |
| **UART - GPS** | | |
| RX | GPIO18 | GPS module TX |
| TX | GPIO17 | GPS module RX |
| **Audio System** | | |
| I2S_BCLK | GPIO4 | MAX98357A bit clock |
| I2S_LRC | GPIO5 | MAX98357A word select |
| I2S_DIN | GPIO6 | MAX98357A data input |
| **Feedback** | | |
| Vibration_1 | GPIO15 | Primary vibration motor |
| Vibration_2 | GPIO16 | Secondary vibration motor |
| Buzzer | GPIO7 | Piezo buzzer PWM |
| IMU_Feedback | GPIO21 | IMU interrupt pin |
| **User Interface** | | |
| Button_1 | GPIO1 | Primary user button |
| Button_2 | GPIO2 | Secondary user button |
| **Servo Motor** | | |
| Servo_PWM | GPIO48 | Radar mode servo control |
| **SD Card SPI** | | |
| SD_MISO | GPIO37 | SD card data out |
| SD_MOSI | GPIO35 | SD card data in |
| SD_SCK | GPIO36 | SD card clock |
| SD_CS | GPIO34 | SD card chip select |
| **Environment** | | |
| DHT22_Data | GPIO3 | Temperature/humidity sensor |

### Reserved Pins (Do Not Use)
- GPIO0: Boot mode selection
- GPIO19, GPIO20: USB D-/D+
- GPIO26-GPIO32: SPI Flash/PSRAM
- GPIO33: SPIIO4
- GPIO38: SPIIO5
- GPIO39-GPIO42: JTAG interface

## Bill of Materials

### Core Components
| Component | Part Number | Quantity | Estimated Cost | Supplier |
|-----------|-------------|----------|----------------|---------|
| ESP32-S3-N16R8 DevKit | ESP32-S3-DevKitC-1 | 1 | $15-20 | Espressif |
| ToF Sensor | VL53L1X | 1 | $8-12 | STMicroelectronics |
| IMU | MPU6050 | 1 | $3-5 | InvenSense |
| Environment Sensor | DHT22 | 1 | $5-8 | Aosong |
| Light Sensor | BH1750 | 1 | $2-4 | ROHM |
| RFID Module | MFRC522 | 1 | $3-5 | NXP |
| GPS Module | NEO-6M | 1 | $8-12 | u-blox |
| Audio Amplifier | MAX98357A | 1 | $4-6 | Maxim |
| Servo Motor | SG90 | 1 | $3-5 | TowerPro |
| Vibration Motors | 3V DC Motor | 2 | $2-4 each | Generic |
| Piezo Buzzer | 12mm Buzzer | 1 | $1-2 | Generic |
| MicroSD Card | 8GB+ Class 10 | 1 | $5-10 | SanDisk |

### Supporting Components
| Component | Specification | Quantity | Notes |
|-----------|---------------|----------|-------|
| Tactile Switches | 6x6mm | 2 | User interface |
| LEDs | 3mm/5mm | 3-5 | Status indicators |
| Resistors | 220Ω, 10kΩ | 10 | Pull-ups, current limiting |
| Capacitors | 100nF, 10μF | 5 | Decoupling |
| Breadboard/PCB | Half-size | 1 | Prototyping |
| Jumper Wires | M-M, M-F | 50 | Connections |
| Power Bank | 5V, 10000mAh | 1 | Portable power |
| Enclosure | Waterproof box | 1 | Weather protection |

**Estimated Total Cost**: $80-120 USD

## Circuit & Wiring

### Power Supply Requirements
- **Input Voltage**: 5V USB or 3.7V Li-Po battery
- **Operating Voltage**: 3.3V (regulated)
- **Current Consumption**:
  - Idle: ~150mA
  - Active (all sensors): ~300mA
  - Peak (audio + vibration): ~500mA

### Wiring Guidelines
1. **Power Distribution**: Use dedicated 3.3V and GND rails
2. **I2C Bus**: Include 4.7kΩ pull-up resistors on SDA/SCL
3. **SPI Connections**: Keep wires short for high-frequency signals
4. **Audio System**: Use shielded cables for I2S signals
5. **Sensor Placement**: Mount ToF sensor at cane tip, IMU at handle

### Circuit Diagrams
Detailed wiring diagrams are available in:
- [Circuit Diagram Documentation](docs/circuit-diagram.md)
- [System Architecture SVG](assets/system-architecture.svg)
- [Circuit Overview SVG](assets/circuit-overview.svg)

## Software Architecture

### Firmware (Arduino ESP32)
```
SmartCaneESP32N16R8.ino
├── Core Modules
│   ├── SensorData.h          # Data structures
│   ├── Pins.h               # Pin definitions
│   ├── BLEManager.h/.cpp    # Bluetooth communication
│   ├── ToF.h/.cpp           # Distance sensing
│   ├── IMU.h/.cpp           # Motion detection
│   ├── GPSModule.h/.cpp     # Location services
│   ├── RFID.h/.cpp          # Indoor navigation
│   ├── EnvMonitor.h/.cpp    # Environmental sensors
│   ├── LightSensor.h/.cpp   # Ambient light
│   ├── SensorHealth.h/.cpp  # System diagnostics
│   └── AudioSystem.h/.cpp   # Voice feedback
├── Libraries
│   ├── VL53L1X              # ToF sensor
│   ├── MPU6050              # IMU
│   ├── DHT                  # Environment
│   ├── BH1750               # Light sensor
│   ├── MFRC522              # RFID
│   ├── SoftwareSerial       # GPS communication
│   └── ESP32Servo           # Servo control
└── Configuration
    ├── arduino_secrets.h    # WiFi/API credentials
    └── config.h            # System parameters
```

### Mobile App (Flutter)
```
lib/
├── main.dart               # App entry point
├── services/
│   ├── ble_service.dart    # Bluetooth communication
│   ├── storage_service.dart # Data persistence
│   ├── prefs_service.dart  # User preferences
│   └── command_service.dart # Serial commands
├── screens/
│   ├── dashboard_screen.dart    # Main dashboard
│   ├── history_screen.dart      # Data history
│   ├── serial_monitor_screen.dart # Debug interface
│   ├── caregiver_dashboard.dart  # Remote monitoring
│   ├── analytics_dashboard.dart  # Data analytics
│   └── settings_screen.dart     # Configuration
├── widgets/
│   ├── sensor_cards/       # UI components
│   ├── charts/            # Data visualization
│   └── common/            # Shared widgets
└── providers/
    └── providers.dart     # State management
```

## Setup & Installation

### Hardware Assembly

1. **Prepare the ESP32**
   ```bash
   # Install ESP32 board package in Arduino IDE
   # Board: ESP32S3 Dev Module
   # Flash Size: 16MB
   # PSRAM: OPI PSRAM
   ```

2. **Wire the Sensors**
   - Follow the pin mapping table above
   - Use breadboard for prototyping
   - Test each sensor individually

3. **Power System**
   - Connect 5V power supply or battery
   - Verify 3.3V regulation
   - Test current consumption

### Firmware Installation

1. **Clone Repository**
   ```bash
   git clone <repository-url>
   cd smart-assistive-cane
   ```

2. **Install Dependencies**
   ```bash
   # Arduino IDE: Install required libraries
   # Tools > Manage Libraries
   # Search and install:
   # - VL53L1X by Pololu
   # - MPU6050 by Electronic Cats
   # - DHT sensor library by Adafruit
   # - BH1750 by Christopher Laws
   # - MFRC522 by GithubCommunity
   # - ESP32Servo by Kevin Harrington
   ```

3. **Configure Secrets**
   ```cpp
   // Create arduino_secrets.h
   #define SECRET_SSID "your-wifi-name"
   #define SECRET_PASS "your-wifi-password"
   #define SECRET_API_KEY "your-api-key"
   ```

4. **Upload Firmware**
   ```bash
   # Select correct board and port
   # Upload SmartCaneESP32N16R8.ino
   # Monitor serial output for debugging
   ```

### Mobile App Installation

1. **Setup Flutter Environment**
   ```bash
   # Install Flutter SDK
   flutter doctor
   cd APP/
   flutter pub get
   ```

2. **Configure Permissions**
   ```yaml
   # Android: Update android/app/src/main/AndroidManifest.xml
   # iOS: Update ios/Runner/Info.plist
   # Add Bluetooth, Location, Microphone permissions
   ```

3. **Build and Install**
   ```bash
   # For Android
   flutter build apk
   flutter install
   
   # For iOS
   flutter build ios
   # Install via Xcode
   ```

## Usage & Operation Modes

### Basic Operation

1. **Power On**
   - Connect power source
   - Wait for initialization (LED indicators)
   - Listen for startup audio confirmation

2. **Pairing with Mobile App**
   - Enable Bluetooth on phone
   - Open Smart Cane app
   - Tap "Connect" and select device
   - Confirm pairing

3. **Walking Mode**
   - Hold cane normally
   - Audio feedback for obstacles
   - Vibration alerts for hazards
   - Automatic step counting

### Operation Modes

#### 1. Simple Mode (Default)
- Basic obstacle detection
- Audio/vibration feedback
- Step counting
- GPS tracking

#### 2. Radar Mode
- 180° scanning with servo
- Enhanced obstacle mapping
- Directional audio feedback
- Increased power consumption

#### 3. Indoor Navigation
- RFID room detection
- Audio room announcements
- Indoor mapping
- Location sharing

#### 4. Emergency Mode
- Fall detection active
- Automatic caregiver alerts
- GPS location sharing
- Emergency contact notification

### Button Controls

| Button | Press Type | Function |
|--------|------------|----------|
| Button 1 | Single | Toggle feedback mode |
| Button 1 | Double | Switch ToF mode |
| Button 1 | Long (3s) | Emergency alert |
| Button 2 | Single | Audio status report |
| Button 2 | Double | Toggle radar mode |
| Button 2 | Long (3s) | System reset |

## Serial Commands

### Command Categories

#### System Commands
| Command | Description | Example |
|---------|-------------|----------|
| `help` | Show all commands | `help` |
| `status` | System status report | `status` |
| `reset` | Restart system | `reset` |
| `version` | Firmware version | `version` |
| `uptime` | System uptime | `uptime` |

#### Sensor Commands
| Command | Description | Example |
|---------|-------------|----------|
| `tof_read` | Read ToF distance | `tof_read` |
| `tof_mode <mode>` | Set ToF mode (simple/radar) | `tof_mode radar` |
| `imu_read` | Read IMU data | `imu_read` |
| `gps_read` | Read GPS coordinates | `gps_read` |
| `env_read` | Read temperature/humidity | `env_read` |
| `light_read` | Read light level | `light_read` |
| `rfid_scan` | Scan for RFID tags | `rfid_scan` |

#### Feedback Commands
| Command | Description | Example |
|---------|-------------|----------|
| `audio_test` | Test audio system | `audio_test` |
| `vibrate <duration>` | Test vibration | `vibrate 1000` |
| `buzzer <freq> <duration>` | Test buzzer | `buzzer 2000 500` |
| `feedback_mode <mode>` | Set feedback mode | `feedback_mode both` |

#### BLE Commands
| Command | Description | Example |
|---------|-------------|----------|
| `ble_status` | BLE connection status | `ble_status` |
| `ble_restart` | Restart BLE service | `ble_restart` |
| `ble_send <data>` | Send BLE message | `ble_send hello` |

#### Configuration Commands
| Command | Description | Example |
|---------|-------------|----------|
| `set_threshold <value>` | Set distance threshold | `set_threshold 100` |
| `set_volume <level>` | Set audio volume | `set_volume 80` |
| `save_config` | Save current settings | `save_config` |
| `load_config` | Load saved settings | `load_config` |

#### Debug Commands
| Command | Description | Example |
|---------|-------------|----------|
| `debug_on` | Enable debug output | `debug_on` |
| `debug_off` | Disable debug output | `debug_off` |
| `sensor_health` | Check sensor status | `sensor_health` |
| `memory_info` | Show memory usage | `memory_info` |
| `task_info` | Show FreeRTOS tasks | `task_info` |

### Command Usage
```bash
# Connect to serial monitor (115200 baud)
# Type commands followed by Enter

> help
Available commands:
- System: help, status, reset, version
- Sensors: tof_read, imu_read, gps_read
- Feedback: audio_test, vibrate, buzzer
...

> tof_read
ToF Distance: 156 cm
Mode: Simple
Status: OK

> feedback_mode vibration
Feedback mode set to: Vibration only
```

## Mobile App

### App Features

#### 1. Dashboard Screen
- Real-time sensor data display
- Connection status indicator
- Quick action buttons
- Emergency alert button

#### 2. History Screen
- Sensor data timeline
- Activity summaries
- Export data functionality
- Trend analysis

#### 3. Serial Monitor
- Direct command interface
- Debug output viewing
- Command history
- Real-time logging

#### 4. Caregiver Dashboard
- Remote monitoring
- Location tracking
- Alert notifications
- Health summaries

#### 5. Analytics Dashboard
- Activity patterns
- Health metrics
- Progress tracking
- Custom reports

#### 6. Settings Screen
- Device configuration
- User preferences
- Notification settings
- Account management

### App Installation

1. **Download APK** (Android)
   ```bash
   # Build from source
   cd APP/
   flutter build apk --release
   # Install generated APK
   ```

2. **iOS Installation**
   ```bash
   # Requires Xcode and Apple Developer account
   flutter build ios --release
   # Deploy via Xcode
   ```

3. **Permissions Setup**
   - Grant Bluetooth permissions
   - Allow location access
   - Enable notifications
   - Grant microphone access (for voice commands)

### BLE Connection

1. **Pairing Process**
   - Ensure cane is powered on
   - Open app and tap "Scan"
   - Select "SmartCane" device
   - Confirm pairing

2. **Connection Troubleshooting**
   - Check Bluetooth is enabled
   - Restart both devices
   - Clear app cache
   - Re-pair devices

## Troubleshooting

### Common Issues

#### Hardware Issues

**Problem**: No power/LED indicators
- **Solution**: Check power connections, verify 5V supply
- **Debug**: Measure voltage at ESP32 3.3V pin

**Problem**: Sensors not responding
- **Solution**: Verify I2C connections, check pull-up resistors
- **Debug**: Use I2C scanner sketch to detect devices

**Problem**: Audio not working
- **Solution**: Check I2S wiring, verify MAX98357A connections
- **Debug**: Test with simple tone generation

**Problem**: GPS not getting fix
- **Solution**: Ensure outdoor location, check antenna
- **Debug**: Monitor NMEA sentences in serial output

#### Software Issues

**Problem**: Compilation errors
- **Solution**: Install required libraries, check board selection
- **Debug**: Verify Arduino IDE configuration

**Problem**: BLE connection fails
- **Solution**: Restart both devices, clear Bluetooth cache
- **Debug**: Check serial output for BLE status

**Problem**: Sensor readings incorrect
- **Solution**: Calibrate sensors, check wiring
- **Debug**: Use individual sensor test commands

#### Mobile App Issues

**Problem**: App won't connect to cane
- **Solution**: Check Bluetooth permissions, restart app
- **Debug**: Use BLE scanner app to verify device visibility

**Problem**: Data not updating
- **Solution**: Check BLE connection, restart services
- **Debug**: Monitor serial output for data transmission

### Debug Procedures

1. **Serial Monitor Debug**
   ```bash
   # Connect at 115200 baud
   # Enable debug output
   > debug_on
   # Check sensor status
   > sensor_health
   # Monitor data flow
   > status
   ```

2. **Hardware Testing**
   ```bash
   # Test individual components
   > tof_read
   > imu_read
   > audio_test
   > vibrate 1000
   ```

3. **BLE Debugging**
   ```bash
   # Check BLE status
   > ble_status
   # Restart BLE service
   > ble_restart
   # Monitor connections
   > debug_ble
   ```

### Performance Optimization

1. **Power Management**
   - Disable unused sensors
   - Adjust sampling rates
   - Use sleep modes when idle

2. **Memory Optimization**
   - Monitor heap usage
   - Optimize data structures
   - Clear unused variables

3. **Communication Optimization**
   - Reduce BLE packet frequency
   - Compress sensor data
   - Use efficient data formats

## Development

### Development Environment

1. **Arduino IDE Setup**
   ```bash
   # Install ESP32 board package
   # Configure board: ESP32S3 Dev Module
   # Set Flash Size: 16MB
   # Enable PSRAM: OPI PSRAM
   ```

2. **Flutter Development**
   ```bash
   # Install Flutter SDK
   flutter doctor
   # Setup IDE (VS Code/Android Studio)
   # Install Flutter/Dart plugins
   ```

3. **Version Control**
   ```bash
   git clone <repository>
   git checkout -b feature/your-feature
   # Make changes
   git commit -m "feat: add new feature"
   git push origin feature/your-feature
   ```

### Code Structure

#### Firmware Architecture
- **Modular Design**: Each sensor has dedicated module
- **FreeRTOS Tasks**: Concurrent sensor processing
- **Event-Driven**: Interrupt-based sensor updates
- **Memory Management**: Efficient buffer usage

#### App Architecture
- **State Management**: Riverpod for reactive updates
- **Service Layer**: Abstracted BLE communication
- **UI Components**: Reusable widget library
- **Data Persistence**: Local storage with Hive

### Testing

1. **Hardware Testing**
   - Unit tests for each sensor
   - Integration tests for sensor fusion
   - Performance benchmarks
   - Power consumption analysis

2. **Software Testing**
   - Unit tests for core functions
   - Widget tests for UI components
   - Integration tests for BLE communication
   - End-to-end user scenarios

### Contributing Guidelines

See [CONTRIBUTING.md](CONTRIBUTING.md) for detailed guidelines on:
- Code style and conventions
- Pull request process
- Issue reporting
- Development workflow

## Roadmap

### Version 1.1 (Q2 2025)
- [ ] Enhanced fall detection algorithms
- [ ] Voice command recognition
- [ ] Improved battery life optimization
- [ ] Weather integration
- [ ] Multi-language support

### Version 1.2 (Q3 2025)
- [ ] Machine learning obstacle classification
- [ ] Cloud data synchronization
- [ ] Advanced analytics dashboard
- [ ] Caregiver mobile notifications
- [ ] Route planning and navigation

### Version 2.0 (Q4 2025)
- [ ] Computer vision integration
- [ ] AI-powered navigation assistance
- [ ] Smart home integration
- [ ] Wearable device connectivity
- [ ] Advanced health monitoring

### Long-term Goals
- [ ] FDA medical device certification
- [ ] Insurance coverage integration
- [ ] Large-scale deployment
- [ ] Research partnerships
- [ ] Open-source community growth

## Contributing

We welcome contributions from the community! Please read our [Contributing Guidelines](CONTRIBUTING.md) and [Code of Conduct](CODE_OF_CONDUCT.md) before getting started.

### Quick Start for Contributors
1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Test thoroughly
5. Submit a pull request

### Areas for Contribution
- 🔧 Hardware improvements and testing
- 💻 Software feature development
- 📚 Documentation and tutorials
- 🧪 Testing and quality assurance
- 🎨 UI/UX design improvements
- 🌍 Internationalization and accessibility

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

---

**Smart Assistive Cane V1** - Empowering independence through technology

For support, questions, or contributions, please visit our [GitHub repository](https://github.com/your-repo/smart-assistive-cane) or contact the development team.

*Last updated: January 2025*