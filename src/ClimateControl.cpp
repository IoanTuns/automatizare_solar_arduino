#include "ClimateControl.h"
#include "config.h"

ClimateControl::ClimateControl(Adafruit_PCF8574& pcf) : _pcf(pcf) {}

void ClimateControl::controlFan(int fanIndex, bool state) {
    if (fanIndex >= 0 && fanIndex < NUM_FANS) {
        _pcf.digitalWrite(PCF1_FAN_PINS[fanIndex], state ? LOW : HIGH);
        fanStatus[fanIndex] = state ? "on" : "off";
    }
}

void ClimateControl::control(const SensorData& sensors) {
    if (!sensors.valid) return;
    
    bool fanNeeded = (sensors.tempInt > FAN_TEMP_PRIMARY) || (sensors.humInt > FAN_HUM_PRIMARY);
    controlFan(0, fanNeeded); // Control first fan

    bool secondaryFanNeeded = (sensors.tempInt > FAN_TEMP_SECONDARY) || (sensors.humInt > FAN_HUM_SECONDARY);
    if (NUM_FANS > 1) {
        controlFan(1, secondaryFanNeeded); // Control second fan
    }
    
    if (fanNeeded) {
        Serial.print("Climate control: Fan ON (T:");
        Serial.print(sensors.tempInt);
        Serial.print("°C, H:");
        Serial.print(sensors.humInt);
        Serial.println("%)");
    }
}