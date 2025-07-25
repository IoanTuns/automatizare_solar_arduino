#pragma once
#include "SensorData.h"
#include "ValveControl.h"
#include "PumpControl.h"
#include "config.h"

class IrrigationControl {
public:
    IrrigationControl(ValveControl& valveControl, PumpControl& pumpControl);
    bool control(const SensorData& sensors);
private:
    ValveControl& valveControl;
    PumpControl& pumpControl;
};