# 🚀 Deployment Guide

## 📋 Overview

This comprehensive deployment guide covers the setup and deployment of all components of the Smart Assistive Cane V1 system, including hardware firmware, mobile application, and supporting infrastructure.

## 🔧 Hardware Deployment

### 📦 Prerequisites

#### Required Hardware
- ESP32-S3 development board
- VL53L0X ToF distance sensor
- MPU6050 IMU sensor
- NEO-6M GPS module
- Buzzer and LED indicators
- Battery pack (3.7V Li-Po recommended)
- Connecting wires and breadboard/PCB

#### Required Software
- Arduino IDE 2.0+ or PlatformIO
- ESP32 board package
- Required libraries (see hardware/README.md)

### ⚙️ Hardware Setup Steps

#### 1. **Environment Setup**
```bash
# Install Arduino IDE or PlatformIO
# Add ESP32 board package
# Install required libraries
```

#### 2. **Hardware Assembly**
```
📌 Pin Connections (ESP32-S3):
├── VL53L0X (I2C)
│   ├── SDA → GPIO 21
│   ├── SCL → GPIO 22
│   ├── VCC → 3.3V
│   └── GND → GND
├── MPU6050 (I2C)
│   ├── SDA → GPIO 21
│   ├── SCL → GPIO 22
│   ├── VCC → 3.3V
│   └── GND → GND
├── GPS Module (UART)
│   ├── TX → GPIO 16
│   ├── RX → GPIO 17
│   ├── VCC → 3.3V
│   └── GND → GND
├── Buzzer
│   ├── Signal → GPIO 25
│   └── GND → GND
└── LED
    ├── Anode → GPIO 2
    └── Cathode → GND (via 220Ω resistor)
```

#### 3. **Firmware Compilation & Upload**
```bash
# Navigate to hardware directory
cd hardware/

# Open main.cpp in Arduino IDE
# Select ESP32-S3 board
# Configure upload settings:
# - Board: "ESP32S3 Dev Module"
# - Upload Speed: "921600"
# - USB Mode: "Hardware CDC and JTAG"
# - Flash Size: "4MB (32Mb)"

# Compile and upload
# Arduino IDE: Ctrl+U
# PlatformIO: pio run --target upload
```

#### 4. **Hardware Testing**
```bash
# Open Serial Monitor (115200 baud)
# Verify sensor initialization
# Test basic functionality:
# - ToF distance readings
# - IMU orientation data
# - GPS coordinate acquisition
# - Audio feedback
# - BLE connectivity
```

### 🔍 Hardware Troubleshooting

| Issue | Symptoms | Solution |
|-------|----------|----------|
| **Sensor Not Detected** | "Sensor initialization failed" | Check I2C connections, verify power supply |
| **GPS No Fix** | No coordinate data | Ensure outdoor testing, check antenna connection |
| **BLE Connection Issues** | Mobile app can't connect | Verify BLE is enabled, check device pairing |
| **Power Issues** | Random resets, unstable operation | Check battery voltage, verify power connections |
| **Upload Failures** | Compilation or upload errors | Check board selection, USB cable, drivers |

## 📱 Mobile App Deployment

### 🛠️ Development Environment Setup

#### Prerequisites
```bash
# Flutter SDK (latest stable)
flutter --version

# Platform-specific requirements:
# Android: Android Studio, Android SDK
# iOS: Xcode, iOS SDK (macOS only)
```

#### Environment Configuration
```bash
# Clone repository
git clone <repository-url>
cd mobile/

# Install dependencies
flutter pub get

# Verify setup
flutter doctor
```

### 🏗️ Build Process

#### Android Deployment

##### Debug Build
```bash
# Connect Android device or start emulator
flutter devices

# Run debug build
flutter run
```

##### Release Build
```bash
# Generate release APK
flutter build apk --release

# Generate App Bundle (recommended for Play Store)
flutter build appbundle --release

# Output location:
# APK: build/app/outputs/flutter-apk/app-release.apk
# AAB: build/app/outputs/bundle/release/app-release.aab
```

##### Play Store Deployment
```bash
# 1. Create keystore (one-time setup)
keytool -genkey -v -keystore ~/upload-keystore.jks -keyalg RSA -keysize 2048 -validity 10000 -alias upload

# 2. Configure signing (android/key.properties)
storePassword=<password>
keyPassword=<password>
keyAlias=upload
storeFile=<path-to-keystore>

# 3. Build signed release
flutter build appbundle --release

# 4. Upload to Play Console
# - Create app listing
# - Upload AAB file
# - Complete store listing
# - Submit for review
```

#### iOS Deployment

##### Debug Build
```bash
# Open iOS Simulator or connect device
open -a Simulator

# Run debug build
flutter run
```

##### Release Build
```bash
# Build iOS release
flutter build ios --release

# Open Xcode for final steps
open ios/Runner.xcworkspace

# In Xcode:
# 1. Select "Runner" project
# 2. Choose "Product" → "Archive"
# 3. Use Organizer to distribute
```

##### App Store Deployment
```bash
# Prerequisites:
# - Apple Developer Account
# - App Store Connect app created
# - Provisioning profiles configured

# 1. Configure app in Xcode
# - Set bundle identifier
# - Configure signing certificates
# - Set deployment target

# 2. Archive and upload
# - Product → Archive in Xcode
# - Distribute App → App Store Connect
# - Upload to App Store Connect

# 3. App Store Connect
# - Complete app information
# - Add screenshots and metadata
# - Submit for App Review
```

### 📱 Mobile App Testing

#### Unit Testing
```bash
# Run all unit tests
flutter test

# Run specific test file
flutter test test/widget_test.dart

# Generate coverage report
flutter test --coverage
genhtml coverage/lcov.info -o coverage/html
```

#### Integration Testing
```bash
# Run integration tests
flutter test integration_test/

# Run on specific device
flutter test integration_test/ -d <device-id>
```

#### Device Testing Checklist
- [ ] BLE connectivity with hardware
- [ ] GPS location services
- [ ] Audio feedback functionality
- [ ] Emergency SOS features
- [ ] Settings persistence
- [ ] Background operation
- [ ] Battery optimization
- [ ] Accessibility features

## 🌐 Infrastructure Deployment

### ☁️ Backend Services (Optional)

If implementing cloud features:

#### Supabase Setup
```bash
# 1. Create Supabase project
# 2. Configure database schema
# 3. Set up authentication
# 4. Configure Row Level Security (RLS)
# 5. Generate API keys
```

#### Environment Configuration
```bash
# Create .env file
SUPABASE_URL=your-project-url
SUPABASE_ANON_KEY=your-anon-key
SUPABASE_SERVICE_ROLE_KEY=your-service-role-key
```

### 📊 Analytics & Monitoring

#### Firebase Setup (Optional)
```bash
# 1. Create Firebase project
# 2. Add Android/iOS apps
# 3. Download configuration files
# 4. Configure analytics
# 5. Set up crash reporting
```

## 🔒 Security Considerations

### 🛡️ Hardware Security
- **Secure Boot**: Enable ESP32 secure boot features
- **Encryption**: Implement data encryption for sensitive information
- **Authentication**: Secure BLE pairing process
- **Firmware Protection**: Code signing and verification

### 📱 Mobile App Security
- **Data Encryption**: Encrypt local storage
- **Network Security**: Use HTTPS/TLS for all communications
- **Permission Management**: Request minimal necessary permissions
- **Code Obfuscation**: Protect against reverse engineering

### 🔐 Best Practices
```bash
# Hardware
- Change default passwords
- Use secure communication protocols
- Implement proper error handling
- Regular security updates

# Mobile
- Validate all inputs
- Implement certificate pinning
- Use secure storage APIs
- Regular dependency updates
```

## 📈 Performance Optimization

### ⚡ Hardware Optimization
- **Power Management**: Implement sleep modes
- **Sensor Polling**: Optimize reading frequencies
- **Memory Usage**: Efficient data structures
- **Processing**: Optimize algorithms

### 📱 Mobile Optimization
- **Battery Usage**: Background processing optimization
- **Memory Management**: Efficient widget lifecycle
- **Network Usage**: Minimize data transfer
- **UI Performance**: Smooth animations and transitions

## 🔧 Maintenance & Updates

### 🔄 Update Process

#### Hardware Updates
```bash
# 1. Test new firmware thoroughly
# 2. Create backup of current firmware
# 3. Deploy via OTA or USB
# 4. Verify functionality
# 5. Monitor for issues
```

#### Mobile App Updates
```bash
# 1. Version increment
# 2. Update changelog
# 3. Test on multiple devices
# 4. Deploy to app stores
# 5. Monitor crash reports
```

### 📊 Monitoring
- **Hardware**: Serial monitoring, telemetry data
- **Mobile**: Crash reports, analytics, user feedback
- **Performance**: Battery usage, response times
- **Security**: Vulnerability scanning, penetration testing

## 🆘 Emergency Procedures

### 🚨 Critical Issues

#### Hardware Failures
1. **Immediate Response**: Switch to backup device if available
2. **Diagnosis**: Identify failed component
3. **Repair/Replace**: Fix or replace faulty hardware
4. **Testing**: Verify full functionality
5. **Documentation**: Record incident and resolution

#### App Store Issues
1. **Rejection**: Address review feedback promptly
2. **Critical Bugs**: Prepare emergency update
3. **Security Issues**: Immediate patch and communication
4. **User Reports**: Rapid response and resolution

### 📞 Support Contacts
- **Hardware Issues**: hardware-support@smartcane.dev
- **Mobile App Issues**: mobile-support@smartcane.dev
- **Security Issues**: security@smartcane.dev
- **Emergency**: emergency@smartcane.dev

---

## 📚 Additional Resources

- [Hardware Documentation](hardware/README.md)
- [Mobile App Documentation](docs/mobile/README.md)
- [API Documentation](docs/api/README.md)
- [Security Policy](SECURITY.md)
- [Contributing Guidelines](CONTRIBUTING.md)
- [Troubleshooting Guide](docs/TROUBLESHOOTING.md)

---

*Deployment Guide - Smart Assistive Cane V1*
*Last Updated: August 2025*