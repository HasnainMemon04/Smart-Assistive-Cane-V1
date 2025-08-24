/*
  Arduino Secrets Template
  
  This file contains template values for sensitive configuration data.
  Copy this file to 'arduino_secrets.h' and fill in your actual values.
  
  IMPORTANT: Never commit arduino_secrets.h to version control!
*/

#ifndef ARDUINO_SECRETS_H
#define ARDUINO_SECRETS_H

// WiFi Credentials
#define SECRET_SSID "YOUR_WIFI_SSID_HERE"
#define SECRET_PASS "YOUR_WIFI_PASSWORD_HERE"

// Arduino IoT Cloud Credentials
#define SECRET_DEVICE_KEY "YOUR_DEVICE_KEY_HERE"
#define SECRET_DEVICE_LOGIN_NAME "YOUR_DEVICE_LOGIN_NAME_HERE"

// Optional: Backup WiFi Network
#define SECRET_BACKUP_SSID "YOUR_BACKUP_WIFI_SSID_HERE"
#define SECRET_BACKUP_PASS "YOUR_BACKUP_WIFI_PASSWORD_HERE"

// Optional: Emergency Contact Information
#define EMERGENCY_CONTACT_1 "YOUR_EMERGENCY_CONTACT_1_HERE"
#define EMERGENCY_CONTACT_2 "YOUR_EMERGENCY_CONTACT_2_HERE"

// Optional: Custom Configuration
#define DEVICE_NAME "Smart_Cane_V1"
#define FIRMWARE_VERSION "1.0.0"

#endif // ARDUINO_SECRETS_H

/*
  Setup Instructions:
  
  1. Copy this file to 'arduino_secrets.h'
  2. Replace all placeholder values with your actual credentials
  3. Save the file
  4. Compile and upload your sketch
  
  Arduino IoT Cloud Setup:
  
  1. Go to https://create.arduino.cc/iot/
  2. Create a new Thing
  3. Add a new device (ESP32)
  4. Copy the Device Key and Device Login Name
  5. Paste them into arduino_secrets.h
  
  WiFi Setup:
  
  1. Replace YOUR_WIFI_SSID_HERE with your WiFi network name
  2. Replace YOUR_WIFI_PASSWORD_HERE with your WiFi password
  3. Optionally configure backup WiFi credentials
*/