#include "GPSModule.h"
#include "Pins.h"
#include <TinyGPS++.h>
#include <HardwareSerial.h>
#include <math.h>

#include "IMU.h"
#include "ConnectivityManager.h"

#define SAT_COUNT 32

static TinyGPSPlus gps;
static HardwareSerial gpsSerial(1);

// Reference "home" coordinates
static const double REF_LAT = 25.382344;
static const double REF_LON = 68.327323;

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

// Kalman Filter Variables
static double latFiltered = 0.0;
static double lonFiltered = 0.0;
static double variance = 1.0;
static const double PROCESS_NOISE = 0.01;
static const double MEASUREMENT_NOISE = 0.2;

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

void GPSModule_init() {
  gpsSerial.begin(GPS_BAUD, SERIAL_8N1, GPS_RX, GPS_TX);
#ifdef SC_DEBUG_GPS
  Serial.println("[GPS] Initialized");
  Serial.println("Commands: [r]aw, [s]ats, [t]op3, [v]iew, [d]status");
#endif
}

void GPSModule_update(SensorData* data) {
  handleCommands();
  while (gpsSerial.available() > 0) {
    char c = gpsSerial.read();
    if (showRawData) Serial.write(c);
    parseNMEA(c);
    if (gps.encode(c)) {
      if (gps.location.isValid() && gps.hdop.isValid() && gps.hdop.value() <= 800) {
        if (gps.speed.isValid() && gps.speed.kmph() > 0.2) {
          processPosition(gps.location.lat(), gps.location.lng());
        }
      }
      displayStatus();
      
      // Set system time when GPS time is available (only if internet time is not available)
      static bool timeSet = false;
      if (!timeSet && gps.date.isValid() && gps.time.isValid()) {
        // Only set GPS time if internet time is not available
        if (!ConnectivityManager::isTimeSynced()) {
          IMU_setTime(gps.time.hour(), gps.time.minute(), gps.date.day(), gps.date.month(), gps.date.year());
          timeSet = true;
#ifdef SC_DEBUG_GPS
          Serial.println("GPS time set as fallback (internet time not available)");
#endif
        } else {
#ifdef SC_DEBUG_GPS
          Serial.println("Internet time available - GPS time not needed");
#endif
          timeSet = true; // Mark as handled to avoid repeated messages
        }
      }
      
      if (data) {
        data->gpsLat = gps.location.isValid() ? gps.location.lat() : 0;
        data->gpsLon = gps.location.isValid() ? gps.location.lng() : 0;
        data->gpsAlt = gps.altitude.isValid() ? gps.altitude.meters() : 0;
        data->gpsSpeed = gps.speed.isValid() ? gps.speed.kmph() : 0;
        data->gpsSatellites = gps.satellites.isValid() ? gps.satellites.value() : 0;
      }
    }
  }
  if (showSatellites && showTop3 && millis() - lastSatDisplay > SAT_DISPLAY_INTERVAL) {
    if (newSatData) {
      displayTop3Satellites();
      newSatData = false;
    }
    lastSatDisplay = millis();
  }
  if (millis() > 5000 && gps.charsProcessed() < 10) {
    Serial.println("ERROR: GPS not detected - check wiring!");
    unsigned long errorStart = millis();
    bool errorFlag = true;
    while (millis() - errorStart < 10000) {
      digitalWrite(LED1_PIN, (millis() / 250) % 2);
      delay(50);
    }
    errorFlag = false;
    digitalWrite(LED1_PIN, LOW);
    return;
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
  if (Serial.available()) {
    char c = Serial.read();
    switch (c) {
      case 'r':
        showRawData = !showRawData;
        Serial.print("Raw data: ");
        Serial.println(showRawData ? "ON" : "OFF");
        break;
      case 's':
        showSatellites = !showSatellites;
        Serial.print("Satellite info: ");
        Serial.println(showSatellites ? "ON" : "OFF");
        break;
      case 't':
        showTop3 = !showTop3;
        Serial.print("Top 3 display: ");
        Serial.println(showTop3 ? "ON" : "OFF");
        if (showTop3 && newSatData) {
          displayTop3Satellites();
          newSatData = false;
        }
        break;
      case 'v':
        displayAllSatellites();
        break;
      case 'd':
        displayStatus();
        if (showSatellites && showTop3 && newSatData) {
          displayTop3Satellites();
          newSatData = false;
        }
        break;
    }
  }
}
static void kalmanUpdate(double latMeasurement, double lonMeasurement) {
  variance += PROCESS_NOISE;
  const double K = variance / (variance + MEASUREMENT_NOISE);
  latFiltered += K * (latMeasurement - latFiltered);
  lonFiltered += K * (lonMeasurement - lonFiltered);
  variance *= (1.0 - K);
}
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
  kalmanUpdate(lat, lon);
  updatePositionBuffer(latFiltered, lonFiltered);
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
  Serial.println("SYS PRN Elev Azim SNR Qual Used");
  Serial.println("-------------------------------");
#endif
  if (satCount == 0) {
#ifdef SC_DEBUG_GPS
    Serial.println("No satellites detected");
#endif
    return;
  }
  int validCount = 0;
  for (int i = 0; i < satCount; i++) {
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
      Serial.print("  ");
      Serial.println(satellites[i].active ? "[YES]" : "[NO]");
      validCount++;
    }
  }
#ifdef SC_DEBUG_GPS
  Serial.print("Total satellites: ");
  Serial.println(validCount);
  Serial.print("Last update: ");
  Serial.print(millis() / 1000);
  Serial.println(" seconds ago");
#endif
  newGSVData = false;
} 