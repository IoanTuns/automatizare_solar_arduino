#pragma once
#include <Arduino.h>
#include "config.h"

class PumpControl {
public:
    PumpControl();
    void set(int pumpIndex, bool on);
    bool isOn(int pumpIndex);
};