# Smart Assistive Cane V1 - Circuit Diagram & Hardware Reference

> Comprehensive hardware reference guide for the Smart Assistive Cane V1, including detailed circuit diagrams, component specifications, assembly instructions, and troubleshooting procedures.

## Table of Contents

- [System Overview](#system-overview)
- [Hardware Architecture](#hardware-architecture)
- [Component Specifications](#component-specifications)
- [Pin Assignment Reference](#pin-assignment-reference)
- [Circuit Diagrams](#circuit-diagrams)
- [Power Management](#power-management)
- [PCB Design Guidelines](#pcb-design-guidelines)
- [Assembly Instructions](#assembly-instructions)
- [Testing & Validation](#testing--validation)
- [Troubleshooting](#troubleshooting)
- [Safety Considerations](#safety-considerations)

## System Overview

The Smart Assistive Cane V1 hardware platform is built around the ESP32-S3-N16R8 microcontroller, integrating multiple sensor systems, feedback mechanisms, and communication interfaces to provide comprehensive assistive functionality.

### Key Hardware Features
- **Processing**: Dual-core Xtensa LX7 @ 240MHz
- **Memory**: 512KB SRAM + 8MB PSRAM + 16MB Flash
- **Connectivity**: WiFi 802.11 b/g/n, Bluetooth 5.0/BLE
- **Sensors**: 6 integrated sensor modules
- **Feedback**: Multi-modal output system
- **Power**: Optimized for battery operation

## Hardware Architecture

### Core Processing Unit

**ESP32-S3-N16R8 DevKit Specifications:**
```
Processor:     Dual-core Xtensa LX7 @ 240MHz
RAM:          512KB SRAM + 8MB PSRAM
Flash:        16MB
GPIO Pins:    45 programmable pins
Interfaces:   I2C, SPI, UART, I2S, PWM, ADC
Connectivity: WiFi 802.11 b/g/n, Bluetooth 5.0, BLE
Operating:    3.3V, -40°C to +85°C
```

### System Block Diagram

```
┌─────────────────────────────────────────────────────────────────┐
│                        ESP32-S3-N16R8                          │
├─────────────────────────────────────────────────────────────────┤
│  I2C Bus (GPIO8/9)     │  SPI Bus (GPIO10-14)  │  UART (17/18) │
│  ┌─────────────────┐   │  ┌─────────────────┐   │  ┌──────────┐ │
│  │ VL53L1X ToF     │   │  │ MFRC522 RFID    │   │  │ NEO-6M   │ │
│  │ MPU6050 IMU     │   │  │ SD Card Slot    │   │  │ GPS      │ │
│  │ BH1750 Light    │   │  └─────────────────┘   │  └──────────┘ │
│  └─────────────────┘   │                        │               │
├─────────────────────────┼────────────────────────┼───────────────┤
│  I2S Audio (GPIO4-6)   │  PWM Feedback (7,15,16,48) │ Digital I/O │
│  ┌─────────────────┐   │  ┌─────────────────┐   │  ┌──────────┐ │
│  │ MAX98357A       │   │  │ Vibration Motors│   │  │ Buttons  │ │
│  │ Audio Amp       │   │  │ Piezo Buzzer    │   │  │ LEDs     │ │
│  └─────────────────┘   │  │ Servo Motor     │   │  │ DHT22    │ │
│                        │  └─────────────────┘   │  └──────────┘ │
└─────────────────────────────────────────────────────────────────┘
```

## Component Specifications

### Sensor Array

#### VL53L1X Time-of-Flight Sensor
```
Function:     Obstacle detection and ranging
Range:        4cm to 4m
Accuracy:     ±3mm (typical)
FOV:          27° diagonal
Interface:    I2C (0x29)
Power:        2.6V-3.5V, 18mA active
Response:     20ms typical
```

#### MPU6050 Inertial Measurement Unit
```
Function:     Motion detection, step counting, fall detection
Gyroscope:    ±250, ±500, ±1000, ±2000°/sec
Accelerometer: ±2g, ±4g, ±8g, ±16g
Interface:    I2C (0x68)
Power:        2.375V-3.46V, 3.9mA
Sample Rate:  Up to 8kHz
```

#### DHT22 Temperature/Humidity Sensor
```
Function:     Environmental monitoring
Temperature:  -40°C to +80°C (±0.5°C)
Humidity:     0-100% RH (±2-5%)
Interface:    Single-wire digital
Power:        3.3V-6V, 1.5mA
Sample Rate:  0.5Hz (2 second intervals)
```

#### BH1750 Light Sensor
```
Function:     Ambient light detection
Range:        1-65535 lux
Accuracy:     ±20%
Interface:    I2C (0x23)
Power:        2.4V-3.6V, 0.12mA
Response:     120ms
```

#### MFRC522 RFID Reader
```
Function:     Indoor navigation and identification
Frequency:    13.56MHz
Protocol:     ISO14443A/MIFARE
Range:        0-5cm
Interface:    SPI
Power:        2.5V-3.3V, 13-26mA
```

#### NEO-6M GPS Module
```
Function:     Position tracking and navigation
Channels:     50 channels
Accuracy:     2.5m CEP
Sensitivity:  -161dBm
Interface:    UART (9600 baud)
Power:        2.7V-3.6V, 25mA
```

### Feedback Systems

#### MAX98357A I2S Audio Amplifier
```
Function:     Audio feedback and voice prompts
Output Power: 3.2W @ 4Ω, 1.8W @ 8Ω
SNR:          92dB
Interface:    I2S
Power:        2.5V-5.5V
Gain:         9dB, 12dB, 15dB, 18dB (selectable)
```

#### Vibration Motors (2x)
```
Function:     Haptic feedback
Type:         Coin vibration motor
Voltage:      3V nominal
Current:      60-80mA
Frequency:    150-200Hz
Control:      PWM via transistor driver
```

#### Piezo Buzzer
```
Function:     Audio alerts
Frequency:    2-4kHz
Sound Level:  85dB @ 10cm
Power:        3.3V, 30mA
Control:      PWM modulation
```

#### SG90 Servo Motor (Radar Mode)
```
Function:     ToF sensor rotation for radar scanning
Torque:       1.8kg⋅cm
Speed:        0.1s/60°
Angle:        180° (±90°)
Control:      PWM (50Hz, 1-2ms pulse)
Power:        4.8V-6V, 100-200mA
```

## Pin Assignment Reference

### Complete Pin Mapping

| GPIO | Function | Interface | Component | Notes |
|------|----------|-----------|-----------|-------|
| **I2C Bus** |
| 8 | SDA | I2C | Sensor Bus | 4.7kΩ pull-up to 3.3V |
| 9 | SCL | I2C | Sensor Bus | 4.7kΩ pull-up to 3.3V |
| **SPI Bus - RFID** |
| 10 | CS | SPI | MFRC522 | Chip Select |
| 11 | RST | Digital | MFRC522 | Reset pin |
| 12 | MISO | SPI | MFRC522 | Master In Slave Out |
| 13 | MOSI | SPI | MFRC522 | Master Out Slave In |
| 14 | SCK | SPI | MFRC522 | Serial Clock |
| **SPI Bus - SD Card** |
| 34 | CS | SPI | SD Card | Chip Select |
| 35 | MISO | SPI | SD Card | Master In Slave Out |
| 36 | MOSI | SPI | SD Card | Master Out Slave In |
| 37 | SCK | SPI | SD Card | Serial Clock |
| **UART - GPS** |
| 17 | RX | UART | NEO-6M | GPS data input |
| 18 | TX | UART | NEO-6M | GPS commands output |
| **I2S Audio** |
| 4 | BCLK | I2S | MAX98357A | Bit Clock |
| 5 | LRC | I2S | MAX98357A | Left/Right Clock |
| 6 | DIN | I2S | MAX98357A | Data Input |
| **Feedback Systems** |
| 7 | PWM | Digital | Servo Motor | Control signal |
| 15 | PWM | Digital | Vibration Motor 1 | Via transistor driver |
| 16 | PWM | Digital | Vibration Motor 2 | Via transistor driver |
| 48 | PWM | Digital | Piezo Buzzer | Audio alerts |
| **User Interface** |
| 1 | Input | Digital | Button 1 | 10kΩ pull-up |
| 2 | Input | Digital | Button 2 | 10kΩ pull-up |
| 3 | Input | Digital | DHT22 | Temperature/Humidity |
| **Status & Control** |
| 21 | Input | Digital | MPU6050 INT | Interrupt pin |
| 38 | Output | Digital | Status LED 1 | 220Ω current limit |
| 39 | Output | Digital | Status LED 2 | 220Ω current limit |

### I2C Device Addresses

| Device | Address | Alternative | Function |
|--------|---------|-------------|----------|
| VL53L1X | 0x29 | - | Time-of-Flight sensor |
| MPU6050 | 0x68 | 0x69 | IMU (AD0 pin controls) |
| BH1750 | 0x23 | 0x5C | Light sensor (ADDR pin) |

## Circuit Diagrams

### Power Distribution Circuit

```
Battery Input (5V-12V)
    │
    ├── AMS1117-3.3 Regulator ──┬── ESP32-S3 (3.3V)
    │   (1A capacity)           ├── Sensor Array (3.3V)
    │                           └── Logic Circuits (3.3V)
    │
    ├── AMS1117-5.0 Regulator ──┬── Servo Motor (5V)
    │   (1A capacity)           └── Audio Amplifier (5V)
    │
    └── Direct Connection ──────┬── Vibration Motors (Variable)
        (via PWM control)       └── Status LEDs (3.3V via resistors)

Decoupling Capacitors:
- 100µF electrolytic on input
- 10µF tantalum on each regulator output
- 100nF ceramic near each IC
```

### I2C Bus Circuit

```
3.3V ──┬── 4.7kΩ ──┬── SDA (GPIO8) ──┬── VL53L1X
       │            │                ├── MPU6050
       └── 4.7kΩ ──┬── SCL (GPIO9) ──┼── BH1750
                    │                └── (Future I2C devices)
                    │
                    └── 100nF ── GND (noise filtering)

Note: Pull-up resistors are critical for I2C operation
Total bus capacitance should not exceed 400pF
```

### SPI Bus Circuit (RFID)

```
ESP32-S3          MFRC522
────────          ───────
GPIO13 (MOSI) ──── MOSI
GPIO12 (MISO) ──── MISO
GPIO14 (SCK)  ──── SCK
GPIO10 (CS)   ──── SDA/SS
GPIO11        ──── RST
3.3V          ──── VCC
GND           ──── GND

Decoupling: 100nF ceramic capacitor between VCC and GND
Antenna: PCB trace antenna or external 13.56MHz antenna
```

### Audio System Circuit

```
ESP32-S3          MAX98357A         Speaker
────────          ─────────         ───────
GPIO4 (BCLK)  ──── BCLK
GPIO5 (LRC)   ──── LRC
GPIO6 (DIN)   ──── DIN
5V            ──── VDD              
GND           ──── GND              
                   OUT+ ──────────── Speaker +
                   OUT- ──────────── Speaker -

Speaker: 4Ω or 8Ω, 3W minimum power rating
Gain setting: Connect GAIN pins for desired output level
```

### Feedback Motor Circuit

```
For each vibration motor:

ESP32-S3                    Motor
────────                    ─────
GPIO15/16 ──── 1kΩ ──── Base (2N2222 NPN)
                        │
3.3V ────────────────── Collector
                        │
                        └── Motor + terminal
                        
Motor - terminal ──── GND

Flyback diode: 1N4148 across motor terminals
Base resistor: 1kΩ for current limiting
Transistor: 2N2222 or equivalent NPN
```

## Power Management

### Power Supply Requirements

| Component | Voltage | Current (Active) | Current (Sleep) |
|-----------|---------|------------------|------------------|
| ESP32-S3 | 3.3V | 45mA | 10µA |
| VL53L1X | 3.3V | 18mA | 5µA |
| MPU6050 | 3.3V | 3.9mA | 8µA |
| DHT22 | 3.3V | 1.5mA | 50µA |
| BH1750 | 3.3V | 0.12mA | 0.01µA |
| MFRC522 | 3.3V | 13mA | 10µA |
| GPS Module | 3.3V | 25mA | 1mA |
| Audio Amp | 5V | 100mA | 1mA |
| Servo Motor | 5V | 150mA | 0mA |
| Vibration Motors | 3.3V | 120mA | 0mA |
| **Total System** | - | **476mA** | **1.1mA** |

### Battery Life Calculations

**With 2000mAh Li-Po Battery:**
- Continuous operation: 4.2 hours
- 50% duty cycle: 8.4 hours
- With sleep mode optimization: 30+ hours

**With 5000mAh Li-Po Battery:**
- Continuous operation: 10.5 hours
- 50% duty cycle: 21 hours
- With sleep mode optimization: 75+ hours

### Power Management Features

1. **Deep Sleep Mode**: ESP32-S3 enters deep sleep between sensor readings
2. **Sensor Power Control**: Individual sensor enable/disable via GPIO
3. **Dynamic Frequency Scaling**: CPU frequency adjustment based on load
4. **Peripheral Clock Gating**: Unused peripherals are clock-gated
5. **Battery Monitoring**: ADC monitoring of battery voltage

## PCB Design Guidelines

### Layer Stack-up (4-layer recommended)

```
Layer 1: Component placement and routing
Layer 2: Ground plane (solid pour)
Layer 3: Power plane (3.3V and 5V)
Layer 4: Additional routing and shielding
```

### Design Rules

| Parameter | Specification | Notes |
|-----------|---------------|-------|
| **Trace Width** |
| Signal traces | 0.1mm minimum | For low-current signals |
| Power traces | 0.3mm minimum | For 100mA+ currents |
| High-current | 0.5mm+ | For motor drivers |
| **Via Specifications** |
| Via size | 0.2mm drill | Standard via |
| Via pad | 0.4mm | Annular ring |
| **Spacing** |
| Trace-to-trace | 0.1mm minimum | Same net |
| Trace-to-via | 0.1mm minimum | Different nets |
| Component spacing | 0.5mm minimum | For hand assembly |

### Critical Design Considerations

1. **Ground Plane Integrity**
   - Solid ground plane on layer 2
   - Minimize ground plane splits
   - Use ground vias liberally

2. **Power Distribution**
   - Dedicated power plane for 3.3V
   - Wide traces for high-current paths
   - Decoupling capacitors near each IC

3. **Signal Integrity**
   - Keep high-speed traces short
   - Use differential pairs for I2S
   - Avoid routing under crystal oscillators

4. **EMI/EMC Considerations**
   - Ferrite beads on power lines
   - Shielding for GPS antenna
   - Proper grounding of metal enclosure

5. **Thermal Management**
   - Thermal vias under power regulators
   - Adequate copper pour for heat dissipation
   - Component placement for airflow

### Component Placement Guidelines

```
┌─────────────────────────────────────────┐
│  [GPS]     [Audio Amp]     [Regulators] │
│                                         │
│  [ESP32-S3]              [Connectors]   │
│                                         │
│  [Sensors]    [RFID]      [Motors]      │
│                                         │
│  [Buttons]    [LEDs]      [Test Points] │
└─────────────────────────────────────────┘

Principles:
- Group by function
- Minimize trace lengths
- Separate analog and digital
- Consider mechanical constraints
```

## Assembly Instructions

### Required Tools
- Soldering iron (temperature controlled)
- Solder (0.6mm, lead-free)
- Flux paste
- Desoldering braid
- Multimeter
- Oscilloscope (for debugging)
- Hot air station (for SMD components)

### Assembly Sequence

1. **Power Supply Components**
   - Install voltage regulators
   - Add decoupling capacitors
   - Test power rails (3.3V, 5V)

2. **ESP32-S3 Module**
   - Solder ESP32-S3 development board
   - Verify power and ground connections
   - Test basic functionality

3. **Sensor Array**
   - Install I2C sensors (VL53L1X, MPU6050, BH1750)
   - Add I2C pull-up resistors
   - Install DHT22 and GPS module
   - Test I2C communication

4. **Communication Modules**
   - Install MFRC522 RFID reader
   - Add SPI connections
   - Test RFID functionality

5. **Feedback Systems**
   - Install audio amplifier
   - Add vibration motor drivers
   - Install servo motor connector
   - Test all feedback mechanisms

6. **User Interface**
   - Install buttons with pull-up resistors
   - Add status LEDs with current limiting resistors
   - Test button responsiveness

7. **Final Assembly**
   - Install connectors and test points
   - Add protective components (fuses, TVS diodes)
   - Perform comprehensive system test

### Soldering Guidelines

1. **Temperature Settings**
   - Through-hole components: 350°C
   - SMD components: 320°C
   - Heat-sensitive components: 300°C

2. **Soldering Technique**
   - Use flux for all joints
   - Heat pad and pin simultaneously
   - Apply solder to joint, not iron
   - Remove solder first, then iron

3. **Quality Checks**
   - Visual inspection of all joints
   - Continuity testing
   - Power-on test at each stage
   - Functional verification

## Testing & Validation

### Power-On Test Sequence

1. **Visual Inspection**
   - Check component orientation
   - Verify solder joint quality
   - Look for shorts or bridges

2. **Power Supply Test**
   ```
   Test Points:
   - Input voltage: 5V ±0.25V
   - 3.3V rail: 3.3V ±0.1V
   - 5V rail: 5.0V ±0.25V
   - Current draw: <50mA (idle)
   ```

3. **Communication Test**
   ```
   I2C Bus Test:
   - SDA/SCL voltage levels: 3.3V
   - Pull-up resistor values: 4.7kΩ ±5%
   - Device detection scan
   
   SPI Bus Test:
   - Clock signal integrity
   - Data line functionality
   - Chip select operation
   
   UART Test:
   - Baud rate accuracy: 9600 ±1%
   - Data transmission
   - GPS module response
   ```

4. **Sensor Validation**
   ```
   VL53L1X: Distance measurement accuracy
   MPU6050: Accelerometer and gyroscope data
   DHT22: Temperature and humidity readings
   BH1750: Light level measurement
   MFRC522: RFID card detection
   GPS: Satellite acquisition and position
   ```

5. **Feedback System Test**
   ```
   Audio: Frequency response and volume
   Vibration: Motor operation and intensity
   LEDs: Brightness and color accuracy
   Servo: Position accuracy and speed
   ```

### Automated Test Procedures

```cpp
// Example test function
void performSystemTest() {
    // Power supply validation
    if (!validatePowerRails()) {
        Serial.println("FAIL: Power supply test");
        return;
    }
    
    // I2C device detection
    if (!scanI2CDevices()) {
        Serial.println("FAIL: I2C device detection");
        return;
    }
    
    // Sensor functionality
    if (!testAllSensors()) {
        Serial.println("FAIL: Sensor validation");
        return;
    }
    
    // Feedback systems
    if (!testFeedbackSystems()) {
        Serial.println("FAIL: Feedback test");
        return;
    }
    
    Serial.println("PASS: All systems operational");
}
```

## Troubleshooting

### Common Issues and Solutions

#### Power Supply Problems

**Symptom**: System doesn't power on
```
Checklist:
□ Verify input voltage (5V ±0.25V)
□ Check regulator output voltages
□ Measure current draw (should be <100mA)
□ Inspect for short circuits
□ Verify decoupling capacitor placement

Solution:
- Check solder joints on power components
- Verify regulator orientation
- Test with external power supply
```

**Symptom**: System resets randomly
```
Checklist:
□ Check power supply stability
□ Verify decoupling capacitors
□ Measure voltage ripple (<50mV)
□ Check for ground loops
□ Verify current capacity

Solution:
- Add more decoupling capacitors
- Increase power supply current rating
- Improve ground plane design
```

#### Communication Issues

**Symptom**: I2C devices not detected
```
Checklist:
□ Verify SDA/SCL connections
□ Check pull-up resistor values (4.7kΩ)
□ Measure voltage levels (should be 3.3V)
□ Test with I2C scanner code
□ Verify device addresses

Solution:
- Check solder joints on I2C lines
- Verify pull-up resistor installation
- Test each device individually
```

**Symptom**: SPI communication failure
```
Checklist:
□ Verify MOSI/MISO/SCK connections
□ Check chip select (CS) operation
□ Measure clock signal integrity
□ Verify SPI mode settings
□ Check timing requirements

Solution:
- Verify SPI pin assignments
- Check clock polarity and phase
- Reduce SPI clock frequency
```

#### Sensor Problems

**Symptom**: ToF sensor gives incorrect readings
```
Checklist:
□ Verify I2C communication
□ Check sensor orientation
□ Test in different lighting conditions
□ Verify calibration settings
□ Check for obstructions

Solution:
- Recalibrate sensor
- Adjust measurement timing
- Check for interference sources
```

**Symptom**: GPS not acquiring satellites
```
Checklist:
□ Verify antenna connection
□ Test in open area
□ Check UART communication
□ Verify baud rate (9600)
□ Monitor NMEA sentences

Solution:
- Improve antenna placement
- Check for interference
- Verify GPS module power
```

#### Feedback System Issues

**Symptom**: Audio output distorted
```
Checklist:
□ Verify I2S connections
□ Check speaker impedance (4Ω or 8Ω)
□ Test with different audio files
□ Verify amplifier gain settings
□ Check power supply to amplifier

Solution:
- Adjust gain settings
- Check speaker connections
- Verify I2S timing
```

**Symptom**: Vibration motors not working
```
Checklist:
□ Verify transistor driver circuit
□ Check PWM signal generation
□ Test motor voltage and current
□ Verify flyback diode installation
□ Check for motor damage

Solution:
- Replace transistor driver
- Check PWM frequency and duty cycle
- Test motors with direct power
```

### Diagnostic Tools

#### Built-in Diagnostics
```cpp
// System diagnostic function
void runDiagnostics() {
    Serial.println("=== System Diagnostics ===");
    
    // Power supply check
    float voltage = analogRead(BATTERY_PIN) * 3.3 / 4095.0;
    Serial.printf("Battery Voltage: %.2fV\n", voltage);
    
    // I2C device scan
    Serial.println("I2C Device Scan:");
    for (int addr = 1; addr < 127; addr++) {
        Wire.beginTransmission(addr);
        if (Wire.endTransmission() == 0) {
            Serial.printf("  Device found at 0x%02X\n", addr);
        }
    }
    
    // Sensor readings
    Serial.printf("ToF Distance: %dmm\n", tofSensor.read());
    Serial.printf("Temperature: %.1f°C\n", dht.readTemperature());
    Serial.printf("Light Level: %d lux\n", lightSensor.readLightLevel());
    
    // Memory usage
    Serial.printf("Free Heap: %d bytes\n", ESP.getFreeHeap());
    Serial.printf("Free PSRAM: %d bytes\n", ESP.getFreePsram());
}
```

#### External Test Equipment

1. **Oscilloscope**: For signal integrity analysis
2. **Logic Analyzer**: For digital communication debugging
3. **Multimeter**: For voltage and resistance measurements
4. **Function Generator**: For testing audio and PWM systems
5. **Spectrum Analyzer**: For EMI/EMC compliance testing

## Safety Considerations

### Electrical Safety

1. **Power Supply Protection**
   - Fuse on input power (1A fast-blow)
   - TVS diodes for ESD protection
   - Reverse polarity protection
   - Overcurrent protection circuits

2. **Component Ratings**
   - All components rated for operating conditions
   - Adequate safety margins (2x minimum)
   - Proper heat dissipation
   - Insulation requirements met

3. **User Safety**
   - No exposed high voltages
   - Smooth edges on enclosure
   - Non-toxic materials only
   - Proper strain relief on cables

### EMC Compliance

1. **Emission Control**
   - Ferrite beads on power lines
   - Proper grounding and shielding
   - Clock frequency management
   - PCB layout optimization

2. **Immunity Requirements**
   - ESD protection on all interfaces
   - Power supply filtering
   - Signal line protection
   - Robust software design

### Environmental Considerations

1. **Operating Conditions**
   - Temperature: -10°C to +50°C
   - Humidity: 10% to 90% RH (non-condensing)
   - Altitude: Up to 2000m
   - Vibration resistance per IEC 60068

2. **Protection Rating**
   - Target: IP54 (dust and splash resistant)
   - Sealed connectors required
   - Conformal coating on PCB
   - Proper gasket design

### Regulatory Compliance

1. **Required Certifications**
   - FCC Part 15 (USA)
   - CE marking (Europe)
   - IC certification (Canada)
   - RoHS compliance

2. **Testing Requirements**
   - EMC testing per CISPR 22
   - Safety testing per IEC 62368
   - RF testing per applicable standards
   - Environmental testing

---

## Revision History

| Version | Date | Changes | Author |
|---------|------|---------|--------|
| 1.0 | 2024-01-15 | Initial release | Development Team |
| 1.1 | 2024-02-01 | Added power management details | Hardware Team |
| 1.2 | 2024-03-01 | Updated pin assignments | Firmware Team |
| 2.0 | 2024-03-15 | Complete rewrite with enhanced specifications | Technical Documentation |

---

*This document serves as the authoritative hardware reference for the Smart Assistive Cane V1 project. For software documentation, refer to the Technical Architecture document.*