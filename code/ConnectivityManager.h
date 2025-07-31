#pragma once
#ifndef CONNECTIVITYMANAGER_H
#define CONNECTIVITYMANAGER_H

#include <Arduino.h>
#include <WiFi.h>
#include <time.h>
#include <Preferences.h>

// WiFi configuration
#define WIFI_RETRY_ATTEMPTS 5
#define WIFI_RECONNECT_INTERVAL 30000  // 30 seconds
#define TIMEZONE_OFFSET 18000  // PKT (UTC+5) in seconds
#define SUNRISE_TIME 6.0   // 6:00 AM
#define SUNSET_TIME 18.0   // 6:00 PM

// WiFi credentials (from secrets.h)
extern const char* ssid;
extern const char* password;
extern const char* ssid2;
extern const char* password2;

class ConnectivityManager {
private:
  // Static member variables
  static bool wifiConnected;
  static bool timeInitialized;
  static bool isDay;
  static uint32_t lastReconnectAttempt;
  static uint32_t lastTimeUpdate;
  static uint8_t reconnectAttempts;
  static double sunriseTime;
  static double sunsetTime;
  
  // NTP servers
  static const char* ntpServers[5];
  static const size_t ntpServerCount;
  
  // Private methods
  static void startBackgroundConnection();
  static void startBackgroundTimeSync();
  static void setupOfflineMode();
  static void updateDayNightStatus();
  static void handleSerialCommands();

public:
  // Initialization
  static void init();
  static void update();
  
  // WiFi management
  static bool connectToWiFi(const char* networkSSID, const char* networkPassword);
  static void reconnect();
  static void resetCredentials();
  
  // Time synchronization
  static bool syncTime();
  
  // Status queries
  static bool isConnected();
  static bool isTimeSynced();
  static bool isDaytime();
  static String getCurrentTime();
  static double getCurrentHour();
  static int getSignalStrength();
  static String getNetworkName();
  static String getIPAddress();
  
  // Status display
  static void printStatus();
};

#endif // CONNECTIVITYMANAGER_H 