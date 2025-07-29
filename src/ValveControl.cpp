#include "ValveControl.h"
#include "config.h"

ValveControl::ValveControl(Adafruit_PCF8574* pcfs[]) : _pcfs(pcfs) {}

void ValveControl::set(int valveIndex, bool open) {
    if (valveIndex >= 0 && valveIndex < NUM_WATER_VALVES) {
        const PinMapping& mapping = VALVE_PINS[valveIndex];
        Adafruit_PCF8574* pcf = _pcfs[mapping.pcfIndex];
        if (pcf) { // Safety check
            pcf->digitalWrite(mapping.pin, open ? LOW : HIGH);
        }
        valveStatus[valveIndex] = open ? "open" : "closed";
    }
}

bool ValveControl::isOpen(int valveIndex) {
    if (valveIndex >= 0 && valveIndex < NUM_WATER_VALVES) {
        const PinMapping& mapping = VALVE_PINS[valveIndex];
        Adafruit_PCF8574* pcf = _pcfs[mapping.pcfIndex];
        return pcf ? (pcf->digitalRead(mapping.pin) == LOW) : false;
    }
    return false;
}

bool ValveControl::isClosed(int valveIndex) {
    if (valveIndex >= 0 && valveIndex < NUM_WATER_VALVES) {
        const PinMapping& mapping = VALVE_PINS[valveIndex];
        Adafruit_PCF8574* pcf = _pcfs[mapping.pcfIndex];
        return pcf ? (pcf->digitalRead(mapping.pin) == HIGH) : false;
    }
    return false;
}