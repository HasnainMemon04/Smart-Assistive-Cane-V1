#pragma once
#ifndef SECRETS_H
#define SECRETS_H

// WiFi Credentials - Replace with your network details
extern const char* ssid;           // Replace with your WiFi name
extern const char* password;   // Replace with your WiFi password

// Alternative WiFi networks (for fallback)
extern const char* ssid2;        // Optional backup network
extern const char* password2; // Optional backup password

// Connectivity Settings
#define WIFI_TIMEOUT_MS 15000        // 15 seconds timeout
#define WIFI_RETRY_ATTEMPTS 3        // Number of retry attempts
#define WIFI_RECONNECT_INTERVAL 30000 // 30 seconds between reconnection attempts
#define NTP_TIMEOUT_MS 10000         // 10 seconds for NTP sync
#define OFFLINE_MODE_ENABLED true    // Enable offline operation

// Timezone and Location Settings
#define TIMEZONE_OFFSET 5            // UTC+5 (adjust for your location)
#define SUNRISE_TIME 5.5             // 5:30 AM
#define SUNSET_TIME 19.0             // 7:00 PM

#endif // SECRETS_H 