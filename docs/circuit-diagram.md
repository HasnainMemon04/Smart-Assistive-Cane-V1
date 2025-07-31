# Circuit Diagram and Wiring Guide

## Overview
This document provides detailed wiring instructions and circuit diagrams for the Smart Assistive Cane V1.

## Main Circuit Diagram
*[Circuit diagram image will be added here]*

## Component Connections

### ESP32-S3 Pin Assignments

#### I2C Bus (Sensors)
- **SDA**: GPIO 8
- **SCL**: GPIO 9
- **Pull-up Resistors**: 4.7kΩ to 3.3V

**Connected Devices:**
- DHT22 (Temperature/Humidity)
- BH1750 (Light Sensor)
- MPU6050 (IMU)
- VL53L1X (ToF Sensor)

#### SPI Bus (RFID)
- **MOSI**: GPIO 40
- **MISO**: GPIO 48
- **SCK**: GPIO 21
- **CS**: GPIO 10
- **RST**: GPIO 11

#### UART (GPS)
- **RX**: GPIO 17
- **TX**: GPIO 18
- **Baud Rate**: 9600

#### Feedback System
- **Buzzer**: GPIO 16 (with 100Ω resistor)
- **Vibration Motor 1**: GPIO 38 (with transistor driver)
- **Vibration Motor 2**: GPIO 39 (with transistor driver)

#### User Interface
- **Button 1**: GPIO 1 (with 10kΩ pull-up)
- **Button 2**: GPIO 2 (with 10kΩ pull-up)
- **Button 3**: GPIO 42 (with 10kΩ pull-up)
- **Button 4**: GPIO 41 (with 10kΩ pull-up)

#### Status Indicators
- **LED 1**: GPIO 43 (with 220Ω resistor)
- **LED 2**: GPIO 44 (with 220Ω resistor)

#### Servo Motor (Radar Mode)
- **Control**: GPIO 47
- **Power**: 5V (external supply recommended)

## Power Supply

### Requirements
- **Voltage**: 3.3V for ESP32-S3
- **Current**: 500mA minimum (1A recommended)
- **Battery**: 7.4V Li-Po with voltage regulator

### Power Distribution
```
Battery (7.4V)
    |
    ├── Voltage Regulator (3.3V) → ESP32-S3 + Sensors
    ├── Voltage Regulator (5V) → Servo Motor
    └── Direct → Vibration Motors (with PWM control)
```

## Wiring Tips

1. **Use twisted pairs** for I2C lines to reduce noise
2. **Keep power lines short** and use bypass capacitors
3. **Separate analog and digital grounds** where possible
4. **Use shielded cable** for GPS antenna connection
5. **Add ferrite beads** on power lines to reduce EMI

## PCB Layout Considerations

- **Ground plane** for noise reduction
- **Separate power domains** for analog and digital
- **Proper trace width** for current carrying capacity
- **Component placement** to minimize interference

## Testing Points

- **3.3V Rail**: Test point for power verification
- **I2C Lines**: Test points for signal integrity
- **GPS UART**: Test points for communication debugging
- **Sensor Outputs**: Test points for individual sensor verification

## Troubleshooting

### Common Issues
1. **I2C Communication Errors**
   - Check pull-up resistors
   - Verify connections
   - Test with I2C scanner

2. **Power Issues**
   - Check voltage levels
   - Verify current capacity
   - Test under load

3. **GPS Not Working**
   - Check antenna connection
   - Verify baud rate
   - Test in open area

*Detailed PCB files and Gerber files will be added to this folder.*