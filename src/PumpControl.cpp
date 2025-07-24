#include "PumpControl.h"
#include <Adafruit_PCF8574.h>
extern Adafruit_PCF8574 pcf;

PumpControl::PumpControl() {}

void PumpControl::set(int pumpIndex, bool on) {
    if (pumpIndex >= 0 && pumpIndex < NUM_WATER_PUMPS) {
        if (pumpIndex == 0) pcf.digitalWrite(PCF_PUMP1_PIN, on ? LOW : HIGH);
        else if (pumpIndex == 1) pcf.digitalWrite(PCF_PUMP2_PIN, on ? LOW : HIGH);
        else if (pumpIndex == 2) pcf.digitalWrite(PCF_PUMP3_PIN, on ? LOW : HIGH);
    }
}

bool PumpControl::isOn(int pumpIndex) {
    if (pumpIndex >= 0 && pumpIndex < NUM_WATER_PUMPS) {
        return pcf.digitalRead(PCF_PUMP1_PIN) == LOW;
    }
    return false;
}