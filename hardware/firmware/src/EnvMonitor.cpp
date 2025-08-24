#include "EnvMonitor.h"
#include "Pins.h"
#include "SensorHealth.h"
#include <DHT.h>

static DHT dht(DHTPIN, DHTTYPE);
static uint32_t lastRead = 0;
static bool indoorMode = true;
static char outputBuffer[150];

static float computeHeatIndex(float tempC, float humidity);
static float computeDewPoint(float tempC, float humidity);
static const char* assessTemperature(float tempC, bool indoor);
static const char* assessHumidity(float humidity, bool indoor);
static const char* assessHeatIndex(float heatIndex);
static const char* assessDewPoint(float dewPoint, float tempC);

void EnvMonitor_init() {
  dht.begin();
  lastRead = 0;
  
  // Test initial reading to verify sensor is working
  delay(2000); // DHT22 needs time to stabilize
  float testTemp = dht.readTemperature();
  float testHum = dht.readHumidity();
  
  if (isnan(testTemp) || isnan(testHum)) {
    SensorHealthManager::updateSensorHealth("dht22", SENSOR_INIT_FAILED, "DHT22 init failed");
  } else {
    SensorHealthManager::updateSensorHealth("dht22", SENSOR_OK, String(testTemp, 1).c_str());
  }
  
#ifdef SC_DEBUG_ENV
  Serial.println("[EnvMonitor] Initialized");
#endif
}

void EnvMonitor_update(SensorData* data) {
  uint32_t now = millis();
  if (now - lastRead < 2000) return;
  lastRead = now;

  float temp = dht.readTemperature();
  float hum = dht.readHumidity();
  if (isnan(temp) || isnan(hum)) {
    temp = dht.readTemperature();
    hum = dht.readHumidity();
  }
  if (isnan(temp) || isnan(hum)) {
    Serial.println("[EnvMonitor] Sensor error");
    SensorHealthManager::updateSensorHealth("dht22", SENSOR_ERROR, nullptr, "DHT22 read failed");
    return;
  }
  
  // Report successful reading
  SensorHealthManager::updateSensorHealth("dht22", SENSOR_OK, String(temp, 1).c_str());
  float heatIndex = computeHeatIndex(temp, hum);
  float dewPoint = computeDewPoint(temp, hum);
  const char* tempAssessment = assessTemperature(temp, indoorMode);
  const char* humidityAssessment = assessHumidity(hum, indoorMode);
  const char* heatAssessment = assessHeatIndex(heatIndex);
  const char* dewAssessment = assessDewPoint(dewPoint, temp);
  snprintf(outputBuffer, sizeof(outputBuffer),
    "[Env] %.1fC | %.1f%% | %s | Feels: %.1fC (%s) | Dew: %.1fC\n",
    temp, hum, indoorMode ? "Indoor" : "Outdoor", heatIndex, heatAssessment, dewPoint);
  Serial.print(outputBuffer);
  if (data) {
    data->temperature = temp;
    data->humidity = hum;
    data->heatIndex = heatIndex;
    data->dewPoint = dewPoint;
  }
}

// --- Helper functions (from v1) ---
static float computeHeatIndex(float tempC, float humidity) {
  float tempF = (tempC * 1.8) + 32;
  float hiF = -42.379 + 2.04901523*tempF + 10.14333127*humidity
    - 0.22475541*tempF*humidity - 0.00683783*tempF*tempF
    - 0.05481717*humidity*humidity
    + 0.00122874*tempF*tempF*humidity
    + 0.00085282*tempF*humidity*humidity
    - 0.00000199*tempF*tempF*humidity*humidity;
  return (hiF - 32) * 5/9;
}
static float computeDewPoint(float tempC, float humidity) {
  float a = 17.62, b = 243.12;
  float gamma = log(humidity/100) + (a*tempC)/(b + tempC);
  return (b * gamma) / (a - gamma);
}
static const char* assessTemperature(float tempC, bool indoor) {
  if (indoor) {
    if (tempC < 16) return "Too Cold!";
    if (tempC < 20) return "Chilly";
    if (tempC < 24) return "Comfortable";
    if (tempC < 28) return "Warm";
    return "Hot!";
  } else {
    if (tempC < 0) return "Freezing!";
    if (tempC < 10) return "Cold";
    if (tempC < 20) return "Cool";
    if (tempC < 30) return "Pleasant";
    if (tempC < 35) return "Warm";
    return "Scorching!";
  }
}
static const char* assessHumidity(float humidity, bool indoor) {
  if (indoor) {
    if (humidity < 30) return "Too Dry";
    if (humidity < 50) return "Ideal";
    if (humidity < 60) return "Humid";
    return "Very Humid";
  } else {
    if (humidity < 30) return "Arid";
    if (humidity < 60) return "Comfortable";
    if (humidity < 80) return "Humid";
    return "Tropical";
  }
}
static const char* assessHeatIndex(float heatIndex) {
  if (heatIndex < 27) return "Comfortable";
  if (heatIndex < 32) return "Warm";
  if (heatIndex < 41) return "Hot";
  if (heatIndex < 54) return "Extreme";
  return "Dangerous";
}
static const char* assessDewPoint(float dewPoint, float tempC) {
  float spread = tempC - dewPoint;
  if (spread < 2.5) return dewPoint > 0 ? "Fog likely" : "Frost possible";
  if (spread < 4) return "Slippery possible";
  if (dewPoint > 16) return "Muggy";
  return "Dry";
}