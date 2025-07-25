#include "ValveControl.h"
#include "config.h"

ValveControl::ValveControl(Adafruit_PCF8574& pcf) : _pcf(pcf) {}

void ValveControl::set(int valveIndex, bool open) {
    if (valveIndex >= 0 && valveIndex < NUM_WATER_VALVES) {
        _pcf.digitalWrite(PCF1_VALVE_PINS[valveIndex], open ? LOW : HIGH);
        valveStatus[valveIndex] = open ? "open" : "closed";
    }
}

bool ValveControl::isOpen(int valveIndex) {
    if (valveIndex >= 0 && valveIndex < NUM_WATER_VALVES) {
        return _pcf.digitalRead(PCF1_VALVE_PINS[valveIndex]) == LOW;
    }
    return false;
}

bool ValveControl::isClosed(int valveIndex) {
    if (valveIndex >= 0 && valveIndex < NUM_WATER_VALVES) {
        return _pcf.digitalRead(PCF1_VALVE_PINS[valveIndex]) == HIGH;
    }
    return false;
}