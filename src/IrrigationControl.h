#pragma once
#include "SensorData.h"
#include "ValveControl.h"
#include "PumpControl.h"
#include "config.h"

class IrrigationControl {
public:
    IrrigationControl(ValveControl& valveControl, PumpControl& pumpControl);
    bool control(const SensorData& sensors);
    void manualControl(int zoneIndex, bool on);
private:
    ValveControl& valveControl;
    PumpControl& pumpControl;
    bool controlZone(const IrrigationZone& zone, const SensorData& sensors);
};