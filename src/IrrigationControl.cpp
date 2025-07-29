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
    bool needsWater = false;

    // Implement hysteresis to prevent the pump from rapidly cycling.
    // If the pump for this zone is already on, we wait for the soil to get wetter before turning it off.
    if (pumpControl.isOn(zone.pumpIndex)) {
        needsWater = (sensorValue > SOIL_THRESHOLD_WET);
    } else {
        // If the pump is off, we only turn it on if the soil is dry enough.
        needsWater = (sensorValue > SOIL_THRESHOLD_DRY);
    }

    // Control the valve and pump for this specific zone
    valveControl.set(zone.valveIndex, needsWater);
    pumpControl.set(zone.pumpIndex, needsWater);

    if (needsWater) {
        soilStatus[zone.soilSensorIndex] = "irrigating";
    } else {
        soilStatus[zone.soilSensorIndex] = "normal";
    }

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
        // When turning off manually, revert to a neutral state.
        // The automatic control loop will then take over and set it to "normal" or "irrigating" on the next cycle.
        soilStatus[zone.soilSensorIndex] = "normal";
    }
}