/*
  ThingProperties.h - Arduino IoT Cloud properties file

  This file has been generated automatically by the Arduino IoT Cloud.
  Do not modify this file - it will be overwritten by the Arduino IoT Cloud.
*/

#ifndef THINGPROPERTIES_H
#define THINGPROPERTIES_H

#include "ArduinoIoTCloud.h"
#include "arduino_secrets.h"

// Cloud ID obtained from https://app.arduino.cc/things
const char DEVICE_LOGIN_NAME[]  = "35ac3b22-4dda-44b2-acbb-5be24dcd48bd";

// Your device credentials
const char SSID[]               = SECRET_SSID;    // Network SSID (name)
const char PASS[]               = SECRET_PASS;    // Network password (use for WPA, or use WEP key)
const char DEVICE_KEY[]         = SECRET_DEVICE_KEY;    // Secret device password

// Cloud variables
// Note: cloudSerialData is declared globally in SmartCane.ino
extern String cloudSerialData; // Declare as external

void initProperties(){
  ArduinoCloud.setBoardId(DEVICE_LOGIN_NAME);
  ArduinoCloud.setSecretDeviceKey(DEVICE_KEY);
  ArduinoCloud.setThingId("35ac3b22-4dda-44b2-acbb-5be24dcd48bd");
  ArduinoCloud.addProperty(cloudSerialData, READWRITE, ON_CHANGE, NULL);
}

WiFiConnectionHandler ArduinoIoTPreferredConnection(SSID, PASS);

#endif // THINGPROPERTIES_H 