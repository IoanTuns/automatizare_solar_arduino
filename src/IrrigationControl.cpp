#include "IrrigationControl.h"

IrrigationControl::IrrigationControl(ValveControl& valveControl)
    : valveControl(valveControl) {}

bool IrrigationControl::control(const SensorData& sensors) {
    bool anyIrrigating = false;
    for (int i = 0; i < NUM_SOIL_SENSORS && i < NUM_WATER_VALVES; i++) {
        bool needsWater = sensors.soilMoisture[i] > SOIL_THRESHOLD;
        valveControl.set(i, needsWater);
        if (needsWater) {
            anyIrrigating = true;
            Serial.print("Zone ");
            Serial.print(i + 1);
            Serial.print(" irrigation ON (moisture: ");
            Serial.print(sensors.soilMoisture[i]);
            Serial.println(")");
        }
    }
    digitalWrite(PCF_PUMP3_PIN, anyIrrigating ? HIGH : LOW);
    return anyIrrigating;
}