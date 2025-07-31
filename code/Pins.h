// Pin mapping for ESP32-S3-N16R8 DevKit (header-safe, avoids strapping pins):
// VIB1_PIN  = 38
// VIB2_PIN  = 39
// LED1_PIN  = 43
// LED2_PIN  = 44
// FEEDBACK  = 47
// I2C_SCL   = 9
// I2C_SDA   = 8
#pragma once
#ifndef SMARTCANE_PINS_H
#define SMARTCANE_PINS_H

// Push-buttons
#define BTN1_PIN 1    // IMU recalibration
#define BTN2_PIN 2    // User mode
#define BTN3_PIN 42   // ToF mode switch
#define BTN4_PIN 41   // Toggle audio amp

// I2C (shared)
#define I2C_SDA 8
#define I2C_SCL 9

// SPI (RFID)
#define SPI_MOSI 40   // Safe on ESP32-S3
#define SPI_MISO 48   // MISO moved to free native USB pin
#define SPI_SCK  21   // SCK moved off flash pin
#define RFID_CS  10
#define RFID_RST 14

// UART-1 (GPS)
#define GPS_RX 17
#define GPS_TX 18
#define GPS_BAUD 9600

// DHT22
#define DHTPIN 5
#define DHTTYPE DHT22

// Vibration motors (header-safe)
#define VIB1_PIN 38
#define VIB2_PIN 39

// Piezo buzzer
#define BUZZER_PIN 16

// LEDs (header-safe, not strapping pins)
#define LED1_PIN 43   // Top-right pin (TX LED)
#define LED2_PIN 44   // Second from top-right (RX LED)

// Feedback for IMU (header-safe)
#define FEEDBACK_PIN 47
#define CALIB_BUTTON_PIN BTN1_PIN

// Radar Mode - Servo
#define SERVO_PIN 13  // Servo control pin for radar scanning

#endif // SMARTCANE_PINS_H 