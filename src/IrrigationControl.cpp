#include "IrrigationControl.h"
#include "config.h"

IrrigationControl::IrrigationControl(ValveControl& valveControl, PumpControl& pumpControl)
    : valveControl(valveControl), pumpControl(pumpControl) {}

/**
 * @brief Manages the irrigation logic for a single zone.
 * @param zone A reference to the IrrigationZone configuration struct.
 * @param sensors A reference to the latest sensor data.
 * @return true if the zone requires water, false otherwise.
 */
bool IrrigationControl::controlZone(const IrrigationZone& zone, const SensorData& sensors) {
    int sensorValue = sensors.soilMoisture[zone.soilSensorIndex];

    // If the sensor reading is invalid, do not perform any control actions for this zone.
    // The status will remain "invalid" as set by the main loop.
    if (sensorValue == -1) {
        pumpControl.set(zone.pumpIndex, false);   // Ensure pump is off for safety
        valveControl.set(zone.valveIndex, false); // Ensure valve is closed for safety
        return false; // Not irrigating
    }

    bool needsWater = false;

    // Implement hysteresis to prevent the pump from rapidly cycling.
    // If the pump for this zone is already on, we wait for the soil to get wetter before turning it off.
    // NOTE: Logic is inverted for sensors where a lower reading means drier soil.
    if (pumpControl.isOn(zone.pumpIndex)) {
        // Pump is ON. Keep it on until the soil is wet enough (reading is > WET threshold).
        needsWater = (sensorValue < SOIL_THRESHOLD_WET);
    } else {
        // Pump is OFF. Turn it on if the soil is dry enough (reading is < DRY threshold).
        needsWater = (sensorValue < SOIL_THRESHOLD_DRY);
    }

    // Control the valve and pump for this specific zone
    valveControl.set(zone.valveIndex, needsWater);
    pumpControl.set(zone.pumpIndex, needsWater);

    if (needsWater) {
        soilStatus[zone.soilSensorIndex] = "irrigating";
    }
    // If not irrigating, we don't change the status. The main loop has already set
    // the correct base status (e.g., "wet", "normal", "dry").
    return needsWater; // Returns true if this zone's pump is now on
}

bool IrrigationControl::control(const SensorData& sensors) {
    bool anyPumpIsOn = false;
    for (int i = 0; i < NUM_IRRIGATION_ZONES; i++) {
        anyPumpIsOn |= controlZone(IRRIGATION_ZONES[i], sensors);
    }
    return anyPumpIsOn;
}

void IrrigationControl::manualControl(int zoneIndex, bool on) {
    if (zoneIndex < 0 || zoneIndex >= NUM_IRRIGATION_ZONES) {
        return;
    }

    const IrrigationZone& zone = IRRIGATION_ZONES[zoneIndex];
    
    // Manually set the state of the pump and valve for this zone
    pumpControl.set(zone.pumpIndex, on);
    valveControl.set(zone.valveIndex, on);

    // Update the status to reflect manual override
    if (on) {
        soilStatus[zone.soilSensorIndex] = "manual on";
    } else {
        // When turning off manually, set a temporary status. The main loop's
        // sensor reading logic will update it to the correct state (wet, normal, dry)
        // on the next cycle. This provides immediate feedback to the user.
        soilStatus[zone.soilSensorIndex] = "manual off";
    }
}