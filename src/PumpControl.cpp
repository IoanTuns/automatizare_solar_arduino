#include "PumpControl.h"
#include "config.h"

PumpControl::PumpControl(Adafruit_PCF8574& pcf) : _pcf(pcf) {}

void PumpControl::set(int pumpIndex, bool on) {
    if (pumpIndex >= 0 && pumpIndex < NUM_WATER_PUMPS) {
        _pcf.digitalWrite(PCF1_PUMP_PINS[pumpIndex], on ? LOW : HIGH);
        pumpStatus[pumpIndex] = on ? "on" : "off";
    }
}

bool PumpControl::isOn(int pumpIndex) {
    if (pumpIndex >= 0 && pumpIndex < NUM_WATER_PUMPS) {
        return _pcf.digitalRead(PCF1_PUMP_PINS[pumpIndex]) == LOW;
    }
    return false;
}