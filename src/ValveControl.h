#pragma once
#include <Arduino.h>
#include "config.h"

class ValveControl {
public:
    ValveControl();
    void set(int valveIndex, bool open);
    bool isOpen(int valveIndex);
};