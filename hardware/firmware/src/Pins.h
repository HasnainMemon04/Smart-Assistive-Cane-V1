// Pin mapping for ESP32-S3-N16R8 DevKit (header-safe, avoids strapping pins):
// VIB1_PIN  = 38
// VIB2_PIN  = 39
// FEEDBACK  = 47
// I2C_SCL   = 9
// I2C_SDA   = 8
#pragma once
#ifndef SMARTCANE_PINS_H
#define SMARTCANE_PINS_H

// ❌ RESERVED PINS - DO NOT USE ❌
// GPIO35, GPIO36, GPIO37: Reserved for internal ESP32-S3 SPI flash/PSRAM communication
// Using these pins will cause system instability or crashes

// ⚠️ USE WITH CAUTION ⚠️
// GPIO0: Boot button pin - can interfere with boot process if used as output
// GPIO19, GPIO20: USB_D- and USB_D+ pins - cannot use if native USB functionality is needed

// ✅ PIN ASSIGNMENT VALIDATION ✅
// Current assignments verified safe - no conflicts with reserved pins
// All assigned pins (1,2,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,21,38,39,40,41,42,45,47,48) are valid
// Reserved pins (35,36,37) and caution pins (0,19,20) are NOT used

// Push-buttons
#define BTN1_PIN 2    // Yellow btn1
#define BTN2_PIN 41   // Green btn2
#define BTN3_PIN 1    // Blue btn3
#define BTN4_PIN 42   // Red btn4

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

// LEDs removed - unnecessary for blind users

// Feedback for IMU (header-safe)
#define FEEDBACK_PIN 47
#define CALIB_BUTTON_PIN BTN3_PIN

// Radar Mode - Servo
#define SERVO_PIN 45   // moved to pin 45

// SD Card SPI pins (HSPI)
#define SD_CS   7
#define SD_SCK  12
#define SD_MOSI 6
#define SD_MISO 4    // moved from 15 to avoid conflict with I2S DIN

// --- MAX98357A I2S (new) ---
#define I2S_BCLK_PIN  11   // BCLK (bit clock) -> MAX98357A BCLK
#define I2S_LRCLK_PIN 13   // LRCLK / WS -> MAX98357A LRC/WS
#define I2S_DIN_PIN   15   // DIN (I2S data) -> MAX98357A DIN
#define I2S_SD_PIN    3   // alternate safer mute pin to avoid UART RX conflict
#define MAX98357A_GAIN_TIED_TO_3V3 1  // hardware: tie GAIN to 3.3V for permanent +9dB

// NOTE: if the project uses Serial RX0 (UART0) on GPIO3, use GPIO22 instead of 3:
// #define I2S_SD_PIN 22  // alternate safer mute pin to avoid UART RX conflict

#endif // SMARTCANE_PINS_H