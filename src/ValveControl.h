#pragma once
#include <Arduino.h>
#include "config.h"
#include <Adafruit_PCF8574.h>

class ValveControl {
public:
    ValveControl(Adafruit_PCF8574* pcfs[]);
    void set(int valveIndex, bool open);
    bool isOpen(int valveIndex);
    bool isClosed(int valveIndex);

private:
    Adafruit_PCF8574** _pcfs;
};