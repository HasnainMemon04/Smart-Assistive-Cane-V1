#pragma once
#ifndef ENVMONITOR_H
#define ENVMONITOR_H
#include "SensorData.h"
void EnvMonitor_init();
void EnvMonitor_update(SensorData* data);
#endif 