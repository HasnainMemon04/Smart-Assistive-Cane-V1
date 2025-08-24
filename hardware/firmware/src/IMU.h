#pragma once
#ifndef IMU_H
#define IMU_H
#include "SensorData.h"

#include <Arduino.h>
void IMU_init();
void IMU_update(SensorData* data);
void IMU_setTime(uint8_t hour, uint8_t minute, uint8_t day, uint8_t month, uint16_t year);
String IMU_getTimeSource();

// Getter functions for BLE access
FallState IMU_getFallState();
float IMU_getMotionEnergy();
bool IMU_getSlopeWarningActive();
#endif