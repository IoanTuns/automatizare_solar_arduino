#include "ClimateControl.h"

ClimateControl::ClimateControl() {}

void ClimateControl::control(const SensorData& sensors) {
    if (!sensors.valid) return;
    bool fanNeeded = (sensors.tempInt > FAN_TEMP) || (sensors.humInt > FAN_HUM);
    controlFan(0, fanNeeded);
    bool secondaryFanNeeded = (sensors.tempInt > FAN_TEMP + 5.0) || (sensors.humInt > FAN_HUM + 10.0);
    if (NUM_FANS > 1) {
        controlFan(1, secondaryFanNeeded);
    }
    if (fanNeeded) {
        Serial.print("Climate control: Fan ON (T:");
        Serial.print(sensors.tempInt);
        Serial.print("°C, H:");
        Serial.print(sensors.humInt);
        Serial.println("%)");
    }
}