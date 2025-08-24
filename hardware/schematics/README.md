# Circuit Schematics

This directory contains circuit schematics and electrical diagrams for the Smart Assistive Cane.

## Purpose

Schematic files include:
- Main system block diagram
- ESP32 microcontroller connections
- Sensor interface circuits
- Power supply and battery management
- Communication module connections
- Audio amplifier circuits

## File Formats

- PDF (for viewing and printing)
- PNG/SVG (for documentation)
- KiCad schematic files (.kicad_sch)
- Eagle schematic files (.sch)
- Altium Designer files (.SchDoc)

## Circuit Overview

The Smart Assistive Cane uses:
- ESP32-WROOM-32 as the main controller
- VL53L0X ToF sensor for distance measurement
- MPU6050 IMU for motion detection
- NEO-6M GPS module for location tracking
- RC522 RFID reader for indoor navigation
- Environmental sensors (temperature, humidity, light)
- Audio feedback system

## Power Requirements

- Operating voltage: 3.3V (regulated from battery)
- Battery: Li-ion 18650 (3.7V nominal)
- Charging: USB-C with built-in charge controller
- Power consumption: ~200mA average, ~500mA peak

## Usage

Refer to these schematics when:
- Building the hardware
- Troubleshooting connections
- Modifying the design
- Understanding signal flow