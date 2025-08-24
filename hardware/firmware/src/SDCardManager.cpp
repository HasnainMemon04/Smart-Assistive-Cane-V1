#include "SDCardManager.h"
#include <FS.h>

// Create HSPI instance for SD card
SPIClass SPI_SD(HSPI);

bool SDCard_init() {
  // Initialize HSPI with custom pins
  SPI_SD.begin(SD_SCK, SD_MISO, SD_MOSI, SD_CS);
  
  // Initialize SD card
  if (!SD.begin(SD_CS, SPI_SD)) {
    Serial.println("❌ SD Card initialization failed!");
    Serial.println("   Check connections:");
    Serial.println("   CS: Pin 7, SCK: Pin 12, MOSI: Pin 6, MISO: Pin 4");
    return false;
  }
  
  uint8_t cardType = SD.cardType();
  if (cardType == CARD_NONE) {
    Serial.println("❌ No SD card attached");
    return false;
  }
  
  Serial.print("✅ SD Card initialized. Type: ");
  if (cardType == CARD_MMC) {
    Serial.println("MMC");
  } else if (cardType == CARD_SD) {
    Serial.println("SDSC");
  } else if (cardType == CARD_SDHC) {
    Serial.println("SDHC");
  } else {
    Serial.println("UNKNOWN");
  }
  
  uint64_t cardSize = SD.cardSize() / (1024 * 1024);
  Serial.printf("   Card Size: %lluMB\n", cardSize);
  
  // Create necessary directories
  if (!SD.exists("/config")) {
    SD.mkdir("/config");
    Serial.println("📁 Created /config directory");
  }
  
  if (!SD.exists("/data")) {
    SD.mkdir("/data");
    Serial.println("📁 Created /data directory");
  }
  
  return true;
}

void SDCard_listFiles(const char* dirname, uint8_t levels) {
  Serial.printf("📂 Listing directory: %s\n", dirname);
  
  File root = SD.open(dirname);
  if (!root) {
    Serial.println("❌ Failed to open directory");
    return;
  }
  
  if (!root.isDirectory()) {
    Serial.println("❌ Not a directory");
    return;
  }
  
  File file = root.openNextFile();
  while (file) {
    if (file.isDirectory()) {
      Serial.print("📁 DIR: ");
      Serial.println(file.name());
      if (levels > 1) {
        SDCard_listFiles(file.path(), levels - 1);
      }
    } else {
      Serial.print("📄 FILE: ");
      Serial.print(file.name());
      Serial.print(" (Size: ");
      Serial.print(file.size());
      Serial.println(" bytes)");
    }
    file = root.openNextFile();
  }
  file.close();
  root.close();
}

bool SDCard_writeFile(const char* path, const char* message) {
  File file = SD.open(path, FILE_WRITE);
  if (!file) {
    Serial.printf("❌ Failed to open file for writing: %s\n", path);
    return false;
  }
  
  if (file.print(message)) {
    Serial.printf("✅ File written: %s\n", path);
    file.close();
    return true;
  } else {
    Serial.printf("❌ Write failed: %s\n", path);
    file.close();
    return false;
  }
}

String SDCard_readFile(const char* path) {
  File file = SD.open(path);
  if (!file) {
    Serial.printf("❌ Failed to open file for reading: %s\n", path);
    return String();
  }
  
  String content = "";
  while (file.available()) {
    content += (char)file.read();
  }
  file.close();
  
  Serial.printf("✅ File read: %s (%d bytes)\n", path, content.length());
  return content;
}

bool SDCard_deleteFile(const char* path) {
  if (SD.remove(path)) {
    Serial.printf("✅ File deleted: %s\n", path);
    return true;
  } else {
    Serial.printf("❌ Delete failed: %s\n", path);
    return false;
  }
}

bool SDCard_writeFile(const char* path, const uint8_t* data, size_t len) {
  File file = SD.open(path, FILE_WRITE);
  if (!file) {
    Serial.printf("❌ Failed to open file for binary writing: %s\n", path);
    return false;
  }
  
  size_t written = file.write(data, len);
  file.close();
  
  if (written == len) {
    Serial.printf("✅ Binary file written: %s (%d bytes)\n", path, len);
    return true;
  } else {
    Serial.printf("❌ Binary write failed: %s (wrote %d of %d bytes)\n", path, written, len);
    return false;
  }
}

bool SDCard_readFile(const char* path, uint8_t* data, size_t maxLen, size_t* bytesRead) {
  File file = SD.open(path);
  if (!file) {
    Serial.printf("❌ Failed to open file for binary reading: %s\n", path);
    if (bytesRead) *bytesRead = 0;
    return false;
  }
  
  size_t fileSize = file.size();
  size_t toRead = (fileSize < maxLen) ? fileSize : maxLen;
  size_t actualRead = file.readBytes((char*)data, toRead);
  file.close();
  
  if (bytesRead) *bytesRead = actualRead;
  
  if (actualRead > 0) {
    Serial.printf("✅ Binary file read: %s (%d bytes)\n", path, actualRead);
    return true;
  } else {
    Serial.printf("❌ Binary read failed: %s\n", path);
    return false;
  }
}

bool SDCard_fileExists(const char* path) {
  return SD.exists(path);
}

bool SDCard_saveJSON(const char* path, JsonDocument& doc) {
  File file = SD.open(path, FILE_WRITE);
  if (!file) {
    Serial.printf("❌ Failed to open JSON file for writing: %s\n", path);
    return false;
  }
  
  if (serializeJson(doc, file) == 0) {
    Serial.printf("❌ Failed to write JSON: %s\n", path);
    file.close();
    return false;
  }
  
  file.close();
  Serial.printf("✅ JSON saved: %s\n", path);
  return true;
}

bool SDCard_loadJSON(const char* path, JsonDocument& doc) {
  File file = SD.open(path);
  if (!file) {
    Serial.printf("ℹ️ JSON file not found: %s\n", path);
    return false;
  }
  
  DeserializationError error = deserializeJson(doc, file);
  file.close();
  
  if (error) {
    Serial.printf("❌ Failed to parse JSON: %s - %s\n", path, error.c_str());
    return false;
  }
  
  Serial.printf("✅ JSON loaded: %s\n", path);
  return true;
}

// Preferences replacement functions
bool SDCard_savePreferences(const char* namespace_name, const char* key, const String& value) {
  StaticJsonDocument<512> doc;
  String configPath = String("/config/") + namespace_name + ".json";
  
  // Load existing config if it exists
  SDCard_loadJSON(configPath.c_str(), doc);
  
  // Update the key
  doc[key] = value;
  
  // Save back to file
  return SDCard_saveJSON(configPath.c_str(), doc);
}

String SDCard_loadPreferences(const char* namespace_name, const char* key, const String& defaultValue) {
  StaticJsonDocument<512> doc;
  String configPath = String("/config/") + namespace_name + ".json";
  
  if (!SDCard_loadJSON(configPath.c_str(), doc)) {
    return defaultValue;
  }
  
  return doc[key] | defaultValue;
}

bool SDCard_savePreferences(const char* namespace_name, const char* key, float value) {
  StaticJsonDocument<512> doc;
  String configPath = String("/config/") + namespace_name + ".json";
  
  // Load existing config if it exists
  SDCard_loadJSON(configPath.c_str(), doc);
  
  // Update the key
  doc[key] = value;
  
  // Save back to file
  return SDCard_saveJSON(configPath.c_str(), doc);
}

float SDCard_loadPreferences(const char* namespace_name, const char* key, float defaultValue) {
  StaticJsonDocument<512> doc;
  String configPath = String("/config/") + namespace_name + ".json";
  
  if (!SDCard_loadJSON(configPath.c_str(), doc)) {
    return defaultValue;
  }
  
  return doc[key] | defaultValue;
}

bool SDCard_savePreferences(const char* namespace_name, const char* key, bool value) {
  StaticJsonDocument<512> doc;
  String configPath = String("/config/") + namespace_name + ".json";
  
  // Load existing config if it exists
  SDCard_loadJSON(configPath.c_str(), doc);
  
  // Update the key
  doc[key] = value;
  
  // Save back to file
  return SDCard_saveJSON(configPath.c_str(), doc);
}

bool SDCard_loadPreferences(const char* namespace_name, const char* key, bool defaultValue) {
  StaticJsonDocument<512> doc;
  String configPath = String("/config/") + namespace_name + ".json";
  
  if (!SDCard_loadJSON(configPath.c_str(), doc)) {
    return defaultValue;
  }
  
  return doc[key] | defaultValue;
}

bool SDCard_clearPreferences(const char* namespace_name) {
  String configPath = String("/config/") + namespace_name + ".json";
  return SDCard_deleteFile(configPath.c_str());
}