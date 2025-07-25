#include "IrrigationControl.h"
#include "config.h"

IrrigationControl::IrrigationControl(ValveControl& valveControl, PumpControl& pumpControl)
    : valveControl(valveControl), pumpControl(pumpControl) {}

bool IrrigationControl::control(const SensorData& sensors) {
    bool anyZoneNeedsWater = false;
    for (int i = 0; i < NUM_SOIL_SENSORS && i < NUM_WATER_VALVES; i++) {
        bool needsWater = sensors.soilMoisture[i] > SOIL_THRESHOLD_DRY;
        // Always update valve status based on immediate need
        valveControl.set(i, needsWater);

        if (needsWater) {
            anyZoneNeedsWater = true;
            soilStatus[i] = "irrigating";
            Serial.print("Zone ");
            Serial.print(i + 1);
            Serial.print(" irrigation ON (moisture: ");
            Serial.print(sensors.soilMoisture[i]);
            Serial.println(")");
        } else {
            soilStatus[i] = "normal";
        }
    }

    // --- Latching Pump Control Logic ---
    // If any zone is dry, ensure the main pump is ON.
    if (anyZoneNeedsWater) {
        pumpControl.set(0, true); // Use pump 0 for general irrigation
    } 
    // If no zone is currently dry, check for the automatic shut-off condition.
    else {
        // The pump only turns off if the master sensor (Soil Sensor 1) is sufficiently wet.
        if (sensors.soilMoisture[0] < SOIL_THRESHOLD_WET) {
            pumpControl.set(0, false);
        }
        // Otherwise, do nothing, leaving the pump ON (latching it).
    }
    
    return pumpControl.isOn(0);
}