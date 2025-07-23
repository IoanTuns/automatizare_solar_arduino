#include "PumpControl.h"

PumpControl::PumpControl() {}

void PumpControl::set(int pumpIndex, bool on) {
    if (pumpIndex >= 0 && pumpIndex < NUM_WATER_PUMPS) {
        digitalWrite(PUMP_PINS[pumpIndex], on ? HIGH : LOW);
    }
}

bool PumpControl::isOn(int pumpIndex) {
    if (pumpIndex >= 0 && pumpIndex < NUM_WATER_PUMPS) {
        return digitalRead(PUMP_PINS[pumpIndex]) == HIGH;
    }
    return false;
}