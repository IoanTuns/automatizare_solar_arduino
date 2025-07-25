#pragma once
#include <Arduino.h>
#include "config.h"
#include <Adafruit_PCF8574.h>

class PumpControl {
public:
    PumpControl(Adafruit_PCF8574& pcf);
    void set(int pumpIndex, bool on);
    bool isOn(int pumpIndex);

private:
    Adafruit_PCF8574& _pcf;
};