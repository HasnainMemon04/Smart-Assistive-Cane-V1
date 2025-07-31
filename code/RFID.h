#pragma once
#ifndef RFID_H
#define RFID_H

#include "SensorData.h"

void RFID_init();
void RFID_update(SensorData* data);

// Indoor zoning system functions
void RFID_setupZoning();
void RFID_addRoomCard(uint8_t roomNumber, const char* cardUID);
const char* RFID_getRoomName(uint8_t roomNumber);
void RFID_clearRoomCards();
void RFID_printRoomCards();

// Auto card registration system
void RFID_startAutoRegistration();
void RFID_stopAutoRegistration();
bool RFID_isAutoRegistrationActive();
void RFID_processAutoRegistration(const char* cardUID);

// Single command registration system
void RFID_startSingleLocationRegistration();

// Sensor control during registration
bool RFID_areSensorsDisabled();

// Enhanced features from RFID text file
void RFID_saveRoomData();
void RFID_loadRoomData();
void RFID_addRoomCardWithSave(uint8_t roomNumber, const char* cardUID, const char* roomName);
void RFID_clearRoomCardsWithSave();

// Zone tracking system
void RFID_updateZoneTracking();
void RFID_printZoneStats();

// Enhanced room tracking
void RFID_updateRoomLocationWithZones(const char* cardUID);

#endif // RFID_H 