#pragma once
#include "SensorData.h"
#include "config.h"

class ClimateControl {
public:
    ClimateControl();
    void control(const SensorData& sensors);
};