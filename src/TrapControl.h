#pragma once
#include <Arduino.h>
#include "config.h"
#include <Adafruit_PCF8574.h>

class TrapControl {
public:
    TrapControl(Adafruit_PCF8574& pcf);
    bool isFullyOpen();
    bool isFullyClosed();
    void open();
    void close();
    void up();
    void down();
    void stop();
    void resetErrors();

private:
    Adafruit_PCF8574& _pcf;
    int _errorCount;
    bool _isDisabled;
};