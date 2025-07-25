#pragma once
#include <Arduino.h>
#include "config.h"
#include <Adafruit_PCF8574.h>

class DoorControl {
public:
    // Constructor now accepts a reference to the PCF8574 it should control.
    DoorControl(Adafruit_PCF8574& pcf);
    bool isFullyOpen(int doorIndex);
    bool isFullyClosed(int doorIndex);
    void open(int doorIndex);
    void close(int doorIndex);
    void up(int doorIndex);
    void down(int doorIndex);
    void stop(int doorIndex);

private:
    Adafruit_PCF8574& _pcf; // Store a reference to the provided PCF8574
};