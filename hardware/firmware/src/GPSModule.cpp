#include "GPSModule.h"
#include "Pins.h"
#include "SensorHealth.h"
#include "SDCardManager.h"
#include <TinyGPS++.h>
#include <HardwareSerial.h>
#include <math.h>

#include "IMU.h"
#include "ConnectivityManager.h"

#define SAT_COUNT 32
#define MAX_GNSS_SYSTEMS 4
#define MIN_SIGNAL_STRENGTH -180 // Lower threshold to accept weaker signals
#define ACCURACY_THRESHOLD 100.0 // Much higher threshold for speed
#define VELOCITY_SMOOTHING_FACTOR 0.1  // Minimal smoothing for faster response
#define HEADING_SMOOTHING_FACTOR 0.1   // Minimal smoothing for faster response

static TinyGPSPlus gps;
static HardwareSerial gpsSerial(1);

// Enhanced GPS Configuration and Status
static GPSConfig gpsConfig = {
  .updateRate = 5,           // 5Hz maximum for fastest updates
  .enableSBAS = false,       // Disable SBAS to reduce processing time
  .enableMultiGNSS = true,   // Keep multi-constellation for more satellites
  .dynamicModel = 0,         // Pedestrian model (fastest)
  .elevationMask = 0,        // 0 degrees elevation mask to use all satellites
  // Jamming detection completely removed for performance
};

static GPSStatus gpsStatus = {
  .isFixed = false,
  .fixType = 0,
  .satellitesUsed = 0,
  .satellitesVisible = 0,
  .hdop = 99.9,
  .vdop = 99.9,
  .pdop = 99.9,
  .accuracy = 999.9,
  .signalStrength = -127,
  .timeToFirstFix = 0
};

// Reference "home" coordinates
static const double REF_LAT = 25.382344;
static const double REF_LON = 68.327323;

// Enhanced tracking variables
static uint32_t firstFixTime = 0;
static bool firstFixAchieved = false;
static float smoothedVelocity = 0.0;
static float smoothedHeading = 0.0;
static int signalStrengthHistory[10] = {0};
static int signalHistoryIndex = 0;
static uint32_t configSaveTimer = 0;
static bool configChanged = false;

// Haversine distance function (meters)
static double haversine(double lat1, double lon1, double lat2, double lon2) {
  const double R = 6371000;
  double dLat = radians(lat2 - lat1);
  double dLon = radians(lon2 - lon1);
  double a = sin(dLat/2) * sin(dLat/2) +
             cos(radians(lat1)) * cos(radians(lat2)) *
             sin(dLon/2) * sin(dLon/2);
  double c = 2 * atan2(sqrt(a), sqrt(1-a));
  return R * c;
}

struct Satellite {
  int prn;
  int elevation;
  int azimuth;
  int snr;
  float quality;
  bool active;
  uint8_t system;
};

static Satellite satellites[SAT_COUNT];
static int satCount = 0;
static bool newSatData = false;

// Simplified position variables for speed
static double latFiltered = 0.0;
static double lonFiltered = 0.0;

// Position averaging
static const int POSITION_BUFFER_SIZE = 30;
static double latBuffer[POSITION_BUFFER_SIZE];
static double lonBuffer[POSITION_BUFFER_SIZE];
static int bufferIndex = 0;
static bool bufferFilled = false;

// Display control
static bool showRawData = false;
static bool showSatellites = true;
static bool showTop3 = true;
static uint32_t lastSatDisplay = 0;
static const uint32_t SAT_DISPLAY_INTERVAL = 5000;

// GSV sentence parsing state
static int gsvSentenceCount = 0;
static int gsvSentenceNumber = 0;
static int gsvSatsInView = 0;
static bool newGSVData = false;

// Prototypes
static void handleCommands();
static void parseNMEA(char c);
static void processGSV(String sentence);
static void displayStatus();
static void displayTop3Satellites();
static void displayAllSatellites();
static void kalmanUpdate(double latMeasurement, double lonMeasurement);
static void updatePositionBuffer(double lat, double lon);
static double getAverageLat();
static double getAverageLon();
static void processPosition(double lat, double lon);
static void printTwoDigits(int number);
static void updateGPSStatus();
static void updateSignalStrength(int strength);
static void sendGPSCommand(const char* command);
static uint8_t calculateChecksum(const char* sentence);
static void saveConfigToSD();
static void loadConfigFromSD();

void GPSModule_init() {
  gpsSerial.begin(GPS_BAUD, SERIAL_8N1, GPS_RX, GPS_TX);
  
  delay(500);  // Minimal delay for speed
  
  // Test GPS UART communication
  gpsSerial.println("$PMTK000*32"); // Query release
  delay(100);
  
  bool responseReceived = false;
  uint32_t startTime = millis();
  while (millis() - startTime < 1000) { // Wait up to 1 second for response
    if (gpsSerial.available()) {
      responseReceived = true;
      break;
    }
  }
  
  if (!responseReceived) {
    SensorHealthManager::updateSensorHealth("neo6m", SENSOR_DISCONNECTED, nullptr, "GPS UART failed");
    Serial.println("[GPS] NEO-6M UART communication failed");
    return;
  }
  
  // Speed-optimized GPS configuration
  // Set maximum update rate (5Hz = 200ms)
  gpsSerial.println("$PMTK220,200*2C");
  delay(50);
  
  // Disable SBAS for faster processing
  gpsSerial.println("$PMTK313,0*2F");
  delay(50);
  
  // Enable all GNSS for maximum satellite availability
  gpsSerial.println("$PMTK353,1,1,1,1,1*2B");  // GPS+GLONASS+Galileo+BeiDou+QZSS
  delay(50);
  
  // Set portable dynamic model (fastest acquisition)
  gpsSerial.println("$PMTK886,0*28");
  delay(50);
  
  // Minimal output sentences for speed
  gpsSerial.println("$PMTK314,0,1,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0*29");
  delay(50);
  
  // Disable elevation mask (use all satellites)
  gpsSerial.println("$PMTK397,0*23");
  delay(50);
  
  // Enable hot start for fastest subsequent fixes
  gpsSerial.println("$PMTK101*32");
  
  firstFixTime = millis();
  
  // Report successful initialization
  SensorHealthManager::updateSensorHealth("neo6m", SENSOR_OK, "GPS initialized");
  
#ifdef SC_DEBUG_GPS
  Serial.println("[GPS] MAXIMUM SPEED initialization complete");
  Serial.println("[GPS] 5Hz update rate, all GNSS enabled, no accuracy filtering");
  Serial.println("Commands: [r]aw, [s]ats, [t]op3, [v]iew, [d]status");
#endif
}

void GPSModule_update(SensorData* data) {
  // Ultra-fast GPS processing - minimal overhead
  static uint32_t lastGPSHealthCheck = 0;
  bool dataReceived = false;
  
  while (gpsSerial.available() > 0) {
    dataReceived = true;
    if (gps.encode(gpsSerial.read())) {
      // Record TTFF only once
      if (!firstFixAchieved && gps.location.isValid()) {
        gpsStatus.timeToFirstFix = millis() - firstFixTime;
        firstFixAchieved = true;
        SensorHealthManager::updateSensorHealth("neo6m", SENSOR_OK, "First fix achieved");
      }
      
      // Process position immediately without filtering
      if (gps.location.isValid()) {
        latFiltered = gps.location.lat();
        lonFiltered = gps.location.lng();
        
        // Update health with satellite count
        uint8_t satCount = gps.satellites.isValid() ? gps.satellites.value() : 0;
        SensorHealthManager::updateSensorHealth("neo6m", SENSOR_OK, String(satCount).c_str());
        
        // Direct assignment - no smoothing for maximum speed
        if (gps.speed.isValid()) {
          smoothedVelocity = gps.speed.kmph();
        }
        if (gps.course.isValid()) {
          smoothedHeading = gps.course.deg();
        }
      }
      
    }
  }
  
  // Check for GPS timeout (no data received)
  if (millis() - lastGPSHealthCheck > 5000) { // Check every 5 seconds
    lastGPSHealthCheck = millis();
    if (!dataReceived && !gps.location.isValid()) {
      SensorHealthManager::updateSensorHealth("neo6m", SENSOR_TIMEOUT, nullptr, "No GPS data");
    } else if (!gps.location.isValid()) {
      SensorHealthManager::updateSensorHealth("neo6m", SENSOR_ERROR, nullptr, "No GPS fix");
    }
  }
  
  // Direct data assignment - no validation checks
   if (data) {
     data->gpsLat = latFiltered;
     data->gpsLon = lonFiltered;
     data->gpsAlt = gps.altitude.isValid() ? gps.altitude.meters() : 0;
     data->gpsSpeed = smoothedVelocity;
     data->gpsSatellites = gps.satellites.isValid() ? gps.satellites.value() : 0;
   }
}

// --- All static helper functions below (from v7.ino) ---
static void parseNMEA(char c) {
  static String nmeaBuffer;
  static bool inSentence = false;
  if (c == '$') {
    nmeaBuffer = "$";
    inSentence = true;
    return;
  }
  if (inSentence) {
    if (c == '\r' || c == '\n') {
      inSentence = false;
      if (nmeaBuffer.startsWith("$GPGSV") || 
          nmeaBuffer.startsWith("$GLGSV") || 
          nmeaBuffer.startsWith("$GAGSV")) {
        processGSV(nmeaBuffer);
      }
      nmeaBuffer = "";
    } else if (nmeaBuffer.length() < 120) {
      nmeaBuffer += c;
    }
  }
}
static void processGSV(String sentence) {
  int params[20] = {0};
  int paramCount = 0;
  int start = 0;
  for (int i = 0; i < sentence.length(); i++) {
    if (sentence[i] == ',' || sentence[i] == '*') {
      String param = sentence.substring(start, i);
      start = i + 1;
      if (paramCount < 20) {
        params[paramCount] = (param.length() > 0 && param[0] >= '0') ? param.toInt() : 0;
      }
      paramCount++;
    }
  }
  if (paramCount >= 4) {
    gsvSentenceCount = params[1];
    gsvSentenceNumber = params[2];
    gsvSatsInView = params[3];
    if (gsvSentenceNumber == 1) satCount = 0;
    uint8_t systemID = 0;
    if (sentence.startsWith("$GLGSV")) systemID = 1;
    if (sentence.startsWith("$GAGSV")) systemID = 2;
    for (int i = 4; i < paramCount && i < 20; i += 4) {
      if (params[i] > 0 && satCount < SAT_COUNT) {
        satellites[satCount].system = systemID;
        satellites[satCount].prn = params[i];
        satellites[satCount].elevation = (i+1 < paramCount) ? params[i+1] : 0;
        satellites[satCount].azimuth = (i+2 < paramCount) ? params[i+2] : 0;
        satellites[satCount].snr = (i+3 < paramCount) ? params[i+3] : 0;
        float elevFactor = 0.5 + (satellites[satCount].elevation / 180.0);
        satellites[satCount].quality = satellites[satCount].snr * elevFactor;
        satellites[satCount].active = false;
        if (gps.satellites.isValid() && satCount < gps.satellites.value()) {
          satellites[satCount].active = true;
        }
        satCount++;
      }
    }
    if (gsvSentenceNumber == gsvSentenceCount) {
      newSatData = true;
      newGSVData = true;
    }
  }
}
static void handleCommands() {
  // GPS module no longer processes individual characters to avoid conflicts
  // with main serial command processing. GPS commands are now handled
  // through the main command system in SmartCaneMain.ino
}
// Removed Kalman filtering for maximum speed
static void updatePositionBuffer(double lat, double lon) {
  latBuffer[bufferIndex] = lat;
  lonBuffer[bufferIndex] = lon;
  bufferIndex = (bufferIndex + 1) % POSITION_BUFFER_SIZE;
  if (bufferIndex == 0) bufferFilled = true;
}
static double getAverageLat() {
  double sum = 0.0;
  int count = bufferFilled ? POSITION_BUFFER_SIZE : bufferIndex;
  if (count == 0) return 0.0;
  for (int i = 0; i < count; i++) sum += latBuffer[i];
  return sum / count;
}
static double getAverageLon() {
  double sum = 0.0;
  int count = bufferFilled ? POSITION_BUFFER_SIZE : bufferIndex;
  if (count == 0) return 0.0;
  for (int i = 0; i < count; i++) sum += lonBuffer[i];
  return sum / count;
}
static void processPosition(double lat, double lon) {
  // Direct assignment for maximum speed - no filtering
  latFiltered = lat;
  lonFiltered = lon;
  updatePositionBuffer(lat, lon);
}
static void displayStatus() {
  static uint32_t lastDisplay = 0;
  if (millis() - lastDisplay < 1000) return;
#ifdef SC_DEBUG_GPS
  Serial.print("\n--- STATUS --- ");
  Serial.print(millis() / 1000);
  Serial.println("s -----------------");
#endif
  if (gps.date.isValid()) {
#ifdef SC_DEBUG_GPS
    Serial.print("Date: ");
    Serial.print(gps.date.month());
    Serial.print('/');
    Serial.print(gps.date.day());
    Serial.print('/');
    Serial.println(gps.date.year());
#endif
  } else {
#ifdef SC_DEBUG_GPS
    Serial.println("Date: --/--/----");
#endif
  }
  if (gps.time.isValid()) {
#ifdef SC_DEBUG_GPS
    Serial.print("Time: ");
    printTwoDigits(gps.time.hour());
    Serial.print(':');
    printTwoDigits(gps.time.minute());
    Serial.print(':');
    printTwoDigits(gps.time.second());
    Serial.println(" UTC");
#endif
  } else {
#ifdef SC_DEBUG_GPS
    Serial.println("Time: --:--:--");
#endif
  }
  if (gps.location.isValid()) {
#ifdef SC_DEBUG_GPS
    Serial.print("LAT: ");
    Serial.print(gps.location.lat(), 9);
    Serial.print("  LON: ");
    Serial.print(gps.location.lng(), 9);
    Serial.print("\nFILT LAT: ");
    Serial.print(latFiltered, 9);
    Serial.print("  FILT LON: ");
    Serial.print(lonFiltered, 9);
    if (bufferFilled || bufferIndex > 4) {
      Serial.print("\nAVG LAT: ");
      Serial.print(getAverageLat(), 9);
      Serial.print("  AVG LON: ");
      Serial.print(getAverageLon(), 9);
    }
    Serial.println();
    double distance = haversine(latFiltered, lonFiltered, REF_LAT, REF_LON);
    Serial.print("Distance from reference: ");
    Serial.print(distance, 2);
    Serial.println(" meters");
    if (distance < 10) {
      Serial.println("\u2705 You are at the reference point!");
    }
    if (gps.altitude.isValid()) {
      Serial.print("ALT: ");
      Serial.print(gps.altitude.meters());
      Serial.print("m");
    }
    if (gps.speed.isValid()) {
      Serial.print("  Speed: ");
      Serial.print(gps.speed.kmph());
      Serial.print("km/h");
    }
    if (gps.course.isValid()) {
      Serial.print("  Course: ");
      Serial.print(gps.course.deg());
      Serial.print("\u00b0");
    }
    Serial.println();
#endif
  } else {
#ifdef SC_DEBUG_GPS
    Serial.println("Location: Waiting for fix...");
#endif
  }
#ifdef SC_DEBUG_GPS
  Serial.print("Sats used: ");
  if (gps.satellites.isValid()) {
    Serial.print(gps.satellites.value());
  } else {
    Serial.print("--");
  }
  Serial.print("  Sats in view: ");
  Serial.print(satCount);
  Serial.print("  HDOP: ");
  if (gps.hdop.isValid()) {
    float hdop = gps.hdop.value() / 100.0;
    Serial.print(hdop, 1);
    Serial.print(" (");
    if (hdop < 1.0) Serial.print("Ideal");
    else if (hdop < 2.0) Serial.print("Excellent");
    else if (hdop < 5.0) Serial.print("Good");
    else if (hdop < 10.0) Serial.print("Moderate");
    else Serial.print("Poor");
    Serial.println(")");
    if (hdop < 2.0) {
      Serial.println("GPS Fix: 3D");
    } else if (hdop < 5.0) {
      Serial.println("GPS Fix: 2D");
    } else {
      Serial.println("GPS Fix: Weak or None");
    }
  } else {
    Serial.println("--");
  }
  lastDisplay = millis();
#endif
}
static void printTwoDigits(int number) {
#ifdef SC_DEBUG_GPS
  if (number < 10) Serial.print('0');
  Serial.print(number);
#endif
}
static void displayTop3Satellites() {
#ifdef SC_DEBUG_GPS
  Serial.println("\n--- TOP 3 SATELLITES BY QUALITY ---");
#endif
  if (satCount == 0) {
#ifdef SC_DEBUG_GPS
    Serial.println("No satellites detected");
#endif
    return;
  }
  for (int i = 0; i < satCount - 1; i++) {
    for (int j = i + 1; j < satCount; j++) {
      if (satellites[j].quality > satellites[i].quality) {
        Satellite temp = satellites[i];
        satellites[i] = satellites[j];
        satellites[j] = temp;
      }
    }
  }
#ifdef SC_DEBUG_GPS
  Serial.println("SYS PRN Elev Azim SNR Qual");
  Serial.println("-------------------------");
#endif
  int displayCount = min(3, satCount);
  int validCount = 0;
  for (int i = 0; i < displayCount; i++) {
    if (satellites[i].prn > 0) {
      switch(satellites[i].system) {
        case 0: Serial.print("GPS "); break;
        case 1: Serial.print("GLO "); break;
        case 2: Serial.print("GAL "); break;
        default: Serial.print("UNK "); break;
      }
      Serial.print(satellites[i].prn);
      Serial.print("   ");
      if (satellites[i].elevation < 100) Serial.print(" ");
      Serial.print(satellites[i].elevation);
      Serial.print("\u00b0");
      Serial.print("  ");
      if (satellites[i].azimuth < 100) Serial.print(" ");
      if (satellites[i].azimuth < 10) Serial.print(" ");
      Serial.print(satellites[i].azimuth);
      Serial.print("\u00b0");
      Serial.print("  ");
      Serial.print(satellites[i].snr);
      Serial.print("  ");
      Serial.print(satellites[i].quality, 1);
      Serial.println();
      validCount++;
    }
  }
  if (validCount == 0) {
#ifdef SC_DEBUG_GPS
    Serial.println("No valid satellites detected");
#endif
  }
#ifdef SC_DEBUG_GPS
  Serial.print("Total tracked: ");
  Serial.println(satCount);
#endif
}
static void displayAllSatellites() {
  if (!newGSVData) {
#ifdef SC_DEBUG_GPS
    Serial.println("\nNo recent satellite data. Wait for GSV update.");
#endif
    return;
  }
#ifdef SC_DEBUG_GPS
  Serial.println("\n--- DETAILED SATELLITE INFO ---");
  // Rest of function continues...
#endif
}

// ===== ENHANCED GPS FUNCTIONS =====

static void updateGPSStatus() {
  // Minimal status update for speed
  gpsStatus.isFixed = gps.location.isValid();
  gpsStatus.fixType = gps.location.isValid() ? 2 : 0;
  gpsStatus.satellitesUsed = gps.satellites.isValid() ? gps.satellites.value() : 0;
  if (gps.hdop.isValid()) {
    gpsStatus.hdop = gps.hdop.value() / 100.0;
  }
}



static void updateSignalStrength(int strength) {
  // Signal strength tracking disabled for speed
}

static void sendGPSCommand(const char* command) {
  uint8_t checksum = calculateChecksum(command);
  gpsSerial.print(command);
  gpsSerial.printf("%02X\r\n", checksum);
  delay(100);
}

static uint8_t calculateChecksum(const char* sentence) {
  uint8_t checksum = 0;
  for (int i = 1; sentence[i] != '*' && sentence[i] != '\0'; i++) {
    checksum ^= sentence[i];
  }
  return checksum;
}

static void saveConfigToSD() {
  String configData = String(gpsConfig.updateRate) + "," +
                     String(gpsConfig.enableSBAS) + "," +
                     String(gpsConfig.enableMultiGNSS) + "," +
                     String(gpsConfig.dynamicModel) + "," +
                     String(gpsConfig.elevationMask) + "," +
                     String(false); // Jamming detection removed
  
  SDCard_writeFile("/config/gps_config.txt", configData.c_str());
}

static void loadConfigFromSD() {
  String configData = SDCard_readFile("/config/gps_config.txt");
  if (configData.length() > 0) {
    int commaIndex = 0;
    int lastIndex = 0;
    int paramIndex = 0;
    
    while ((commaIndex = configData.indexOf(',', lastIndex)) != -1 && paramIndex < 6) {
      String param = configData.substring(lastIndex, commaIndex);
      
      switch (paramIndex) {
        case 0: gpsConfig.updateRate = param.toInt(); break;
        case 1: gpsConfig.enableSBAS = param.toInt(); break;
        case 2: gpsConfig.enableMultiGNSS = param.toInt(); break;
        case 3: gpsConfig.dynamicModel = param.toInt(); break;
        case 4: gpsConfig.elevationMask = param.toInt(); break;
        case 5: /* Jamming detection removed */ break;
      }
      
      lastIndex = commaIndex + 1;
      paramIndex++;
    }
  }
}

// ===== PUBLIC API FUNCTIONS =====

void GPSModule_setUpdateRate(uint8_t rate) {
  if (rate >= 1 && rate <= 5) {
    gpsConfig.updateRate = rate;
    String cmd = "$PMTK220," + String(1000 / rate) + "*";
    sendGPSCommand(cmd.c_str());
    configChanged = true;
  }
}

void GPSModule_enableSBAS(bool enable) {
  gpsConfig.enableSBAS = enable;
  if (enable) {
    sendGPSCommand("$PMTK313,1*");  // Enable SBAS
    sendGPSCommand("$PMTK301,2*");  // Enable DGPS
  } else {
    sendGPSCommand("$PMTK313,0*");  // Disable SBAS
  }
  configChanged = true;
}

void GPSModule_setDynamicModel(uint8_t model) {
  gpsConfig.dynamicModel = model;
  String cmd = "$PMTK886," + String(model) + "*";
  sendGPSCommand(cmd.c_str());
  configChanged = true;
}

void GPSModule_enableAntiJamming(bool enable) {
  // Anti-jamming functionality completely removed for performance
}

void GPSModule_performColdStart() {
  sendGPSCommand("$PMTK104*");  // Cold start
  firstFixTime = millis();
  firstFixAchieved = false;
#ifdef SC_DEBUG_GPS
  Serial.println("[GPS] Cold start initiated");
#endif
}

void GPSModule_performWarmStart() {
  sendGPSCommand("$PMTK102*");  // Warm start
  firstFixTime = millis();
  firstFixAchieved = false;
#ifdef SC_DEBUG_GPS
  Serial.println("[GPS] Warm start initiated");
#endif
}

void GPSModule_performHotStart() {
  sendGPSCommand("$PMTK101*");  // Hot start
  firstFixTime = millis();
  firstFixAchieved = false;
#ifdef SC_DEBUG_GPS
  Serial.println("[GPS] Hot start initiated");
#endif
}

GPSStatus GPSModule_getStatus() {
  return gpsStatus;
}

GPSConfig GPSModule_getConfig() {
  return gpsConfig;
}

void GPSModule_saveConfig() {
  saveConfigToSD();
  configChanged = false;
#ifdef SC_DEBUG_GPS
  Serial.println("[GPS] Configuration saved to SD card");
#endif
}

void GPSModule_loadConfig() {
  loadConfigFromSD();
#ifdef SC_DEBUG_GPS
  Serial.println("[GPS] Configuration loaded from SD card");
#endif
}

bool GPSModule_isJammingDetected() {
  return false;  // Jamming detection completely removed
}

float GPSModule_getPositionAccuracy() {
  return gpsStatus.accuracy;
}

uint32_t GPSModule_getTimeToFirstFix() {
  return gpsStatus.timeToFirstFix;
}

void GPSModule_enablePowerSaving(bool enable) {
  if (enable) {
    sendGPSCommand("$PMTK225,4*");  // Backup mode
  } else {
    sendGPSCommand("$PMTK225,0*");  // Normal mode
  }
#ifdef SC_DEBUG_GPS
  Serial.printf("[GPS] Power saving %s\n", enable ? "enabled" : "disabled");
#endif
}

void GPSModule_setElevationMask(uint8_t degrees) {
  if (degrees <= 90) {
    gpsConfig.elevationMask = degrees;
    String cmd = "$PMTK307," + String(degrees) + "*";
    sendGPSCommand(cmd.c_str());
    configChanged = true;
  }
}