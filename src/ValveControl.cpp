#include "ValveControl.h"

ValveControl::ValveControl() {}

void ValveControl::set(int valveIndex, bool open) {
    if (valveIndex >= 0 && valveIndex < NUM_WATER_VALVES) {
        digitalWrite(VALVE_PINS[valveIndex], open ? LOW : HIGH);
    }
}

bool ValveControl::isOpen(int valveIndex) {
    if (valveIndex >= 0 && valveIndex < NUM_WATER_VALVES) {
        return digitalRead(VALVE_PINS[valveIndex]) == LOW;
    }
    return false;
}