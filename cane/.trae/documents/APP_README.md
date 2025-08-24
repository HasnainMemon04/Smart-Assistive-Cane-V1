# Smart Assistive Cane Mobile App

> Flutter-based companion app for the Smart Assistive Cane V1 hardware system

## Overview

The Smart Assistive Cane mobile app is a cross-platform Flutter application that provides real-time monitoring, control, and data analysis for the Smart Assistive Cane hardware. The app connects via Bluetooth Low Energy (BLE) to stream sensor data, configure settings, and provide caregiver monitoring capabilities.

## Features

### 📊 Real-Time Dashboard
- Live sensor data visualization
- Connection status monitoring
- Quick action controls
- Emergency alert button
- Battery level indicator

### 📈 Data Analytics
- Historical sensor data tracking
- Activity pattern analysis
- Health metrics visualization
- Progress tracking charts
- Custom date range reports

### 🔧 Device Control
- Sensor configuration
- Feedback mode settings
- Audio volume control
- Vibration intensity adjustment
- Operation mode switching

### 👥 Caregiver Features
- Remote monitoring dashboard
- Location tracking and sharing
- Emergency alert notifications
- Health status summaries
- Activity reports

### 🛠️ Debug & Maintenance
- Serial monitor interface
- Command execution
- Sensor health monitoring
- Diagnostic information
- Log file management

## Screenshots

*Note: Screenshots will be added as the UI is finalized*

## Installation

### Prerequisites

- **Android**: Version 6.0+ (API level 23+)
- **iOS**: Version 12.0+
- **Bluetooth**: BLE 4.0+ support
- **Permissions**: Location, Bluetooth, Microphone, Notifications

### Download Options

#### Option 1: Pre-built APK (Android)
```bash
# Download from releases page
# Enable "Install from unknown sources"
# Install the APK file
```

#### Option 2: Build from Source
```bash
# Clone repository
git clone <repository-url>
cd smart-assistive-cane/APP

# Install dependencies
flutter pub get

# Build for Android
flutter build apk --release

# Build for iOS (requires Xcode)
flutter build ios --release
```

### Development Setup

1. **Install Flutter SDK**
   ```bash
   # Follow official Flutter installation guide
   # https://flutter.dev/docs/get-started/install
   flutter doctor  # Verify installation
   ```

2. **Setup IDE**
   - **VS Code**: Install Flutter and Dart extensions
   - **Android Studio**: Install Flutter plugin
   - **Xcode**: Required for iOS development

3. **Configure Development Environment**
   ```bash
   cd APP/
   flutter pub get
   flutter run  # Run in debug mode
   ```

## App Architecture

### Project Structure
```
lib/
├── main.dart                    # App entry point
├── services/                    # Business logic layer
│   ├── ble_service.dart        # Bluetooth communication
│   ├── storage_service.dart    # Local data persistence
│   ├── prefs_service.dart      # User preferences
│   ├── command_service.dart    # Serial command interface
│   └── notification_service.dart # Push notifications
├── screens/                     # UI screens
│   ├── dashboard_screen.dart   # Main dashboard
│   ├── history_screen.dart     # Data history
│   ├── serial_monitor_screen.dart # Debug interface
│   ├── caregiver_dashboard.dart # Remote monitoring
│   ├── analytics_dashboard.dart # Data analytics
│   └── settings_screen.dart    # Configuration
├── widgets/                     # Reusable UI components
│   ├── sensor_cards/          # Sensor data displays
│   ├── charts/                # Data visualization
│   ├── common/                # Shared widgets
│   └── dialogs/               # Modal dialogs
├── providers/                   # State management
│   ├── providers.dart         # Riverpod providers
│   ├── ble_provider.dart      # BLE state management
│   └── settings_provider.dart # App settings
├── models/                      # Data models
│   ├── sensor_data.dart       # Sensor data structures
│   ├── device_info.dart       # Device information
│   └── user_settings.dart     # User preferences
└── utils/                       # Utility functions
    ├── constants.dart          # App constants
    ├── helpers.dart           # Helper functions
    └── formatters.dart        # Data formatters
```

### State Management

The app uses **Riverpod** for state management:

```dart
// Provider example
final bleServiceProvider = Provider<BleService>((ref) {
  return BleService();
});

final sensorDataProvider = StreamProvider<SensorData>((ref) {
  final bleService = ref.watch(bleServiceProvider);
  return bleService.sensorDataStream;
});

// Widget usage
class DashboardScreen extends ConsumerWidget {
  @override
  Widget build(BuildContext context, WidgetRef ref) {
    final sensorData = ref.watch(sensorDataProvider);
    
    return sensorData.when(
      data: (data) => SensorDataDisplay(data: data),
      loading: () => CircularProgressIndicator(),
      error: (error, stack) => ErrorWidget(error),
    );
  }
}
```

## BLE Communication

### Connection Process

1. **Device Discovery**
   ```dart
   // Scan for devices
   await FlutterBluePlus.startScan(
     withServices: [serviceUUID],
     timeout: Duration(seconds: 10),
   );
   ```

2. **Connection Establishment**
   ```dart
   // Connect to device
   await device.connect();
   
   // Discover services
   List<BluetoothService> services = await device.discoverServices();
   ```

3. **Data Streaming**
   ```dart
   // Subscribe to characteristics
   await characteristic.setNotifyValue(true);
   characteristic.value.listen((value) {
     // Process incoming data
     processSensorData(value);
   });
   ```

### BLE Service UUIDs

```dart
// Service UUIDs
static const String MAIN_SERVICE_UUID = "12345678-1234-1234-1234-123456789abc";
static const String SENSOR_SERVICE_UUID = "87654321-4321-4321-4321-cba987654321";

// Characteristic UUIDs
static const String TEMPERATURE_CHAR_UUID = "temp-1234-5678-9abc-def012345678";
static const String HUMIDITY_CHAR_UUID = "humi-1234-5678-9abc-def012345678";
static const String DISTANCE_CHAR_UUID = "dist-1234-5678-9abc-def012345678";
static const String IMU_CHAR_UUID = "imu-1234-5678-9abc-def012345678";
static const String GPS_CHAR_UUID = "gps-1234-5678-9abc-def012345678";
static const String COMMAND_CHAR_UUID = "cmd-1234-5678-9abc-def012345678";
```

### Data Packet Structure

```dart
class SensorData {
  final double temperature;     // °C
  final double humidity;        // %RH
  final double distance;        // meters
  final IMUData imuData;        // acceleration, gyroscope
  final GPSData gpsData;        // latitude, longitude, speed
  final int stepCount;          // steps
  final double batteryLevel;    // percentage
  final DateTime timestamp;     // data timestamp
  
  // Serialize/deserialize methods
  factory SensorData.fromBytes(List<int> bytes) {
    // Parse binary data from BLE
  }
  
  List<int> toBytes() {
    // Convert to binary for transmission
  }
}
```

## Screen Descriptions

### 1. Dashboard Screen

**Purpose**: Main interface showing real-time sensor data

**Features**:
- Live sensor readings with visual indicators
- Connection status and signal strength
- Quick action buttons (emergency, settings)
- Battery level and charging status
- Last update timestamp

**Key Widgets**:
```dart
// Sensor data cards
TemperatureCard(temperature: sensorData.temperature)
DistanceCard(distance: sensorData.distance)
BatteryCard(level: sensorData.batteryLevel)

// Action buttons
EmergencyButton(onPressed: () => sendEmergencyAlert())
SettingsButton(onPressed: () => navigateToSettings())
```

### 2. History Screen

**Purpose**: Historical data visualization and analysis

**Features**:
- Time-series charts for all sensors
- Date range selection
- Data export functionality
- Activity summaries
- Trend analysis

**Chart Types**:
- Line charts for continuous data (temperature, distance)
- Bar charts for discrete data (steps, activities)
- Heatmaps for pattern visualization
- Pie charts for categorical data

### 3. Serial Monitor Screen

**Purpose**: Direct communication with hardware for debugging

**Features**:
- Command input field
- Real-time log display
- Command history
- Auto-scroll toggle
- Log export functionality

**Usage Example**:
```dart
// Send command
commandService.sendCommand("tof_read");

// Receive response
"ToF Distance: 1.25m\nStatus: OK\nMode: Simple"
```

### 4. Caregiver Dashboard

**Purpose**: Remote monitoring for caregivers and family

**Features**:
- Real-time location tracking
- Health status overview
- Emergency alert history
- Activity patterns
- Contact management

**Notifications**:
- Fall detection alerts
- Low battery warnings
- Unusual activity patterns
- Device disconnection alerts

### 5. Analytics Dashboard

**Purpose**: Long-term data analysis and insights

**Features**:
- Weekly/monthly activity reports
- Health trend analysis
- Goal tracking and progress
- Comparative analysis
- Custom report generation

### 6. Settings Screen

**Purpose**: App and device configuration

**Categories**:
- **Device Settings**: Sensor thresholds, feedback modes
- **App Preferences**: Theme, notifications, language
- **Privacy**: Data sharing, location permissions
- **Account**: User profile, caregiver contacts
- **About**: Version info, support, licenses

## Permissions

### Required Permissions

#### Android (android/app/src/main/AndroidManifest.xml)
```xml
<!-- Bluetooth permissions -->
<uses-permission android:name="android.permission.BLUETOOTH" />
<uses-permission android:name="android.permission.BLUETOOTH_ADMIN" />
<uses-permission android:name="android.permission.BLUETOOTH_CONNECT" />
<uses-permission android:name="android.permission.BLUETOOTH_SCAN" />

<!-- Location permissions (required for BLE scanning) -->
<uses-permission android:name="android.permission.ACCESS_COARSE_LOCATION" />
<uses-permission android:name="android.permission.ACCESS_FINE_LOCATION" />

<!-- Notification permissions -->
<uses-permission android:name="android.permission.VIBRATE" />
<uses-permission android:name="android.permission.WAKE_LOCK" />

<!-- Storage permissions -->
<uses-permission android:name="android.permission.WRITE_EXTERNAL_STORAGE" />
<uses-permission android:name="android.permission.READ_EXTERNAL_STORAGE" />

<!-- Microphone (for voice commands) -->
<uses-permission android:name="android.permission.RECORD_AUDIO" />
```

#### iOS (ios/Runner/Info.plist)
```xml
<!-- Bluetooth usage description -->
<key>NSBluetoothAlwaysUsageDescription</key>
<string>This app uses Bluetooth to connect to the Smart Assistive Cane device</string>

<!-- Location usage description -->
<key>NSLocationWhenInUseUsageDescription</key>
<string>Location access is required for Bluetooth device scanning</string>

<!-- Microphone usage description -->
<key>NSMicrophoneUsageDescription</key>
<string>Microphone access is required for voice commands</string>
```

### Permission Handling

```dart
// Request permissions at app startup
Future<void> requestPermissions() async {
  // Bluetooth permissions
  await Permission.bluetooth.request();
  await Permission.bluetoothScan.request();
  await Permission.bluetoothConnect.request();
  
  // Location permissions
  await Permission.locationWhenInUse.request();
  
  // Microphone permissions
  await Permission.microphone.request();
  
  // Notification permissions
  await Permission.notification.request();
}
```

## Configuration

### App Configuration

```dart
// lib/utils/constants.dart
class AppConstants {
  // BLE Configuration
  static const Duration scanTimeout = Duration(seconds: 10);
  static const Duration connectionTimeout = Duration(seconds: 15);
  static const int maxReconnectAttempts = 3;
  
  // Data Configuration
  static const int maxHistoryDays = 30;
  static const Duration dataUpdateInterval = Duration(seconds: 1);
  
  // UI Configuration
  static const double chartAnimationDuration = 1.0;
  static const int maxLogLines = 1000;
}
```

### Build Configuration

```yaml
# pubspec.yaml
name: smartassistivecanev1
description: Smart Assistive Cane companion app
version: 1.0.0+1

environment:
  sdk: '>=3.0.0 <4.0.0'
  flutter: '>=3.0.0'

dependencies:
  flutter:
    sdk: flutter
  
  # State Management
  flutter_riverpod: ^2.4.9
  
  # Bluetooth
  flutter_blue_plus: ^1.14.6
  
  # Storage
  shared_preferences: ^2.2.2
  hive: ^2.2.3
  hive_flutter: ^1.1.0
  
  # Charts and Visualization
  fl_chart: ^0.65.0
  sleek_circular_slider: ^2.0.1
  
  # Audio and Notifications
  flutter_tts: ^3.8.5
  flutter_local_notifications: ^16.3.0
  
  # Permissions
  permission_handler: ^11.1.0
  
  # Other utilities
  google_fonts: ^6.1.0
  url_launcher: ^6.2.2
  file_picker: ^6.1.1
  path_provider: ^2.1.1
```

## Troubleshooting

### Common Issues

#### BLE Connection Problems

**Problem**: App cannot find the cane device
- **Solution**: Ensure Bluetooth is enabled and permissions granted
- **Debug**: Check if device is advertising and in range

**Problem**: Connection drops frequently
- **Solution**: Check signal strength and interference
- **Debug**: Monitor connection logs in serial monitor

**Problem**: Data not updating
- **Solution**: Verify BLE characteristics are properly subscribed
- **Debug**: Check data packet format and parsing

#### App Performance Issues

**Problem**: App is slow or unresponsive
- **Solution**: Reduce data update frequency, optimize UI rendering
- **Debug**: Use Flutter DevTools for performance profiling

**Problem**: High battery consumption
- **Solution**: Implement background processing limits
- **Debug**: Monitor CPU and network usage

#### Permission Issues

**Problem**: Location permission denied
- **Solution**: Explain why location is needed for BLE scanning
- **Debug**: Check permission status in app settings

**Problem**: Bluetooth permission denied
- **Solution**: Guide user to app settings to enable permissions
- **Debug**: Verify all required permissions are declared

### Debug Tools

1. **Flutter DevTools**
   ```bash
   flutter pub global activate devtools
   flutter pub global run devtools
   ```

2. **BLE Scanner Apps**
   - Use third-party BLE scanner to verify device advertising
   - Check service and characteristic UUIDs

3. **Logging**
   ```dart
   // Enable debug logging
   import 'package:flutter/foundation.dart';
   
   void debugLog(String message) {
     if (kDebugMode) {
       print('[DEBUG] $message');
     }
   }
   ```

## Testing

### Unit Testing

```dart
// test/services/ble_service_test.dart
import 'package:flutter_test/flutter_test.dart';
import 'package:mockito/mockito.dart';

void main() {
  group('BleService Tests', () {
    late BleService bleService;
    late MockBluetoothDevice mockDevice;
    
    setUp(() {
      bleService = BleService();
      mockDevice = MockBluetoothDevice();
    });
    
    test('should parse sensor data correctly', () {
      // Test data parsing logic
      final testData = [0x01, 0x02, 0x03, 0x04];
      final result = bleService.parseSensorData(testData);
      
      expect(result.temperature, equals(25.0));
      expect(result.humidity, equals(60.0));
    });
  });
}
```

### Widget Testing

```dart
// test/widgets/dashboard_test.dart
import 'package:flutter/material.dart';
import 'package:flutter_test/flutter_test.dart';

void main() {
  testWidgets('Dashboard displays sensor data', (WidgetTester tester) async {
    // Build the widget
    await tester.pumpWidget(
      MaterialApp(
        home: DashboardScreen(),
      ),
    );
    
    // Verify UI elements
    expect(find.text('Temperature'), findsOneWidget);
    expect(find.text('Distance'), findsOneWidget);
    expect(find.byType(CircularProgressIndicator), findsNothing);
  });
}
```

### Integration Testing

```dart
// integration_test/app_test.dart
import 'package:flutter/material.dart';
import 'package:flutter_test/flutter_test.dart';
import 'package:integration_test/integration_test.dart';

void main() {
  IntegrationTestWidgetsFlutterBinding.ensureInitialized();
  
  group('App Integration Tests', () {
    testWidgets('Complete user flow', (WidgetTester tester) async {
      // Launch app
      await tester.pumpWidget(MyApp());
      
      // Test navigation
      await tester.tap(find.text('History'));
      await tester.pumpAndSettle();
      
      // Verify screen transition
      expect(find.text('Data History'), findsOneWidget);
    });
  });
}
```

## Deployment

### Android Deployment

1. **Build Release APK**
   ```bash
   flutter build apk --release
   ```

2. **Build App Bundle** (for Play Store)
   ```bash
   flutter build appbundle --release
   ```

3. **Signing Configuration**
   ```properties
   # android/key.properties
   storePassword=your-store-password
   keyPassword=your-key-password
   keyAlias=your-key-alias
   storeFile=path/to/keystore.jks
   ```

### iOS Deployment

1. **Build for iOS**
   ```bash
   flutter build ios --release
   ```

2. **Archive in Xcode**
   - Open `ios/Runner.xcworkspace` in Xcode
   - Select "Any iOS Device" as target
   - Product → Archive
   - Upload to App Store Connect

## Contributing

See the main [CONTRIBUTING.md](../CONTRIBUTING.md) for general guidelines.

### App-Specific Guidelines

1. **UI/UX Considerations**
   - Follow Material Design guidelines
   - Ensure accessibility compliance
   - Test on various screen sizes
   - Consider users with visual impairments

2. **Performance**
   - Minimize BLE data processing on UI thread
   - Use efficient data structures
   - Implement proper error handling
   - Test on low-end devices

3. **Testing**
   - Write unit tests for business logic
   - Create widget tests for UI components
   - Test BLE communication thoroughly
   - Verify accessibility features

## Support

For app-specific issues:

1. **Check existing issues** in the GitHub repository
2. **Create detailed bug reports** with:
   - Device model and OS version
   - App version
   - Steps to reproduce
   - Screenshots or videos
   - Log files if available

3. **Feature requests** should include:
   - Use case description
   - Expected behavior
   - Mockups or wireframes (if applicable)
   - Accessibility considerations

## License

This project is licensed under the MIT License - see the [LICENSE](../LICENSE) file for details.

---

**Smart Assistive Cane Mobile App** - Connecting technology with accessibility

*Last updated: January 2025*