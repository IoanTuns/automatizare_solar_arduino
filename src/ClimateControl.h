#pragma once
#include "SensorData.h"
#include "config.h"
#include <Adafruit_PCF8574.h>

class ClimateControl {
public:
    ClimateControl(Adafruit_PCF8574& pcf);
    void control(const SensorData& sensors);
    void controlFan(int fanIndex, bool state);
private:
    Adafruit_PCF8574& _pcf;
};