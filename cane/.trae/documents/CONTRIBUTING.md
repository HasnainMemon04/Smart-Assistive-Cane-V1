# Contributing to Smart Assistive Cane V1

Thank you for your interest in contributing to the Smart Assistive Cane project! This assistive technology aims to enhance mobility and safety for visually impaired individuals, and your contributions can make a real difference in people's lives.

## Table of Contents

- [Code of Conduct](#code-of-conduct)
- [Getting Started](#getting-started)
- [Development Workflow](#development-workflow)
- [Contribution Types](#contribution-types)
- [Coding Standards](#coding-standards)
- [Commit Guidelines](#commit-guidelines)
- [Pull Request Process](#pull-request-process)
- [Testing Requirements](#testing-requirements)
- [Documentation Guidelines](#documentation-guidelines)
- [Security Guidelines](#security-guidelines)
- [Accessibility Requirements](#accessibility-requirements)
- [Community Guidelines](#community-guidelines)

## Code of Conduct

This project adheres to the [Contributor Covenant Code of Conduct](CODE_OF_CONDUCT.md). By participating, you are expected to uphold this code. Please report unacceptable behavior to the project maintainers.

**Special Focus**: As an assistive technology project, we prioritize accessibility, inclusive design, and user safety in all contributions.

## Getting Started

### Prerequisites

#### For Hardware/Firmware Development
- Arduino IDE 2.0+ or PlatformIO
- ESP32 board package installed
- Required libraries (see [README.md](README.md))
- ESP32-S3-N16R8 development board
- Basic electronics knowledge

#### For Mobile App Development
- Flutter SDK 3.0+
- Android Studio or VS Code
- Android/iOS development setup
- Physical device for BLE testing

#### For Documentation
- Markdown editor
- Basic understanding of assistive technology
- Familiarity with accessibility guidelines

### Development Environment Setup

1. **Fork and Clone**
   ```bash
   git clone https://github.com/YOUR_USERNAME/smart-assistive-cane.git
   cd smart-assistive-cane
   git remote add upstream https://github.com/ORIGINAL_OWNER/smart-assistive-cane.git
   ```

2. **Hardware Setup**
   ```bash
   # Install Arduino IDE
   # Add ESP32 board package
   # Install required libraries
   # Connect ESP32-S3 development board
   ```

3. **Flutter Setup**
   ```bash
   cd APP/
   flutter pub get
   flutter doctor  # Verify setup
   ```

4. **Create Development Branch**
   ```bash
   git checkout -b feature/your-feature-name
   ```

## Development Workflow

### Branch Naming Convention

Use descriptive branch names with prefixes:

- `feature/` - New features
- `fix/` - Bug fixes
- `docs/` - Documentation updates
- `refactor/` - Code refactoring
- `test/` - Testing improvements
- `chore/` - Maintenance tasks

**Examples**:
- `feature/voice-commands`
- `fix/ble-connection-stability`
- `docs/api-reference`
- `refactor/sensor-data-structure`

### Development Process

1. **Create Issue** (for significant changes)
   - Describe the problem or enhancement
   - Include user impact assessment
   - Add appropriate labels
   - Discuss approach with maintainers

2. **Develop**
   - Write clean, documented code
   - Follow coding standards
   - Test thoroughly
   - Update documentation

3. **Test**
   - Unit tests for new functions
   - Integration tests for system changes
   - Hardware testing on actual device
   - Accessibility testing

4. **Submit Pull Request**
   - Follow PR template
   - Include detailed description
   - Reference related issues
   - Request appropriate reviewers

## Contribution Types

### 🔧 Hardware Contributions
- Circuit design improvements
- Component testing and validation
- Power optimization
- Mechanical design enhancements
- PCB layout improvements

### 💻 Software Contributions
- Firmware feature development
- Mobile app enhancements
- Algorithm improvements
- Performance optimizations
- Bug fixes and stability improvements

### 📚 Documentation
- User guides and tutorials
- API documentation
- Code comments and explanations
- Accessibility guidelines
- Translation and localization

### 🧪 Testing and Quality Assurance
- Automated testing
- Manual testing procedures
- Performance benchmarking
- Accessibility testing
- User experience testing

### 🎨 Design and User Experience
- User interface improvements
- Accessibility enhancements
- User experience research
- Visual design improvements
- Audio feedback optimization

## Coding Standards

### C++ (Arduino/ESP32)

```cpp
// File header with description
/**
 * @file SensorModule.cpp
 * @brief Brief description of the module
 * @author Your Name
 * @date 2025-01-XX
 */

// Include guards
#ifndef SENSOR_MODULE_H
#define SENSOR_MODULE_H

// Naming conventions
class SensorManager {  // PascalCase for classes
public:
    void initializeSensors();  // camelCase for functions
    bool getSensorData();      // Clear, descriptive names
    
private:
    int sensorCount_;          // Underscore suffix for private members
    bool isInitialized_;       // Boolean prefix with 'is', 'has', 'can'
};

// Constants
const int MAX_SENSORS = 10;        // UPPER_CASE for constants
const float THRESHOLD_DISTANCE = 1.5f;  // Include units in name when relevant

// Comments for complex logic
void complexAlgorithm() {
    // Explain the purpose and approach
    // Break down complex operations
    // Document any assumptions or limitations
}
```

### Dart (Flutter)

```dart
// File documentation
/// Service for managing Bluetooth Low Energy connections
/// 
/// Handles device discovery, connection management, and data streaming
/// for the Smart Assistive Cane hardware.
class BleService {
  // Private members with underscore prefix
  final StreamController<SensorData> _sensorDataController;
  
  // Public interface with clear documentation
  /// Connects to the smart cane device
  /// 
  /// Returns true if connection successful, false otherwise
  /// Throws [BleException] if Bluetooth is not available
  Future<bool> connectToDevice(String deviceId) async {
    // Implementation with error handling
  }
  
  // Use descriptive variable names
  void processSensorData(List<int> rawData) {
    final temperatureReading = extractTemperature(rawData);
    final humidityLevel = extractHumidity(rawData);
    // Process data...
  }
}
```

### General Guidelines

- **Readability**: Code should be self-documenting
- **Comments**: Explain why, not what
- **Error Handling**: Always handle potential failures
- **Resource Management**: Properly manage memory and connections
- **Performance**: Consider power consumption and response time
- **Accessibility**: Ensure code supports assistive features

## Commit Guidelines

### Conventional Commits

We follow the [Conventional Commits](https://www.conventionalcommits.org/) specification:

```
<type>[optional scope]: <description>

[optional body]

[optional footer(s)]
```

### Commit Types

- `feat`: New feature
- `fix`: Bug fix
- `docs`: Documentation changes
- `style`: Code style changes (formatting, etc.)
- `refactor`: Code refactoring
- `test`: Adding or updating tests
- `chore`: Maintenance tasks
- `perf`: Performance improvements
- `ci`: CI/CD changes
- `build`: Build system changes

### Scopes

- `firmware`: ESP32 firmware changes
- `app`: Flutter mobile app changes
- `hardware`: Hardware design changes
- `docs`: Documentation changes
- `sensors`: Sensor-related changes
- `ble`: Bluetooth communication
- `ui`: User interface changes
- `api`: API changes

### Examples

```bash
# Good commit messages
feat(sensors): add fall detection algorithm
fix(ble): resolve connection timeout issues
docs(api): update command reference table
refactor(firmware): optimize memory usage in sensor loop
test(app): add unit tests for BLE service
chore(deps): update Flutter dependencies

# Commit with body and footer
feat(sensors): implement voice command recognition

Add support for basic voice commands including:
- "Status report"
- "Emergency alert"
- "Toggle feedback mode"

Uses on-device speech recognition to preserve privacy.

Closes #123
Breaking-change: Requires microphone permission
```

### Commit Best Practices

- **Atomic commits**: One logical change per commit
- **Clear messages**: Describe what and why
- **Present tense**: "Add feature" not "Added feature"
- **Imperative mood**: "Fix bug" not "Fixes bug"
- **Line length**: Keep first line under 50 characters
- **Reference issues**: Include issue numbers when relevant

## Pull Request Process

### Before Submitting

1. **Update your branch**
   ```bash
   git fetch upstream
   git rebase upstream/main
   ```

2. **Test thoroughly**
   - Run all existing tests
   - Add tests for new functionality
   - Test on actual hardware when possible
   - Verify accessibility features

3. **Update documentation**
   - Update README if needed
   - Add/update code comments
   - Update API documentation
   - Include usage examples

### Pull Request Template

When creating a PR, include:

```markdown
## Description
Brief description of changes and motivation.

## Type of Change
- [ ] Bug fix (non-breaking change)
- [ ] New feature (non-breaking change)
- [ ] Breaking change (fix or feature that would cause existing functionality to not work as expected)
- [ ] Documentation update
- [ ] Performance improvement
- [ ] Code refactoring

## Testing
- [ ] Unit tests pass
- [ ] Integration tests pass
- [ ] Hardware testing completed
- [ ] Accessibility testing completed
- [ ] Manual testing performed

## Hardware Impact
- [ ] No hardware changes required
- [ ] Requires firmware update
- [ ] Requires hardware modification
- [ ] Changes power consumption

## User Impact
- [ ] No user-facing changes
- [ ] Improves existing functionality
- [ ] Adds new functionality
- [ ] Changes user interface
- [ ] Requires user action/setup

## Accessibility
- [ ] No accessibility impact
- [ ] Improves accessibility
- [ ] Maintains existing accessibility
- [ ] Requires accessibility review

## Checklist
- [ ] Code follows project style guidelines
- [ ] Self-review completed
- [ ] Code is commented appropriately
- [ ] Documentation updated
- [ ] Tests added/updated
- [ ] No secrets or sensitive data included
- [ ] Backwards compatibility considered
```

### Review Process

1. **Automated Checks**
   - Code style validation
   - Automated testing
   - Security scanning
   - Documentation checks

2. **Peer Review**
   - Code quality assessment
   - Architecture review
   - Accessibility evaluation
   - User impact analysis

3. **Maintainer Review**
   - Final approval
   - Merge coordination
   - Release planning

## Testing Requirements

### Firmware Testing

```cpp
// Unit test example
void testDistanceSensor() {
    // Setup
    ToF sensor;
    sensor.initialize();
    
    // Test
    float distance = sensor.readDistance();
    
    // Assert
    assert(distance >= 0.04 && distance <= 4.0);  // Valid range
    assert(sensor.getStatus() == SENSOR_OK);
}

// Integration test
void testSensorFusion() {
    // Test multiple sensors working together
    // Verify data consistency
    // Check timing requirements
}
```

### Mobile App Testing

```dart
// Widget test example
testWidgets('Dashboard displays sensor data', (WidgetTester tester) async {
  // Setup mock data
  final mockBleService = MockBleService();
  when(mockBleService.sensorDataStream)
      .thenAnswer((_) => Stream.value(mockSensorData));
  
  // Build widget
  await tester.pumpWidget(DashboardScreen());
  
  // Verify UI elements
  expect(find.text('Temperature: 25.0°C'), findsOneWidget);
  expect(find.text('Distance: 1.5m'), findsOneWidget);
});
```

### Hardware Testing

- **Component Testing**: Individual sensor validation
- **Integration Testing**: Full system operation
- **Performance Testing**: Power consumption, response time
- **Stress Testing**: Extended operation, edge cases
- **Environmental Testing**: Temperature, humidity variations

## Documentation Guidelines

### Code Documentation

```cpp
/**
 * @brief Calculates obstacle avoidance direction
 * 
 * Uses ToF sensor data and IMU orientation to determine
 * the safest direction for navigation assistance.
 * 
 * @param distance Current obstacle distance in meters
 * @param orientation Current device orientation in degrees
 * @return Recommended direction (-1: left, 0: stop, 1: right)
 * 
 * @note This function assumes the sensor is properly calibrated
 * @warning Returns 0 (stop) if distance < SAFETY_THRESHOLD
 */
int calculateAvoidanceDirection(float distance, float orientation);
```

### User Documentation

- **Clear language**: Avoid technical jargon
- **Step-by-step instructions**: Break down complex procedures
- **Visual aids**: Include diagrams and screenshots
- **Accessibility**: Provide alternative text for images
- **Examples**: Include practical usage scenarios

### API Documentation

- **Complete parameter descriptions**
- **Return value explanations**
- **Error conditions**
- **Usage examples**
- **Version compatibility notes**

## Security Guidelines

### Secrets Management

**❌ NEVER commit secrets to the repository**

```cpp
// BAD - Don't do this
#define WIFI_PASSWORD "mypassword123"
#define API_KEY "sk-1234567890abcdef"

// GOOD - Use configuration files
#include "arduino_secrets.h"  // Not tracked in git
#define WIFI_PASSWORD SECRET_WIFI_PASS
#define API_KEY SECRET_API_KEY
```

### Security Best Practices

1. **Use environment variables** for sensitive configuration
2. **Validate all inputs** from external sources
3. **Encrypt sensitive data** in storage and transmission
4. **Follow principle of least privilege**
5. **Keep dependencies updated**
6. **Report security issues privately**

### Reporting Security Issues

For security vulnerabilities:
1. **DO NOT** create public issues
2. Email security@project-domain.com
3. Include detailed description
4. Provide reproduction steps
5. Suggest mitigation if possible

## Accessibility Requirements

### Design Principles

1. **Perceivable**: Information must be presentable in ways users can perceive
2. **Operable**: Interface components must be operable
3. **Understandable**: Information and UI operation must be understandable
4. **Robust**: Content must be robust enough for various assistive technologies

### Implementation Guidelines

#### Audio Feedback
- Clear, concise voice messages
- Adjustable volume and speech rate
- Multiple language support
- Non-speech audio alternatives

#### Haptic Feedback
- Distinct vibration patterns
- Adjustable intensity
- Meaningful tactile information
- Battery-efficient implementation

#### Mobile App Accessibility
- Screen reader compatibility
- High contrast mode support
- Large text options
- Voice control integration
- Gesture alternatives

### Testing Accessibility

- **Screen readers**: Test with TalkBack (Android) and VoiceOver (iOS)
- **Voice control**: Verify voice command functionality
- **High contrast**: Test in high contrast modes
- **Large text**: Verify layout with increased text sizes
- **User testing**: Include users with disabilities in testing

## Community Guidelines

### Communication Channels

- **GitHub Issues**: Bug reports, feature requests
- **GitHub Discussions**: General questions, ideas
- **Pull Requests**: Code review and discussion
- **Email**: Security issues, private matters

### Getting Help

1. **Search existing issues** before creating new ones
2. **Use issue templates** for bug reports and features
3. **Provide detailed information** including:
   - Hardware configuration
   - Software versions
   - Steps to reproduce
   - Expected vs actual behavior
   - Logs and error messages

### Mentorship

We welcome new contributors! If you're new to:
- **Hardware development**: We can help with circuit design and component selection
- **Firmware programming**: Guidance on ESP32 development and sensor integration
- **Mobile development**: Flutter and BLE communication assistance
- **Accessibility**: Best practices for assistive technology

### Recognition

Contributors are recognized through:
- **Contributor list** in README
- **Release notes** acknowledgments
- **Special recognition** for significant contributions
- **Mentorship opportunities** for experienced contributors

## Priority Areas

We especially welcome contributions in:

### High Priority
- 🔋 **Power optimization**: Extending battery life
- 🧠 **Algorithm improvements**: Better fall detection, obstacle classification
- 📱 **Mobile app features**: Enhanced user interface, new functionality
- 🔊 **Audio system**: Voice recognition, better feedback
- 🌐 **Accessibility**: Screen reader support, voice control

### Medium Priority
- 📊 **Data analytics**: Usage patterns, health insights
- 🔗 **Integration**: Smart home, wearable devices
- 🌍 **Localization**: Multi-language support
- 🧪 **Testing**: Automated testing, CI/CD improvements
- 📚 **Documentation**: Tutorials, API docs

### Research Areas
- 🤖 **Machine learning**: Advanced obstacle classification
- 👁️ **Computer vision**: Camera-based navigation
- 🏠 **Indoor positioning**: Improved room detection
- ☁️ **Cloud services**: Data synchronization, remote monitoring

## Questions?

If you have questions about contributing:

1. Check the [FAQ](docs/FAQ.md)
2. Search [existing discussions](https://github.com/project/discussions)
3. Create a new discussion
4. Contact maintainers directly

Thank you for contributing to making technology more accessible and improving lives through innovation!

---

*This contributing guide is a living document. Please suggest improvements through issues or pull requests.*

*Last updated: January 2025*