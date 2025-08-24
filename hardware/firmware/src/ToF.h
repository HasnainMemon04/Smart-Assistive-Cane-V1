#pragma once
#ifndef TOF_H
#define TOF_H
#include "SensorData.h"

void ToF_init();
void ToF_update(SensorData* data);

// Radar Mode Functions
void ToF_switchToRadarMode();
void ToF_switchToSimpleMode();
bool ToF_isRadarMode();

// Manual Reset Function
void ToF_manualReset();

// Diagnostic Function
void ToF_diagnostics();

// Getter functions for BLE access
OperationMode ToF_getCurrentMode();
uint16_t* ToF_getScanData();

#endif // TOF_H