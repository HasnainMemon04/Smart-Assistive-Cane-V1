#pragma once
#ifndef SDCARD_MANAGER_H
#define SDCARD_MANAGER_H

#include <SD.h>
#include <SPI.h>
#include <ArduinoJson.h>
#include "Pins.h"

// SD Card SPI instance
extern SPIClass SPI_SD;

// SD Card Manager functions
bool SDCard_init();
void SDCard_listFiles(const char* dirname = "/", uint8_t levels = 1);
bool SDCard_writeFile(const char* path, const char* message);
String SDCard_readFile(const char* path);
bool SDCard_writeFile(const char* path, const uint8_t* data, size_t len);
bool SDCard_readFile(const char* path, uint8_t* data, size_t maxLen, size_t* bytesRead);
bool SDCard_deleteFile(const char* path);
bool SDCard_fileExists(const char* path);

// JSON configuration helpers
bool SDCard_saveJSON(const char* path, JsonDocument& doc);
bool SDCard_loadJSON(const char* path, JsonDocument& doc);

// Preferences replacement functions
bool SDCard_savePreferences(const char* namespace_name, const char* key, const String& value);
String SDCard_loadPreferences(const char* namespace_name, const char* key, const String& defaultValue = "");
bool SDCard_savePreferences(const char* namespace_name, const char* key, float value);
float SDCard_loadPreferences(const char* namespace_name, const char* key, float defaultValue = 0.0);
bool SDCard_savePreferences(const char* namespace_name, const char* key, bool value);
bool SDCard_loadPreferences(const char* namespace_name, const char* key, bool defaultValue = false);
bool SDCard_clearPreferences(const char* namespace_name);

#endif // SDCARD_MANAGER_H