#pragma once
#include "SensorData.h"
#include "ValveControl.h"
#include "config.h"

class IrrigationControl {
public:
    IrrigationControl(ValveControl& valveControl);
    bool control(const SensorData& sensors);
private:
    ValveControl& valveControl;
};