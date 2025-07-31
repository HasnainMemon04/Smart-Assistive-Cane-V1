#pragma once
#ifndef GPSMODULE_H
#define GPSMODULE_H
#include "SensorData.h"
void GPSModule_init();
void GPSModule_update(SensorData* data);
#endif 