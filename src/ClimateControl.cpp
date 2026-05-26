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
    
    // Primary Fan Logic with Hysteresis
    bool fan1CurrentlyOn = (fanStatus[0] == "on");
    bool fan1Needed = fan1CurrentlyOn; 
    
    if (!fan1CurrentlyOn) {
        fan1Needed = (sensors.tempInt > FAN_TEMP_PRIMARY) || (sensors.humInt > FAN_HUM_PRIMARY);
    } else {
        fan1Needed = (sensors.tempInt > (FAN_TEMP_PRIMARY - CLIMATE_HYSTERESIS)) && 
                     (sensors.humInt > (FAN_HUM_PRIMARY - CLIMATE_HYSTERESIS));
    }
    controlFan(0, fan1Needed);

    // Secondary Fan Logic
    if (NUM_FANS > 1) {
        bool fan2CurrentlyOn = (fanStatus[1] == "on");
        bool secondaryFanNeeded = fan2CurrentlyOn ? 
            (sensors.tempInt > (FAN_TEMP_SECONDARY - CLIMATE_HYSTERESIS)) : 
            (sensors.tempInt > FAN_TEMP_SECONDARY);
            
        controlFan(1, secondaryFanNeeded); // Control second fan
    }

    if (fan1Needed || (NUM_FANS > 1 && (fanStatus[1] == "on"))) {
        Serial.print("Climate control: Fan ON (T:");
        Serial.print(sensors.tempInt);
        Serial.print("°C, H:");
        Serial.print(sensors.humInt);
        Serial.println("%)");
    }
}