# 📱 Smart Assistive Cane V1 - Mobile Application

![Flutter](https://img.shields.io/badge/Flutter-02569B?style=flat&logo=flutter&logoColor=white)
![Dart](https://img.shields.io/badge/Dart-0175C2?style=flat&logo=dart&logoColor=white)
![License](https://img.shields.io/badge/license-MIT-blue.svg)
![Platform](https://img.shields.io/badge/platform-Android%20%7C%20iOS-lightgrey)
![Made with ❤️](https://img.shields.io/badge/Made%20with-❤️-red.svg)

A comprehensive Flutter mobile application designed to complement the Smart Assistive Cane hardware, providing real-time monitoring, navigation assistance, and emergency management for visually impaired users and their caregivers.

## 🎯 Application Vision

Bridging the gap between assistive hardware and intelligent software to create a seamless navigation experience that empowers independence while ensuring safety and connectivity for visually impaired individuals.

## ✨ Key Features

### 👤 Multi-User Support
- **Primary Users**: Visually impaired individuals with comprehensive navigation tools
- **Caregivers**: Remote monitoring and emergency response capabilities
- **Technical Support**: Device management and troubleshooting tools

### 🧭 Navigation & Guidance
- **Real-time Obstacle Detection**: Live distance readings with audio/haptic feedback
- **Indoor Navigation**: RFID-based room identification and waypoint management
- **GPS Tracking**: Outdoor positioning with route planning and guidance
- **Environmental Awareness**: Temperature, humidity, and lighting condition monitoring
- **Motion Analysis**: Step counting, gait analysis, and fall detection

### 🔗 Smart Connectivity
- **Bluetooth Low Energy**: Real-time data streaming from cane hardware
- **Cloud Synchronization**: Data backup and multi-device access
- **Offline Mode**: Core functionality without internet connectivity
- **Emergency Alerts**: Automatic notifications to emergency contacts

### 📊 Health & Analytics
- **Activity Tracking**: Daily step counts and movement patterns
- **Health Monitoring**: Fall detection history and recovery metrics
- **Usage Statistics**: Device utilization and feature adoption
- **Progress Reports**: Weekly and monthly activity summaries

## 🏗️ Application Architecture

### Project Structure

```
APP/
├── lib/
│   ├── main.dart                    # Application entry point
│   ├── app.dart                     # App configuration and routing
│   ├── core/
│   │   ├── constants/
│   │   │   ├── app_constants.dart   # Global constants
│   │   │   ├── colors.dart          # Color palette
│   │   │   └── text_styles.dart     # Typography definitions
│   │   ├── services/
│   │   │   ├── bluetooth_service.dart # BLE communication
│   │   │   ├── location_service.dart  # GPS and positioning
│   │   │   ├── storage_service.dart   # Local data storage
│   │   │   └── notification_service.dart # Push notifications
│   │   ├── utils/
│   │   │   ├── helpers.dart         # Utility functions
│   │   │   ├── validators.dart      # Input validation
│   │   │   └── formatters.dart      # Data formatting
│   │   └── models/
│   │       ├── sensor_data.dart     # Hardware sensor models
│   │       ├── user_profile.dart    # User account models
│   │       └── navigation_data.dart # Location and route models
│   ├── features/
│   │   ├── authentication/
│   │   │   ├── screens/
│   │   │   │   ├── login_screen.dart
│   │   │   │   ├── register_screen.dart
│   │   │   │   └── profile_setup_screen.dart
│   │   │   ├── widgets/
│   │   │   └── providers/
│   │   ├── dashboard/
│   │   │   ├── screens/
│   │   │   │   ├── main_dashboard.dart
│   │   │   │   ├── device_status.dart
│   │   │   │   └── quick_actions.dart
│   │   │   ├── widgets/
│   │   │   │   ├── sensor_card.dart
│   │   │   │   ├── battery_indicator.dart
│   │   │   │   └── connection_status.dart
│   │   │   └── providers/
│   │   ├── navigation/
│   │   │   ├── screens/
│   │   │   │   ├── indoor_navigation.dart
│   │   │   │   ├── outdoor_navigation.dart
│   │   │   │   └── waypoint_management.dart
│   │   │   ├── widgets/
│   │   │   │   ├── compass_widget.dart
│   │   │   │   ├── distance_indicator.dart
│   │   │   │   └── route_planner.dart
│   │   │   └── providers/
│   │   ├── health/
│   │   │   ├── screens/
│   │   │   │   ├── activity_tracking.dart
│   │   │   │   ├── fall_detection.dart
│   │   │   │   └── health_reports.dart
│   │   │   ├── widgets/
│   │   │   │   ├── step_counter.dart
│   │   │   │   ├── activity_chart.dart
│   │   │   │   └── health_metrics.dart
│   │   │   └── providers/
│   │   ├── emergency/
│   │   │   ├── screens/
│   │   │   │   ├── emergency_contacts.dart
│   │   │   │   ├── alert_history.dart
│   │   │   │   └── emergency_settings.dart
│   │   │   ├── widgets/
│   │   │   │   ├── emergency_button.dart
│   │   │   │   ├── contact_card.dart
│   │   │   │   └── alert_notification.dart
│   │   │   └── providers/
│   │   ├── settings/
│   │   │   ├── screens/
│   │   │   │   ├── device_settings.dart
│   │   │   │   ├── accessibility_settings.dart
│   │   │   │   └── app_preferences.dart
│   │   │   ├── widgets/
│   │   │   │   ├── setting_tile.dart
│   │   │   │   ├── slider_setting.dart
│   │   │   │   └── toggle_setting.dart
│   │   │   └── providers/
│   │   └── caregiver/
│   │       ├── screens/
│   │       │   ├── monitoring_dashboard.dart
│   │       │   ├── location_tracking.dart
│   │       │   └── alert_management.dart
│   │       ├── widgets/
│   │       │   ├── user_location_map.dart
│   │       │   ├── activity_timeline.dart
│   │       │   └── alert_card.dart
│   │       └── providers/
│   └── shared/
│       ├── widgets/
│       │   ├── custom_button.dart
│       │   ├── loading_indicator.dart
│       │   ├── error_widget.dart
│       │   └── accessibility_wrapper.dart
│       ├── themes/
│       │   ├── app_theme.dart
│       │   ├── light_theme.dart
│       │   └── dark_theme.dart
│       └── extensions/
│           ├── context_extensions.dart
│           ├── string_extensions.dart
│           └── datetime_extensions.dart
├── assets/
│   ├── images/
│   │   ├── icons/
│   │   ├── illustrations/
│   │   └── logos/
│   ├── sounds/
│   │   ├── alerts/
│   │   ├── navigation/
│   │   └── feedback/
│   └── fonts/
├── test/
│   ├── unit/
│   ├── widget/
│   └── integration/
├── android/
├── ios/
├── web/
├── pubspec.yaml
└── README.md
```

### State Management

**Provider Pattern Implementation:**
```dart
// Core state management using Provider
class AppState extends ChangeNotifier {
  // Device connection state
  bool _isConnected = false;
  DeviceInfo? _deviceInfo;
  
  // Sensor data state
  SensorData? _currentSensorData;
  List<SensorReading> _sensorHistory = [];
  
  // Navigation state
  LocationData? _currentLocation;
  List<Waypoint> _waypoints = [];
  
  // User preferences
  UserPreferences _preferences = UserPreferences();
  
  // Getters and methods...
}
```

### Core Dependencies

```yaml
dependencies:
  flutter:
    sdk: flutter
  
  # State Management
  provider: ^6.1.1
  
  # Bluetooth Communication
  flutter_bluetooth_serial: ^0.4.0
  permission_handler: ^11.0.1
  
  # Location Services
  geolocator: ^10.1.0
  google_maps_flutter: ^2.5.0
  
  # Local Storage
  shared_preferences: ^2.2.2
  sqflite: ^2.3.0
  
  # UI Components
  cupertino_icons: ^1.0.6
  flutter_svg: ^2.0.9
  cached_network_image: ^3.3.0
  
  # Charts and Visualization
  fl_chart: ^0.65.0
  syncfusion_flutter_charts: ^23.2.7
  
  # Notifications
  flutter_local_notifications: ^16.3.0
  firebase_messaging: ^14.7.6
  
  # Audio and Accessibility
  flutter_tts: ^3.8.5
  vibration: ^1.8.4
  
  # Networking
  http: ^1.1.2
  dio: ^5.4.0
  
  # Utilities
  intl: ^0.19.0
  uuid: ^4.2.1
  path_provider: ^2.1.1

dev_dependencies:
  flutter_test:
    sdk: flutter
  flutter_lints: ^3.0.1
  mockito: ^5.4.4
  integration_test:
    sdk: flutter
```

## 🛠️ Installation & Setup

### Prerequisites

1. **Flutter SDK** (version 3.16.0 or later)
2. **Dart SDK** (version 3.2.0 or later)
3. **Android Studio** or **VS Code** with Flutter extensions
4. **Android SDK** (API level 21+) for Android development
5. **Xcode** (version 14+) for iOS development (macOS only)

### Step-by-Step Installation

#### 1. Environment Setup

```bash
# Clone the repository
git clone https://github.com/HasnainMemon2001/Smart-Assistive-Cane-V1.git
cd Smart-Assistive-Cane-V1/APP

# Verify Flutter installation
flutter doctor

# Get dependencies
flutter pub get
```

#### 2. Platform Configuration

**Android Setup:**

1. **Permissions** (android/app/src/main/AndroidManifest.xml):
```xml
<uses-permission android:name="android.permission.BLUETOOTH" />
<uses-permission android:name="android.permission.BLUETOOTH_ADMIN" />
<uses-permission android:name="android.permission.ACCESS_FINE_LOCATION" />
<uses-permission android:name="android.permission.ACCESS_COARSE_LOCATION" />
<uses-permission android:name="android.permission.VIBRATE" />
<uses-permission android:name="android.permission.WAKE_LOCK" />
<uses-permission android:name="android.permission.FOREGROUND_SERVICE" />
```

2. **Minimum SDK** (android/app/build.gradle):
```gradle
android {
    compileSdkVersion 34
    defaultConfig {
        minSdkVersion 21
        targetSdkVersion 34
    }
}
```

**iOS Setup:**

1. **Permissions** (ios/Runner/Info.plist):
```xml
<key>NSBluetoothAlwaysUsageDescription</key>
<string>This app needs Bluetooth to connect to the Smart Cane device</string>
<key>NSLocationWhenInUseUsageDescription</key>
<string>This app needs location access for navigation assistance</string>
<key>NSLocationAlwaysAndWhenInUseUsageDescription</key>
<string>This app needs location access for emergency features</string>
```

2. **Deployment Target** (ios/Runner.xcodeproj):
   - Set iOS Deployment Target to 12.0 or higher

#### 3. Firebase Configuration (Optional)

**For push notifications and cloud features:**

```bash
# Install Firebase CLI
npm install -g firebase-tools

# Login to Firebase
firebase login

# Initialize Firebase in project
firebase init

# Add Firebase configuration files
# - google-services.json (Android)
# - GoogleService-Info.plist (iOS)
```

#### 4. Build and Run

```bash
# Run on connected device/emulator
flutter run

# Build for release
flutter build apk --release  # Android
flutter build ios --release  # iOS

# Run tests
flutter test
```

## 📱 Application Features

### 🏠 Dashboard Screen

**Real-time Device Monitoring:**
- Live sensor readings (distance, motion, environment)
- Battery level and charging status
- Bluetooth connection strength
- GPS signal quality
- Device health indicators

**Quick Actions:**
- Emergency alert button
- Device settings shortcut
- Navigation mode toggle
- Feedback configuration

### 🧭 Navigation Features

**Indoor Navigation:**
- RFID waypoint management
- Room identification and mapping
- Audio guidance for room transitions
- Custom location labeling

**Outdoor Navigation:**
- GPS-based route planning
- Turn-by-turn voice guidance
- Landmark identification
- Safe route suggestions

**Obstacle Detection:**
- Real-time distance visualization
- Proximity alerts configuration
- Detection mode switching (simple/radar)
- Sensitivity adjustment

### 📊 Health & Activity Tracking

**Daily Activity:**
- Step counting and goal setting
- Distance traveled tracking
- Active time monitoring
- Calorie estimation

**Fall Detection:**
- Automatic fall detection alerts
- Fall history and analysis
- Recovery time tracking
- Emergency contact notification

**Health Reports:**
- Weekly activity summaries
- Monthly progress reports
- Trend analysis and insights
- Export functionality

### 🚨 Emergency Management

**Emergency Contacts:**
- Primary and secondary contact setup
- Automatic location sharing
- Emergency message templates
- Contact verification system

**Alert System:**
- Manual emergency activation
- Automatic fall detection alerts
- GPS location broadcasting
- Emergency services integration

### ⚙️ Settings & Configuration

**Device Settings:**
- Sensor calibration
- Feedback preferences (audio/haptic)
- Detection sensitivity
- Power management

**Accessibility Settings:**
- Voice guidance configuration
- Text size and contrast
- Button size adjustment
- Screen reader compatibility

**App Preferences:**
- Notification settings
- Data sync preferences
- Privacy controls
- Language selection

### 👥 Caregiver Features

**Remote Monitoring:**
- Real-time location tracking
- Activity status updates
- Device health monitoring
- Emergency alert reception

**Historical Data:**
- Activity timeline view
- Route history
- Health trend analysis
- Incident reports

## 🎨 User Interface Design

### Design System

**Color Palette:**
```dart
class AppColors {
  // Primary Colors
  static const Color primary = Color(0xFF2196F3);      // Blue
  static const Color primaryDark = Color(0xFF1976D2);  // Dark Blue
  static const Color accent = Color(0xFF4CAF50);       // Green
  
  // Semantic Colors
  static const Color success = Color(0xFF4CAF50);      // Green
  static const Color warning = Color(0xFFFF9800);      // Orange
  static const Color error = Color(0xFFF44336);        // Red
  static const Color info = Color(0xFF2196F3);         // Blue
  
  // Neutral Colors
  static const Color background = Color(0xFFF5F5F5);   // Light Gray
  static const Color surface = Color(0xFFFFFFFF);      // White
  static const Color onSurface = Color(0xFF212121);    // Dark Gray
  
  // High Contrast (Accessibility)
  static const Color highContrastText = Color(0xFF000000);
  static const Color highContrastBackground = Color(0xFFFFFFFF);
}
```

**Typography:**
```dart
class AppTextStyles {
  static const TextStyle heading1 = TextStyle(
    fontSize: 32,
    fontWeight: FontWeight.bold,
    height: 1.2,
  );
  
  static const TextStyle heading2 = TextStyle(
    fontSize: 24,
    fontWeight: FontWeight.w600,
    height: 1.3,
  );
  
  static const TextStyle body1 = TextStyle(
    fontSize: 16,
    fontWeight: FontWeight.normal,
    height: 1.5,
  );
  
  static const TextStyle button = TextStyle(
    fontSize: 16,
    fontWeight: FontWeight.w600,
    letterSpacing: 0.5,
  );
}
```

### Accessibility Features

**Screen Reader Support:**
- Semantic labels for all interactive elements
- Proper heading hierarchy
- Descriptive button labels
- Content grouping and navigation

**Visual Accessibility:**
- High contrast mode support
- Scalable text sizes (up to 200%)
- Color-blind friendly palette
- Focus indicators for keyboard navigation

**Motor Accessibility:**
- Large touch targets (minimum 44x44 dp)
- Gesture alternatives for all actions
- Voice control integration
- Switch control support

## 🧪 Testing

### Test Structure

```bash
test/
├── unit/
│   ├── services/
│   │   ├── bluetooth_service_test.dart
│   │   ├── location_service_test.dart
│   │   └── storage_service_test.dart
│   ├── models/
│   │   ├── sensor_data_test.dart
│   │   └── user_profile_test.dart
│   └── utils/
│       ├── helpers_test.dart
│       └── validators_test.dart
├── widget/
│   ├── dashboard/
│   │   ├── sensor_card_test.dart
│   │   └── battery_indicator_test.dart
│   ├── navigation/
│   │   ├── compass_widget_test.dart
│   │   └── distance_indicator_test.dart
│   └── shared/
│       ├── custom_button_test.dart
│       └── accessibility_wrapper_test.dart
└── integration/
    ├── app_test.dart
    ├── bluetooth_connection_test.dart
    └── navigation_flow_test.dart
```

### Running Tests

```bash
# Run all tests
flutter test

# Run specific test file
flutter test test/unit/services/bluetooth_service_test.dart

# Run tests with coverage
flutter test --coverage

# Run integration tests
flutter drive --target=test_driver/app.dart
```

### Test Examples

**Unit Test Example:**
```dart
import 'package:flutter_test/flutter_test.dart';
import 'package:mockito/mockito.dart';
import 'package:smart_cane_app/services/bluetooth_service.dart';

void main() {
  group('BluetoothService', () {
    late BluetoothService bluetoothService;
    
    setUp(() {
      bluetoothService = BluetoothService();
    });
    
    test('should connect to device successfully', () async {
      // Arrange
      const deviceAddress = '00:11:22:33:44:55';
      
      // Act
      final result = await bluetoothService.connectToDevice(deviceAddress);
      
      // Assert
      expect(result.isSuccess, true);
      expect(bluetoothService.isConnected, true);
    });
  });
}
```

**Widget Test Example:**
```dart
import 'package:flutter/material.dart';
import 'package:flutter_test/flutter_test.dart';
import 'package:smart_cane_app/features/dashboard/widgets/sensor_card.dart';

void main() {
  testWidgets('SensorCard displays correct data', (WidgetTester tester) async {
    // Arrange
    const sensorData = SensorData(
      distance: 150,
      temperature: 25.5,
      humidity: 60.0,
    );
    
    // Act
    await tester.pumpWidget(
      MaterialApp(
        home: Scaffold(
          body: SensorCard(data: sensorData),
        ),
      ),
    );
    
    // Assert
    expect(find.text('150 cm'), findsOneWidget);
    expect(find.text('25.5°C'), findsOneWidget);
    expect(find.text('60.0%'), findsOneWidget);
  });
}
```

## 🚀 Build & Deployment

### Android Deployment

**1. Generate Signing Key:**
```bash
keytool -genkey -v -keystore ~/upload-keystore.jks -keyalg RSA -keysize 2048 -validity 10000 -alias upload
```

**2. Configure Signing (android/key.properties):**
```properties
storePassword=<password>
keyPassword=<password>
keyAlias=upload
storeFile=<path-to-keystore>
```

**3. Build Release APK:**
```bash
flutter build apk --release
flutter build appbundle --release  # For Play Store
```

### iOS Deployment

**1. Configure Xcode Project:**
- Set up provisioning profiles
- Configure app signing
- Set deployment target

**2. Build Release IPA:**
```bash
flutter build ios --release
```

**3. Archive and Upload:**
- Use Xcode Organizer
- Upload to App Store Connect
- Submit for review

### Continuous Integration

**GitHub Actions Example (.github/workflows/ci.yml):**
```yaml
name: CI/CD Pipeline

on:
  push:
    branches: [ main, develop ]
  pull_request:
    branches: [ main ]

jobs:
  test:
    runs-on: ubuntu-latest
    steps:
    - uses: actions/checkout@v3
    - uses: subosito/flutter-action@v2
      with:
        flutter-version: '3.16.0'
    - run: flutter pub get
    - run: flutter analyze
    - run: flutter test
    
  build-android:
    needs: test
    runs-on: ubuntu-latest
    steps:
    - uses: actions/checkout@v3
    - uses: subosito/flutter-action@v2
    - run: flutter pub get
    - run: flutter build apk --release
    
  build-ios:
    needs: test
    runs-on: macos-latest
    steps:
    - uses: actions/checkout@v3
    - uses: subosito/flutter-action@v2
    - run: flutter pub get
    - run: flutter build ios --release --no-codesign
```

## 🤝 Contributing

We welcome contributions to improve the Smart Assistive Cane mobile application! Please see our [CONTRIBUTING.md](../CONTRIBUTING.md) for detailed guidelines.

### Development Guidelines

**Code Style:**
- Follow [Dart Style Guide](https://dart.dev/guides/language/effective-dart/style)
- Use `flutter analyze` to check code quality
- Format code with `dart format`
- Write meaningful commit messages

**Pull Request Process:**
1. Fork the repository
2. Create a feature branch
3. Write tests for new functionality
4. Ensure all tests pass
5. Update documentation
6. Submit pull request

**Testing Requirements:**
- Unit tests for all business logic
- Widget tests for custom components
- Integration tests for critical user flows
- Accessibility testing with screen readers
- Performance testing on low-end devices

## 📄 License

This project is licensed under the MIT License - see the [LICENSE](../LICENSE) file for details.

## 🙏 Acknowledgments

- **Flutter Team** for the excellent cross-platform framework
- **Accessibility Community** for feedback and testing
- **Open Source Contributors** for various packages and libraries
- **Beta Testers** for valuable user experience insights

## 📞 Support

For technical support and questions:

- **Issues**: [GitHub Issues](https://github.com/HasnainMemon2001/Smart-Assistive-Cane-V1/issues)
- **Discussions**: [GitHub Discussions](https://github.com/HasnainMemon2001/Smart-Assistive-Cane-V1/discussions)
- **Email**: hasnainmemon2001@gmail.com
- **Documentation**: [Project Wiki](https://github.com/HasnainMemon2001/Smart-Assistive-Cane-V1/wiki)

## 🔮 Roadmap

### Upcoming Features
- [ ] **Voice Commands**: Hands-free app navigation
- [ ] **Machine Learning**: Personalized navigation suggestions
- [ ] **Social Features**: Community waypoint sharing
- [ ] **Wearable Integration**: Smartwatch companion app
- [ ] **AR Navigation**: Augmented reality guidance (future)

### Performance Improvements
- [ ] **Offline Maps**: Cached navigation data
- [ ] **Battery Optimization**: Reduced background processing
- [ ] **Faster Startup**: Optimized app initialization
- [ ] **Memory Usage**: Reduced RAM consumption

---

**Made with ❤️ for the visually impaired community**

*Empowering independence through intelligent mobile technology*
