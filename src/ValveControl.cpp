#include "ValveControl.h"
#include <Adafruit_PCF8574.h>
extern Adafruit_PCF8574 pcf;

ValveControl::ValveControl() {}

void ValveControl::set(int valveIndex, bool open) {
    if (valveIndex >= 0 && valveIndex < NUM_WATER_VALVES) {
        if (valveIndex == 0) pcf.digitalWrite(PCF_VALVE1_PIN, open ? LOW : HIGH);
        else if (valveIndex == 1) pcf.digitalWrite(PCF_VALVE2_PIN, open ? LOW : HIGH);
        else if (valveIndex == 2) pcf.digitalWrite(PCF_VALVE3_PIN, open ? LOW : HIGH);
    }
}

bool ValveControl::isOpen(int valveIndex) {
    if (valveIndex >= 0 && valveIndex < NUM_WATER_VALVES) {
        return pcf.digitalRead(PCF_VALVE1_PIN) == LOW;
    }
    return false;
}

bool ValveControl::isClosed(int valveIndex) {
    if (valveIndex >= 0 && valveIndex < NUM_WATER_VALVES) {
        if (valveIndex == 0) return pcf.digitalRead(PCF_VALVE1_PIN) == HIGH;
        else if (valveIndex == 1) return pcf.digitalRead(PCF_VALVE2_PIN) == HIGH;
        else if (valveIndex == 2) return pcf.digitalRead(PCF_VALVE3_PIN) == HIGH;
    }
    return false;
}