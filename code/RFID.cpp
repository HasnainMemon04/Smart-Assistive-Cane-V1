#include "RFID.h"
#include "Pins.h"
#include <SPI.h>
#include <MFRC522.h>
#include <string.h>
#include <SPIFFS.h>

#ifndef VSPI
#define VSPI 1
#endif

#define POLL_INTERVAL 15
#define ANTENNA_GAIN MFRC522::RxGain_48dB
#define SPI_SPEED 4000000   // Reduced for reliability

static MFRC522 mfrc522(RFID_CS, RFID_RST);
static char lastUID[30] = {0};
static bool tagPresent = false;
static unsigned long lastPoll = 0;

// Indoor zoning system variables
static char roomCards[MAX_ROOMS][30] = {0};  // Store card UIDs for each room
static char roomNames[MAX_ROOMS + 1][ROOM_NAME_LENGTH] = {
  "Lobby", "Room 1", "Room 2", "Room 3", "Room 4"
};
static uint8_t currentRoom = 0;
static bool roomChanged = false;
static char lastScannedCard[30] = {0};  // Track last scanned card for exit logic

// Auto registration system variables
static bool autoRegistrationActive = false;
static uint8_t nextRoomToRegister = 1;
static char pendingCardUID[30] = {0};
static bool cardWaitingForRoom = false;
static char pendingLocationName[ROOM_NAME_LENGTH] = {0};
static bool waitingForLocationName = false;
static bool sensorsDisabled = false;
static bool singleCommandMode = false;

// Enhanced features from RFID text file
// Zone detection states
enum ZoneState {
  ZONE_UNKNOWN = 0,
  ZONE_LOBBY = 1,
  ZONE_ROOM_1 = 2,
  ZONE_ROOM_2 = 3,
  ZONE_ROOM_3 = 4,
  ZONE_ROOM_4 = 5,
  ZONE_TRANSITION = 6
};

static ZoneState currentZone = ZONE_LOBBY;
static ZoneState previousZone = ZONE_LOBBY;
static uint32_t zoneEntryTime = 0;
static uint32_t zoneExitTime = 0;

// Zone statistics
struct ZoneStats {
  uint32_t totalTimeInZone[7]; // Time spent in each zone
  uint32_t zoneEntryCount[7];  // Number of entries to each zone
  uint32_t lastZoneEntry[7];   // Last entry time for each zone
};

static ZoneStats zoneStats;

// Room data structure for persistence
struct RoomData {
  char cardUID[30];
  char roomName[ROOM_NAME_LENGTH];
  bool isRegistered;
  uint8_t roomNumber;
};

// Global room data array
static RoomData roomDataArray[MAX_ROOMS];

// Non-blocking feedback system
static uint32_t feedbackStartTime = 0;
static bool feedbackActive = false;
static uint8_t feedbackStep = 0;
static const uint32_t FEEDBACK_DURATION = 200; // 200ms per step
static const uint8_t FEEDBACK_STEPS = 4; // 4 steps total

static void processNewTag();
static void triggerFeedback();
static void updateRoomLocation(const char* cardUID);
static uint8_t findRoomByCard(const char* cardUID);
static bool isExitAction(const char* cardUID);
static void handleAutoRegistration(const char* cardUID);
static void handleLocationNameInput(const char* locationName);
static void disableAllSensors();
static void enableAllSensors();
static void startSingleLocationRegistration();
static void handleSingleCommandInput(const char* input);

// Enhanced functions from RFID text file
static ZoneState getZoneFromRoom(uint8_t room);
static void logZoneExit(ZoneState zone);
static void logZoneEntry(ZoneState zone);
static void triggerZoneChangeFeedback();
static void triggerZoneFeedback(uint8_t pattern);
static void updateFeedback();
static void sendZoneDataToApp();

void RFID_init() {
  SPI.begin(SPI_SCK, 48, SPI_MOSI, RFID_CS);
  mfrc522.PCD_Init();
  mfrc522.PCD_SetAntennaGain(ANTENNA_GAIN);
  mfrc522.PCD_AntennaOn();
  mfrc522.PCD_WriteRegister(MFRC522::TModeReg, 0x80);
  mfrc522.PCD_WriteRegister(MFRC522::TPrescalerReg, 0x43);
  mfrc522.PCD_WriteRegister(MFRC522::TReloadRegL, 30);
  mfrc522.PCD_WriteRegister(MFRC522::TReloadRegH, 0x00);
  mfrc522.PCD_WriteRegister(MFRC522::RFCfgReg, 0x70);
  mfrc522.PCD_WriteRegister(MFRC522::TxASKReg, 0x40);
  mfrc522.PCD_WriteRegister(MFRC522::ComIrqReg, 0x7F);
  
  // Initialize vibration motors
  pinMode(VIB1_PIN, OUTPUT);
  pinMode(VIB2_PIN, OUTPUT);
  digitalWrite(VIB1_PIN, LOW);
  digitalWrite(VIB2_PIN, LOW);
  
  // Initialize SPIFFS for room data persistence
  if (!SPIFFS.begin(true)) {
    Serial.println("❌ SPIFFS initialization failed");
  } else {
    Serial.println("✅ SPIFFS initialized for room data persistence");
  }
  
  // Initialize indoor zoning system
  RFID_setupZoning();
  
  // Load saved room data
  RFID_loadRoomData();
  
#ifdef SC_DEBUG_RFID
  Serial.println("\n[RC522] Ultra-Sensitive Mode Ready");
  Serial.println("[ZONING] Indoor zoning system initialized");
#endif
}

void RFID_update(SensorData* data) {
  // Update non-blocking feedback
  updateFeedback();
  
  // If sensors are disabled during room registration, only process RFID
  if (sensorsDisabled) {
    if (millis() - lastPoll < POLL_INTERVAL) return;
    lastPoll = millis();
    bool currentTagState = mfrc522.PICC_IsNewCardPresent();
    if (currentTagState && !tagPresent) {
      if (mfrc522.PICC_ReadCardSerial()) {
        processNewTag();
        tagPresent = true;
        
        // Handle auto registration if active
        if (autoRegistrationActive) {
          handleAutoRegistration(lastUID);
        }
        
        // Handle single command mode
        if (singleCommandMode) {
          handleSingleCommandInput(lastUID);
        }
        
        if (data) {
          strncpy(data->rfidUID, lastUID, sizeof(data->rfidUID)-1);
          data->rfidUID[sizeof(data->rfidUID)-1] = '\0';
          
          // Update room location for app sensing (even during registration)
          RFID_updateRoomLocationWithZones(lastUID);
          data->currentRoom = currentRoom;
          strncpy(data->currentRoomName, roomNames[currentRoom], ROOM_NAME_LENGTH-1);
          data->currentRoomName[ROOM_NAME_LENGTH-1] = '\0';
          data->roomChanged = roomChanged;
          roomChanged = false; // Reset flag after updating
        }
      }
    } else if (!currentTagState && tagPresent) {
      tagPresent = false;
      memset(lastUID, 0, sizeof(lastUID));
      mfrc522.PCD_WriteRegister(MFRC522::ComIrqReg, 0x7F);
      if (data) {
        data->rfidUID[0] = '\0';
      }
    }
    return; // Exit early - don't process other sensors
  }
  
  // Normal sensor processing when not in registration mode
  if (millis() - lastPoll < POLL_INTERVAL) return;
  lastPoll = millis();
  bool currentTagState = mfrc522.PICC_IsNewCardPresent();
  if (currentTagState && !tagPresent) {
    if (mfrc522.PICC_ReadCardSerial()) {
      processNewTag();
      tagPresent = true;
      
      // Handle auto registration if active
      if (autoRegistrationActive) {
        handleAutoRegistration(lastUID);
      }
      
      if (data) {
        strncpy(data->rfidUID, lastUID, sizeof(data->rfidUID)-1);
        data->rfidUID[sizeof(data->rfidUID)-1] = '\0';
        
        // Update room location with enhanced zone tracking
        RFID_updateRoomLocationWithZones(lastUID);
        data->currentRoom = currentRoom;
        strncpy(data->currentRoomName, roomNames[currentRoom], ROOM_NAME_LENGTH-1);
        data->currentRoomName[ROOM_NAME_LENGTH-1] = '\0';
        data->roomChanged = roomChanged;
        roomChanged = false; // Reset flag after updating
      }
    }
  } else if (!currentTagState && tagPresent) {
    tagPresent = false;
    memset(lastUID, 0, sizeof(lastUID));
    mfrc522.PCD_WriteRegister(MFRC522::ComIrqReg, 0x7F);
    if (data) {
      data->rfidUID[0] = '\0';
      // Don't clear room immediately - keep last known room
    }
  }
}

// ============= Single Command Registration System =============

static void startSingleLocationRegistration() {
  Serial.println("DEBUG: Starting single location registration...");
  
  singleCommandMode = true;
  cardWaitingForRoom = true;
  waitingForLocationName = false;
  memset(pendingCardUID, 0, sizeof(pendingCardUID));
  memset(pendingLocationName, 0, sizeof(pendingLocationName));
  
  // Disable all sensors
  disableAllSensors();
  
  Serial.println("\n🎯 SINGLE COMMAND LOCATION REGISTRATION");
  Serial.println("📱 Please scan the RFID card for this location...");
  Serial.println("🔇 All sensors and serial data disabled - only RFID active");
  
  Serial.println("DEBUG: Single location registration started successfully");
}

static void handleSingleCommandInput(const char* input) {
  if (cardWaitingForRoom && !waitingForLocationName) {
    // Card was scanned - store it and ask for location name
    strncpy(pendingCardUID, input, sizeof(pendingCardUID)-1);
    pendingCardUID[sizeof(pendingCardUID)-1] = '\0';
    cardWaitingForRoom = false;
    waitingForLocationName = true;
    
    Serial.printf("📱 Card scanned: %s\n", input);
    Serial.println("🏠 Please type the location name (e.g., 'kitchen', 'bedroom', 'office'):");
  } else if (waitingForLocationName) {
    // Location name was entered - complete the registration
    String locationName = String(input);
    
    // Find an available room slot
    uint8_t availableRoom = 0;
    for (int i = 0; i < MAX_ROOMS; i++) {
      if (strlen(roomCards[i]) == 0) {
        availableRoom = i + 1;
        break;
      }
    }
    
    if (availableRoom == 0) {
      Serial.println("❌ All room slots are full. Use 'clearrooms' to clear existing registrations.");
      // Re-enable sensors and exit
      singleCommandMode = false;
      cardWaitingForRoom = false;
      waitingForLocationName = false;
      enableAllSensors();
      return;
    }
    
    // Register the card with the location name
    RFID_addRoomCard(availableRoom, pendingCardUID);
    
    // Update the room name
    strncpy(roomNames[availableRoom], locationName.c_str(), ROOM_NAME_LENGTH-1);
    roomNames[availableRoom][ROOM_NAME_LENGTH-1] = '\0';
    
    Serial.printf("✅ Location registered: %s (Room %d) - Card: %s\n", 
                  locationName.c_str(), availableRoom, pendingCardUID);
    
    // Complete the process - re-enable sensors and exit
    singleCommandMode = false;
    cardWaitingForRoom = false;
    waitingForLocationName = false;
    memset(pendingCardUID, 0, sizeof(pendingCardUID));
    memset(pendingLocationName, 0, sizeof(pendingLocationName));
    
    // Re-enable all sensors
    enableAllSensors();
    
    Serial.println("✅ Single command registration completed!");
    Serial.println("🔊 All sensors and serial data re-enabled - normal operation resumed");
  }
}

// ============= Auto Registration System =============

void RFID_startSingleLocationRegistration() {
  startSingleLocationRegistration();
}

void RFID_startAutoRegistration() {
  autoRegistrationActive = true;
  nextRoomToRegister = 1;
  cardWaitingForRoom = false;
  waitingForLocationName = false;
  memset(pendingCardUID, 0, sizeof(pendingCardUID));
  memset(pendingLocationName, 0, sizeof(pendingLocationName));
  
  // Disable all sensors during registration
  disableAllSensors();
  
  Serial.println("\n🎯 AUTO REGISTRATION MODE STARTED");
  Serial.println("📋 Instructions:");
  Serial.println("   1. Type 'addroom' to start adding a location");
  Serial.println("   2. Scan the RFID card for that location");
  Serial.println("   3. Type the location name (e.g., 'kitchen', 'bedroom')");
  Serial.println("   4. Repeat for other locations");
  Serial.println("   5. Type 'done' when finished");
  Serial.println("   6. Type 'cancel' to stop auto registration");
  Serial.println("\n🔍 Type 'addroom' to start adding a location...");
  Serial.println("⚠️ All sensors disabled during registration for clean setup");
}

void RFID_stopAutoRegistration() {
  autoRegistrationActive = false;
  cardWaitingForRoom = false;
  waitingForLocationName = false;
  memset(pendingCardUID, 0, sizeof(pendingCardUID));
  memset(pendingLocationName, 0, sizeof(pendingLocationName));
  
  // Re-enable all sensors
  enableAllSensors();
  
  Serial.println("❌ Auto registration mode stopped");
  Serial.println("✅ All sensors re-enabled");
}

bool RFID_isAutoRegistrationActive() {
  return autoRegistrationActive;
}

void RFID_processAutoRegistration(const char* command) {
  if (!autoRegistrationActive && !singleCommandMode) return;
  
  String cmd = String(command);
  cmd.toLowerCase();
  
  if (cmd == "cancel") {
    if (autoRegistrationActive) {
      RFID_stopAutoRegistration();
    }
    if (singleCommandMode) {
      singleCommandMode = false;
      cardWaitingForRoom = false;
      waitingForLocationName = false;
      enableAllSensors();
      Serial.println("❌ Single command registration cancelled");
    }
    return;
  }
  
  if (cmd == "done") {
    if (autoRegistrationActive) {
      RFID_stopAutoRegistration();
      Serial.println("✅ Auto registration completed!");
      RFID_printRoomCards();
    }
    return;
  }
  
  // Handle single command mode
  if (singleCommandMode) {
    if (waitingForLocationName) {
      handleSingleCommandInput(cmd.c_str());
    } else {
      Serial.println("❌ Please scan a card first, then type the location name.");
    }
    return;
  }
  
  // Handle auto registration mode
  if (autoRegistrationActive) {
    if (cmd == "addroom") {
      if (!cardWaitingForRoom && !waitingForLocationName) {
        // Start new room registration
        cardWaitingForRoom = true;
        waitingForLocationName = false;
        memset(pendingCardUID, 0, sizeof(pendingCardUID));
        memset(pendingLocationName, 0, sizeof(pendingLocationName));
        
        Serial.println("📱 Please scan the RFID card for this location...");
        Serial.println("🔇 All sensors disabled - only RFID active");
      } else {
        Serial.println("❌ Already waiting for card scan or location name. Please complete current registration first.");
      }
      return;
    }
    
    // If waiting for location name, process it
    if (waitingForLocationName && strlen(pendingCardUID) > 0) {
      handleLocationNameInput(cmd.c_str());
      return;
    }
    
    // If waiting for card but got text, it might be a location name
    if (cardWaitingForRoom && strlen(pendingCardUID) > 0) {
      Serial.println("❌ Please scan a card first, then type the location name.");
      return;
    }
    
    Serial.println("❌ Unknown command. Use 'addroom', 'done', or 'cancel'");
  }
}

static void handleAutoRegistration(const char* cardUID) {
  if (!autoRegistrationActive) return;
  
  if (cardWaitingForRoom && !waitingForLocationName) {
    // Store the card UID and wait for location name
    strncpy(pendingCardUID, cardUID, sizeof(pendingCardUID)-1);
    pendingCardUID[sizeof(pendingCardUID)-1] = '\0';
    cardWaitingForRoom = false;
    waitingForLocationName = true;
    
    Serial.printf("📱 Card scanned: %s\n", cardUID);
    Serial.println("🏠 Please type the location name (e.g., 'kitchen', 'bedroom', 'office'):");
  } else if (waitingForLocationName) {
    Serial.println("⚠️ Already waiting for location name. Please type the location name first.");
  } else {
    Serial.println("⚠️ Not in registration mode. Type 'addroom' to start adding a location.");
  }
}

static void handleLocationNameInput(const char* locationName) {
  if (!waitingForLocationName || strlen(pendingCardUID) == 0) {
    Serial.println("❌ No card scanned yet. Please scan a card first.");
    return;
  }
  
  // Find an available room slot
  uint8_t availableRoom = 0;
  for (int i = 0; i < MAX_ROOMS; i++) {
    if (strlen(roomCards[i]) == 0) {
      availableRoom = i + 1;
      break;
    }
  }
  
  if (availableRoom == 0) {
    Serial.println("❌ All room slots are full. Use 'clearrooms' to clear existing registrations.");
    waitingForLocationName = false;
    memset(pendingCardUID, 0, sizeof(pendingCardUID));
    return;
  }
  
  // Register the card with the location name
  RFID_addRoomCard(availableRoom, pendingCardUID);
  
  // Update the room name
  strncpy(roomNames[availableRoom], locationName, ROOM_NAME_LENGTH-1);
  roomNames[availableRoom][ROOM_NAME_LENGTH-1] = '\0';
  
  Serial.printf("✅ Location registered: %s (Room %d) - Card: %s\n", 
                locationName, availableRoom, pendingCardUID);
  
  // Reset for next registration
  waitingForLocationName = false;
  cardWaitingForRoom = false;
  memset(pendingCardUID, 0, sizeof(pendingCardUID));
  memset(pendingLocationName, 0, sizeof(pendingLocationName));
  
  Serial.println("🔍 Type 'addroom' to add another location, or 'done' to finish...");
}

// ============= Indoor Zoning System =============

void RFID_setupZoning() {
  // Clear all room cards
  memset(roomCards, 0, sizeof(roomCards));
  
  // Set default room names (already initialized in static declaration)
  // roomNames array is already initialized with correct values
  
  currentRoom = 0;
  roomChanged = false;
  memset(lastScannedCard, 0, sizeof(lastScannedCard));
  
  // Auto-register room cards (uncomment and modify with your card UIDs)
  // RFID_addRoomCard(1, "UID:12 34 56 78");  // Room 1 card
  // RFID_addRoomCard(2, "UID:AB CD EF 12");  // Room 2 card
  // RFID_addRoomCard(3, "UID:34 56 78 9A");  // Room 3 card
  // RFID_addRoomCard(4, "UID:BC DE F0 12");  // Room 4 card
  
  Serial.println("🏠 Indoor zoning system initialized");
  Serial.println("   Use RFID_addRoomCard() to register room cards");
  Serial.println("   Exit logic: Scan same card again to return to lobby");
  Serial.println("   Commands: 'rooms', 'addroom <room> <uid>', 'clearrooms'");
  Serial.println("   Auto registration: 'autoreg' to start guided setup");
}

void RFID_addRoomCard(uint8_t roomNumber, const char* cardUID) {
  if (roomNumber < 1 || roomNumber > MAX_ROOMS) {
    Serial.printf("❌ Invalid room number: %d (must be 1-%d)\n", roomNumber, MAX_ROOMS);
    return;
  }
  
  if (strlen(cardUID) == 0) {
    Serial.printf("❌ Invalid card UID for room %d\n", roomNumber);
    return;
  }
  
  strncpy(roomCards[roomNumber-1], cardUID, 29);
  roomCards[roomNumber-1][29] = '\0';
  
  Serial.printf("✅ Room %d card registered: %s\n", roomNumber, cardUID);
}

const char* RFID_getRoomName(uint8_t roomNumber) {
  if (roomNumber >= 0 && roomNumber <= MAX_ROOMS) {
    return roomNames[roomNumber];
  }
  return "Unknown";
}

void RFID_clearRoomCards() {
  memset(roomCards, 0, sizeof(roomCards));
  currentRoom = 0;
  roomChanged = false;
  memset(lastScannedCard, 0, sizeof(lastScannedCard));
  Serial.println("🗑️ All room cards cleared");
}

void RFID_printRoomCards() {
  Serial.println("\n🏠 Registered Room Cards:");
  for (int i = 0; i < MAX_ROOMS; i++) {
    if (strlen(roomCards[i]) > 0) {
      Serial.printf("   Room %d: %s\n", i+1, roomCards[i]);
    }
  }
  Serial.printf("   Current Room: %s\n", roomNames[currentRoom]);
}

// ============= Private Zoning Functions =============

static void updateRoomLocation(const char* cardUID) {
  // Check if this is an exit action (same card scanned again)
  if (isExitAction(cardUID)) {
    uint8_t oldRoom = currentRoom;
    currentRoom = 0; // Return to lobby
    roomChanged = true;
    
    // Only show room change messages when not in registration mode
    if (!sensorsDisabled) {
      Serial.printf("🚪 EXIT: Left %s → Lobby\n", roomNames[oldRoom]);
    }
    return;
  }
  
  // Normal room entry logic
  uint8_t newRoom = findRoomByCard(cardUID);
  
  if (newRoom != currentRoom) {
    uint8_t oldRoom = currentRoom;
    currentRoom = newRoom;
    roomChanged = true;
    
    // Only show room change messages when not in registration mode
    if (!sensorsDisabled) {
      if (newRoom > 0) {
        Serial.printf("🏠 ENTER: %s (Room %d)\n", roomNames[newRoom], newRoom);
      } else {
        Serial.println("🏠 Location: Unknown area");
      }
    }
  }
  
  // Store this card as last scanned for exit logic
  strncpy(lastScannedCard, cardUID, sizeof(lastScannedCard)-1);
  lastScannedCard[sizeof(lastScannedCard)-1] = '\0';
}

static uint8_t findRoomByCard(const char* cardUID) {
  for (int i = 0; i < MAX_ROOMS; i++) {
    if (strlen(roomCards[i]) > 0 && strcmp(roomCards[i], cardUID) == 0) {
      return i + 1; // Return room number (1-4)
    }
  }
  return 0; // No matching room found
}

static bool isExitAction(const char* cardUID) {
  // Check if this is the same card that was last scanned
  if (strlen(lastScannedCard) > 0 && strcmp(lastScannedCard, cardUID) == 0) {
    // Same card scanned again - treat as exit action
    return true;
  }
  return false;
}

// ============= Original RFID Functions =============

static void processNewTag() {
  char uidBuffer[30];
  char* ptr = uidBuffer;
  *ptr++ = 'U'; *ptr++ = 'I'; *ptr++ = 'D'; *ptr++ = ':';
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    *ptr++ = "0123456789ABCDEF"[mfrc522.uid.uidByte[i] >> 4];
    *ptr++ = "0123456789ABCDEF"[mfrc522.uid.uidByte[i] & 0x0F];
    *ptr++ = ' ';
  }
  *ptr = '\0';
  if (strcmp(uidBuffer, lastUID) != 0) {
    strcpy(lastUID, uidBuffer);
    
    // Only show detailed RFID info when not in registration mode
    if (!sensorsDisabled) {
    Serial.print("Detected: ");
    Serial.println(uidBuffer);
    MFRC522::PICC_Type piccType = mfrc522.PICC_GetType(mfrc522.uid.sak);
    Serial.print("Type: ");
    Serial.println(mfrc522.PICC_GetTypeName(piccType));
    }
    
    // Trigger synchronized feedback
    triggerFeedback();
  }
  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();
}

// ============= Synchronized Feedback Function =============
static void triggerFeedback() {
  // Get current feedback mode from SensorData
  extern SensorData sensorData;
  uint8_t feedbackMode = sensorData.feedbackMode;
  
  // RFID detection pattern: Double beep with SYNCHRONIZED vibration
  // Pattern: Buzzer ON-OFF-ON-OFF, Vibration ON-OFF-ON-OFF (200ms each)
  
  // First beep (buzzer ON, vibration ON)
  if (feedbackMode == FEEDBACK_MODE_BOTH || feedbackMode == FEEDBACK_MODE_BUZZER) {
    digitalWrite(BUZZER_PIN, HIGH);
  }
  if (feedbackMode == FEEDBACK_MODE_BOTH || feedbackMode == FEEDBACK_MODE_VIBRATION) {
    digitalWrite(VIB1_PIN, HIGH);
    digitalWrite(VIB2_PIN, HIGH);
  }
  delay(200);
  
  // First pause (buzzer OFF, vibration OFF)
  if (feedbackMode == FEEDBACK_MODE_BOTH || feedbackMode == FEEDBACK_MODE_BUZZER) {
    digitalWrite(BUZZER_PIN, LOW);
  }
  if (feedbackMode == FEEDBACK_MODE_BOTH || feedbackMode == FEEDBACK_MODE_VIBRATION) {
    digitalWrite(VIB1_PIN, LOW);
    digitalWrite(VIB2_PIN, LOW);
  }
  delay(200);
  
  // Second beep (buzzer ON, vibration ON)
  if (feedbackMode == FEEDBACK_MODE_BOTH || feedbackMode == FEEDBACK_MODE_BUZZER) {
    digitalWrite(BUZZER_PIN, HIGH);
  }
  if (feedbackMode == FEEDBACK_MODE_BOTH || feedbackMode == FEEDBACK_MODE_VIBRATION) {
    digitalWrite(VIB1_PIN, HIGH);
    digitalWrite(VIB2_PIN, HIGH);
  }
  delay(200);
  
  // Final pause (buzzer OFF, vibration OFF)
  if (feedbackMode == FEEDBACK_MODE_BOTH || feedbackMode == FEEDBACK_MODE_BUZZER) {
    digitalWrite(BUZZER_PIN, LOW);
  }
  if (feedbackMode == FEEDBACK_MODE_BOTH || feedbackMode == FEEDBACK_MODE_VIBRATION) {
    digitalWrite(VIB1_PIN, LOW);
    digitalWrite(VIB2_PIN, LOW);
  }
  delay(200);
  
  // Turn off everything
  if (feedbackMode == FEEDBACK_MODE_BOTH || feedbackMode == FEEDBACK_MODE_BUZZER) {
    digitalWrite(BUZZER_PIN, LOW);
  }
  if (feedbackMode == FEEDBACK_MODE_BOTH || feedbackMode == FEEDBACK_MODE_VIBRATION) {
    digitalWrite(VIB1_PIN, LOW);
    digitalWrite(VIB2_PIN, LOW);
  }
} 

// ============= Sensor Control Functions =============

static void disableAllSensors() {
  sensorsDisabled = true;
  Serial.println("🔇 All sensors disabled - only RFID active for registration");
  Serial.println("DEBUG: sensorsDisabled = true");
}

static void enableAllSensors() {
  sensorsDisabled = false;
  Serial.println("🔊 All sensors re-enabled");
  Serial.println("DEBUG: sensorsDisabled = false");
}

bool RFID_areSensorsDisabled() {
  // Only show debug occasionally to avoid spam
  static uint32_t lastDebugTime = 0;
  if (millis() - lastDebugTime > 5000) { // Every 5 seconds
    Serial.printf("DEBUG: RFID_areSensorsDisabled() = %s\n", sensorsDisabled ? "true" : "false");
    lastDebugTime = millis();
  }
  return sensorsDisabled;
}

// ============= Enhanced Features from RFID Text File =============

// Room persistence functions
void RFID_saveRoomData() {
  File file = SPIFFS.open("/rooms.dat", "w");
  if (file) {
    for (int i = 0; i < MAX_ROOMS; i++) {
      file.write((uint8_t*)&roomDataArray[i], sizeof(RoomData));
    }
    file.close();
    Serial.println("✅ Room data saved to storage");
  } else {
    Serial.println("❌ Failed to save room data");
  }
}

void RFID_loadRoomData() {
  File file = SPIFFS.open("/rooms.dat", "r");
  if (file) {
    for (int i = 0; i < MAX_ROOMS; i++) {
      if (file.read((uint8_t*)&roomDataArray[i], sizeof(RoomData)) == sizeof(RoomData)) {
        if (roomDataArray[i].isRegistered) {
          // Restore room registration
          RFID_addRoomCard(roomDataArray[i].roomNumber, roomDataArray[i].cardUID);
          strncpy(roomNames[roomDataArray[i].roomNumber], roomDataArray[i].roomName, ROOM_NAME_LENGTH-1);
          roomNames[roomDataArray[i].roomNumber][ROOM_NAME_LENGTH-1] = '\0';
        }
      }
    }
    file.close();
    Serial.println("✅ Room data loaded from storage");
  } else {
    Serial.println("ℹ️ No saved room data found");
  }
}

void RFID_addRoomCardWithSave(uint8_t roomNumber, const char* cardUID, const char* roomName) {
  // Add room card normally
  RFID_addRoomCard(roomNumber, cardUID);
  
  // Update room name
  if (roomName && strlen(roomName) > 0) {
    strncpy(roomNames[roomNumber], roomName, ROOM_NAME_LENGTH-1);
    roomNames[roomNumber][ROOM_NAME_LENGTH-1] = '\0';
  }
  
  // Save to persistent storage
  roomDataArray[roomNumber-1].roomNumber = roomNumber;
  strncpy(roomDataArray[roomNumber-1].cardUID, cardUID, sizeof(roomDataArray[roomNumber-1].cardUID)-1);
  roomDataArray[roomNumber-1].cardUID[sizeof(roomDataArray[roomNumber-1].cardUID)-1] = '\0';
  strncpy(roomDataArray[roomNumber-1].roomName, roomNames[roomNumber], sizeof(roomDataArray[roomNumber-1].roomName)-1);
  roomDataArray[roomNumber-1].roomName[sizeof(roomDataArray[roomNumber-1].roomName)-1] = '\0';
  roomDataArray[roomNumber-1].isRegistered = true;
  
  // Save to storage
  RFID_saveRoomData();
  
  Serial.printf("✅ Room %d saved: %s (%s)\n", roomNumber, roomNames[roomNumber], cardUID);
}

void RFID_clearRoomCardsWithSave() {
  // Clear room cards normally
  RFID_clearRoomCards();
  
  // Clear persistent storage
  memset(roomDataArray, 0, sizeof(roomDataArray));
  
  // Save empty data
  RFID_saveRoomData();
  
  Serial.println("🗑️ All room cards cleared and saved");
}

// Zone tracking functions
static ZoneState getZoneFromRoom(uint8_t room) {
  switch (room) {
    case 0: return ZONE_LOBBY;
    case 1: return ZONE_ROOM_1;
    case 2: return ZONE_ROOM_2;
    case 3: return ZONE_ROOM_3;
    case 4: return ZONE_ROOM_4;
    default: return ZONE_UNKNOWN;
  }
}

static void logZoneExit(ZoneState zone) {
  const char* zoneNames[] = {"Unknown", "Lobby", "Room 1", "Room 2", "Room 3", "Room 4", "Transition"};
  Serial.printf("🚪 EXIT ZONE: %s\n", zoneNames[zone]);
  
  // Send zone exit notification for mobile app
  extern SensorData sensorData;
  if (sensorData.feedbackMode != FEEDBACK_MODE_NONE) {
    Serial.printf("📱 Zone Exit: %s\n", zoneNames[zone]);
  }
}

static void logZoneEntry(ZoneState zone) {
  const char* zoneNames[] = {"Unknown", "Lobby", "Room 1", "Room 2", "Room 3", "Room 4", "Transition"};
  Serial.printf("🏠 ENTER ZONE: %s\n", zoneNames[zone]);
  
  // Send zone entry notification for mobile app
  extern SensorData sensorData;
  if (sensorData.feedbackMode != FEEDBACK_MODE_NONE) {
    Serial.printf("📱 Zone Entry: %s\n", zoneNames[zone]);
  }
}

void RFID_updateZoneTracking() {
  ZoneState newZone = getZoneFromRoom(currentRoom);
  
  if (newZone != currentZone) {
    // Zone change detected
    previousZone = currentZone;
    zoneExitTime = millis();
    
    // Log zone exit
    logZoneExit(currentZone);
    
    // Update to new zone
    currentZone = newZone;
    zoneEntryTime = millis();
    
    // Log zone entry
    logZoneEntry(currentZone);
    
    // Trigger zone change feedback
    triggerZoneChangeFeedback();
  }
}

static void triggerZoneChangeFeedback() {
  // Different feedback patterns for different zones
  uint8_t feedbackPattern = 0;
  
  switch (currentZone) {
    case ZONE_LOBBY:
      feedbackPattern = 1; // Single beep
      break;
    case ZONE_ROOM_1:
    case ZONE_ROOM_2:
    case ZONE_ROOM_3:
    case ZONE_ROOM_4:
      feedbackPattern = 2; // Double beep
      break;
    case ZONE_UNKNOWN:
      feedbackPattern = 3; // Triple beep (warning)
      break;
    default:
      feedbackPattern = 1;
      break;
  }
  
  // Trigger zone-specific feedback
  triggerZoneFeedback(feedbackPattern);
}

static void triggerZoneFeedback(uint8_t pattern) {
  extern SensorData sensorData;
  uint8_t feedbackMode = sensorData.feedbackMode;
  
  switch (pattern) {
    case 1: // Single beep
      if (feedbackMode == FEEDBACK_MODE_BOTH || feedbackMode == FEEDBACK_MODE_BUZZER) {
        digitalWrite(BUZZER_PIN, HIGH);
        delay(100);
        digitalWrite(BUZZER_PIN, LOW);
      }
      if (feedbackMode == FEEDBACK_MODE_BOTH || feedbackMode == FEEDBACK_MODE_VIBRATION) {
        digitalWrite(VIB1_PIN, HIGH);
        digitalWrite(VIB2_PIN, HIGH);
        delay(100);
        digitalWrite(VIB1_PIN, LOW);
        digitalWrite(VIB2_PIN, LOW);
      }
      break;
      
    case 2: // Double beep
      for (int i = 0; i < 2; i++) {
        if (feedbackMode == FEEDBACK_MODE_BOTH || feedbackMode == FEEDBACK_MODE_BUZZER) {
          digitalWrite(BUZZER_PIN, HIGH);
          delay(80);
          digitalWrite(BUZZER_PIN, LOW);
          delay(120);
        }
        if (feedbackMode == FEEDBACK_MODE_BOTH || feedbackMode == FEEDBACK_MODE_VIBRATION) {
          digitalWrite(VIB1_PIN, HIGH);
          digitalWrite(VIB2_PIN, HIGH);
          delay(80);
          digitalWrite(VIB1_PIN, LOW);
          digitalWrite(VIB2_PIN, LOW);
          delay(120);
        }
      }
      break;
      
    case 3: // Triple beep (warning)
      for (int i = 0; i < 3; i++) {
        if (feedbackMode == FEEDBACK_MODE_BOTH || feedbackMode == FEEDBACK_MODE_BUZZER) {
          digitalWrite(BUZZER_PIN, HIGH);
          delay(60);
          digitalWrite(BUZZER_PIN, LOW);
          delay(140);
        }
        if (feedbackMode == FEEDBACK_MODE_BOTH || feedbackMode == FEEDBACK_MODE_VIBRATION) {
          digitalWrite(VIB1_PIN, HIGH);
          digitalWrite(VIB2_PIN, HIGH);
          delay(60);
          digitalWrite(VIB1_PIN, LOW);
          digitalWrite(VIB2_PIN, LOW);
          delay(140);
        }
      }
      break;
  }
}

void RFID_updateZoneStats() {
  uint32_t currentTime = millis();
  
  // Update time spent in current zone
  if (currentZone < 7) {
    zoneStats.totalTimeInZone[currentZone] += (currentTime - zoneStats.lastZoneEntry[currentZone]);
    zoneStats.lastZoneEntry[currentZone] = currentTime;
  }
}

void RFID_printZoneStats() {
  const char* zoneNames[] = {"Unknown", "Lobby", "Room 1", "Room 2", "Room 3", "Room 4", "Transition"};
  
  Serial.println("\n📊 ZONE STATISTICS:");
  Serial.println("====================");
  
  for (int i = 0; i < 7; i++) {
    if (zoneStats.zoneEntryCount[i] > 0) {
      uint32_t totalMinutes = zoneStats.totalTimeInZone[i] / 60000;
      Serial.printf("   %s: %d entries, %lu minutes\n", 
                    zoneNames[i], 
                    zoneStats.zoneEntryCount[i], 
                    totalMinutes);
    }
  }
  
  Serial.printf("   Current Zone: %s\n", zoneNames[currentZone]);
  Serial.printf("   Previous Zone: %s\n", zoneNames[previousZone]);
}

static void sendZoneDataToApp() {
  const char* zoneNames[] = {"Unknown", "Lobby", "Room 1", "Room 2", "Room 3", "Room 4", "Transition"};
  
  Serial.printf("📱 Zone Data: Current=%s, Previous=%s, Room=%s\n", 
                zoneNames[currentZone], 
                zoneNames[previousZone], 
                roomNames[currentRoom]);
}

void RFID_updateRoomLocationWithZones(const char* cardUID) {
  // Update room location normally
  updateRoomLocation(cardUID);
  
  // Update zone tracking
  RFID_updateZoneTracking();
  
  // Update zone statistics
  RFID_updateZoneStats();
  
  // Send enhanced zone data for mobile app
  sendZoneDataToApp();
}

static void updateFeedback() {
  if (!feedbackActive) return;
  
  uint32_t elapsed = millis() - feedbackStartTime;
  uint8_t currentStep = elapsed / FEEDBACK_DURATION;
  
  if (currentStep >= FEEDBACK_STEPS) {
    // Feedback sequence complete
    feedbackActive = false;
    feedbackStep = 0;
    
    // Turn off all feedback
    extern SensorData sensorData;
    uint8_t feedbackMode = sensorData.feedbackMode;
    
    if (feedbackMode == FEEDBACK_MODE_BOTH || feedbackMode == FEEDBACK_MODE_BUZZER) {
      digitalWrite(BUZZER_PIN, LOW);
    }
    if (feedbackMode == FEEDBACK_MODE_BOTH || feedbackMode == FEEDBACK_MODE_VIBRATION) {
      digitalWrite(VIB1_PIN, LOW);
      digitalWrite(VIB2_PIN, LOW);
    }
    
    return;
  }
  
  // Update feedback based on current step
  if (currentStep != feedbackStep) {
    feedbackStep = currentStep;
    
    extern SensorData sensorData;
    uint8_t feedbackMode = sensorData.feedbackMode;
    
    bool buzzerState = (feedbackStep == 0 || feedbackStep == 2); // First and third steps
    bool vibrationState = buzzerState; // Synchronized with buzzer
    
    if (feedbackMode == FEEDBACK_MODE_BOTH || feedbackMode == FEEDBACK_MODE_BUZZER) {
      digitalWrite(BUZZER_PIN, buzzerState ? HIGH : LOW);
    }
    if (feedbackMode == FEEDBACK_MODE_BOTH || feedbackMode == FEEDBACK_MODE_VIBRATION) {
      digitalWrite(VIB1_PIN, vibrationState ? HIGH : LOW);
      digitalWrite(VIB2_PIN, vibrationState ? HIGH : LOW);
    }
  }
} 