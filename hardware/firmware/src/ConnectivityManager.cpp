#include "ConnectivityManager.h"
#include "SDCardManager.h"
#include <Arduino.h>

// Static member initialization
bool ConnectivityManager::wifiConnected = false;
bool ConnectivityManager::timeInitialized = false;
bool ConnectivityManager::isDay = false;
uint32_t ConnectivityManager::lastReconnectAttempt = 0;
uint32_t ConnectivityManager::lastTimeUpdate = 0;
uint8_t ConnectivityManager::reconnectAttempts = 0;
double ConnectivityManager::sunriseTime = SUNRISE_TIME;
double ConnectivityManager::sunsetTime = SUNSET_TIME;

const char* ConnectivityManager::ntpServers[5] = {
  "pool.ntp.org",
  "time.nist.gov", 
  "time.google.com",
  "time.windows.com",
  "ntp.ubuntu.com"
};
const size_t ConnectivityManager::ntpServerCount = 5;

void ConnectivityManager::init() {
  Serial.println("🌐 Initializing Connectivity Manager...");
  
  // Set WiFi mode
  WiFi.mode(WIFI_STA);
  WiFi.setSleep(false); // Disable WiFi sleep for better reliability
  
  // Start background connection attempt
  startBackgroundConnection();
  
  // Setup offline mode as fallback
  setupOfflineMode();
  
  Serial.println("🌐 Connectivity Manager Ready (Background WiFi)");
  printStatus();
}

void ConnectivityManager::update() {
  handleSerialCommands();
  
  // Check WiFi status without blocking
  if (wifiConnected && WiFi.status() != WL_CONNECTED) {
    Serial.println("❌ WiFi connection lost");
    wifiConnected = false;
    timeInitialized = false;
  } else if (!wifiConnected && WiFi.status() == WL_CONNECTED) {
    Serial.println("✅ WiFi reconnected");
    wifiConnected = true;
    reconnectAttempts = 0;
    // Try to sync time in background
    if (!timeInitialized) {
      startBackgroundTimeSync();
    }
  }
  
  // Background reconnection logic
  if (!wifiConnected && (millis() - lastReconnectAttempt > WIFI_RECONNECT_INTERVAL)) {
    startBackgroundConnection();
  }
  
  // Update time status periodically
  if (wifiConnected && (millis() - lastTimeUpdate > 60000)) {
    updateDayNightStatus();
    lastTimeUpdate = millis();
  }
}

void ConnectivityManager::startBackgroundConnection() {
  if (strlen(ssid) == 0) {
    Serial.println("⚠️ No WiFi credentials - staying offline");
    return;
  }
  
  reconnectAttempts++;
  lastReconnectAttempt = millis();
  
  if (reconnectAttempts <= WIFI_RETRY_ATTEMPTS) {
    Serial.print("🔄 Background WiFi connection attempt ");
    Serial.print(reconnectAttempts);
    Serial.print("/");
    Serial.println(WIFI_RETRY_ATTEMPTS);
    
    // Non-blocking connection attempt
    WiFi.begin(ssid, password);
    
    // Don't wait here - let it connect in background
  } else {
    Serial.println("🔄 Max reconnection attempts reached - staying offline");
    // Try backup network if available
    if (strlen(ssid2) > 0 && reconnectAttempts == WIFI_RETRY_ATTEMPTS + 1) {
      Serial.println("🔄 Trying backup network...");
      WiFi.begin(ssid2, password2);
      reconnectAttempts = 0; // Reset for backup network
    }
  }
}

void ConnectivityManager::startBackgroundTimeSync() {
  if (!wifiConnected) return;
  
  Serial.println("🕐 Starting background time sync...");
  
  // Configure NTP non-blocking
  configTime(TIMEZONE_OFFSET, 0, ntpServers[0], ntpServers[1], ntpServers[2]);
  
  // Time sync will happen in background - check status in update()
}

bool ConnectivityManager::connectToWiFi(const char* networkSSID, const char* networkPassword) {
  // This function is now deprecated - use startBackgroundConnection instead
  return false;
}

bool ConnectivityManager::syncTime() {
  if (!wifiConnected) return false;
  
  // Check if time sync is complete
  struct tm timeinfo;
  if (getLocalTime(&timeinfo)) {
    timeInitialized = true;
    updateDayNightStatus();
    Serial.println("✅ Time synchronized in background!");
    char timeStr[64];
    strftime(timeStr, sizeof(timeStr), "%A, %B %d %Y %H:%M:%S", &timeinfo);
    Serial.println(timeStr);
    return true;
  }
  
  return false;
}

void ConnectivityManager::setupOfflineMode() {
  Serial.println("📱 Offline mode active");
  
  // Estimate time based on uptime
  unsigned long ms = millis();
  int totalSeconds = ms / 1000;
  int hours = (totalSeconds / 3600) % 24;
  
  isDay = (hours >= (int)SUNRISE_TIME && hours < (int)SUNSET_TIME);
  
  Serial.print("🕐 Estimated time: ");
  Serial.print(hours);
  Serial.print(":00 | Day status: ");
  Serial.println(isDay ? "Daytime" : "Nighttime");
}

void ConnectivityManager::updateDayNightStatus() {
  if (!timeInitialized) return;
  
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) return;
  
  double currentTime = timeinfo.tm_hour + timeinfo.tm_min / 60.0 + timeinfo.tm_sec / 3600.0;
  isDay = (currentTime >= sunriseTime && currentTime < sunsetTime);
}

void ConnectivityManager::reconnect() {
  Serial.println("🔄 Manual reconnection requested");
  reconnectAttempts = 0;
  startBackgroundConnection();
}

void ConnectivityManager::handleSerialCommands() {
  if (Serial.available()) {
    String cmd = Serial.readStringUntil('\n');
    cmd.trim();
    
    if (cmd == "wifi") {
      printStatus();
    } else if (cmd == "reconnect") {
      reconnect();
    } else if (cmd == "resetwifi") {
      resetCredentials();
    } else if (cmd == "offline") {
      Serial.println("📱 Switching to offline mode");
      WiFi.disconnect();
      wifiConnected = false;
      setupOfflineMode();
    }
  }
}

void ConnectivityManager::resetCredentials() {
  Serial.println("🗑️ Clearing WiFi credentials...");
  SDCard_deleteFile("/config/wifi_config.txt");
  
  WiFi.disconnect();
  wifiConnected = false;
  timeInitialized = false;
  
  Serial.println("✅ WiFi credentials cleared. Please update secrets.h and reboot.");
  while (1); // Halt for safety
}

void ConnectivityManager::printStatus() {
  Serial.println("\n=== CONNECTIVITY STATUS ===");
  Serial.print("WiFi: ");
  Serial.println(wifiConnected ? "✅ Connected" : "❌ Disconnected");
  
  if (wifiConnected) {
    Serial.print("Network: ");
    Serial.println(WiFi.SSID());
    Serial.print("IP: ");
    Serial.println(WiFi.localIP());
    Serial.print("Signal: ");
    Serial.print(WiFi.RSSI());
    Serial.println(" dBm");
  }
  
  Serial.print("Time: ");
  Serial.println(timeInitialized ? "✅ Synced" : "❌ Not synced");
  
  if (timeInitialized) {
    Serial.print("Current: ");
    Serial.println(getCurrentTime());
  }
  
  Serial.print("Day/Night: ");
  Serial.println(isDay ? "☀️ Daytime" : "🌙 Nighttime");
  Serial.println("===========================");
}

// Public interface functions
bool ConnectivityManager::isConnected() {
  return wifiConnected;
}

bool ConnectivityManager::isTimeSynced() {
  return timeInitialized;
}

bool ConnectivityManager::isDaytime() {
  return isDay;
}

String ConnectivityManager::getCurrentTime() {
  if (!timeInitialized) return "Unknown";
  
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) return "Error";
  
  char timeStr[9];
  strftime(timeStr, sizeof(timeStr), "%H:%M:%S", &timeinfo);
  return String(timeStr);
}

double ConnectivityManager::getCurrentHour() {
  if (!timeInitialized) return 12.0; // Default to noon
  
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) return 12.0;
  
  return timeinfo.tm_hour + timeinfo.tm_min / 60.0 + timeinfo.tm_sec / 3600.0;
}

int ConnectivityManager::getSignalStrength() {
  return wifiConnected ? WiFi.RSSI() : -100;
}

String ConnectivityManager::getNetworkName() {
  return wifiConnected ? WiFi.SSID() : "Offline";
}

String ConnectivityManager::getIPAddress() {
  return wifiConnected ? WiFi.localIP().toString() : "0.0.0.0";
}