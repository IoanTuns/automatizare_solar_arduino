#include "config.h"

// ================== Global State Variables ==================
// These are the actual definitions for the 'extern' variables in config.h
String doorStatus[NUM_OF_DOORS] = { "closed", "closed" };
String trapStatus = "stopped";
String pumpStatus[NUM_WATER_PUMPS] = { "off", "off", "off" };
String fanStatus[NUM_FANS] = { "off", "off" };
String valveStatus[NUM_WATER_VALVES] = { "closed", "closed", "closed"};
String soilStatus[NUM_SOIL_SENSORS] = { "invalid", "invalid", "invalid" };
String rainStatus = "dry";
String i2cScanResults = "Scan not performed yet.";

// Hardware status flags
bool rtcStatus = false;
bool sdStatus = false;
bool pcf1Status = false;
bool pcf2Status = false;
bool muxStatus = false;

// Definition for the irrigation zones. This allows flexible mapping of components.
const IrrigationZone IRRIGATION_ZONES[NUM_IRRIGATION_ZONES] = {
    {0, 0, 0}, // Zone 1: Pump 1 (index 0), Valve 1 (index 0), Soil Sensor 1 (index 0)
    {1, 1, 1}, // Zone 2: Pump 2 (index 1), Valve 2 (index 1), Soil Sensor 2 (index 1)
    {2, 2, 2}  // Zone 3: Pump 3 (index 2), Valve 3 (index 2), Soil Sensor 3 (index 2)
};

// Definition for the new valve pin mapping array
const PinMapping VALVE_PINS[NUM_WATER_VALVES] = {
    {0, PCF1_VALVE1_PIN}, // Valve 1 on pcf1 (index 0), pin 0
    {0, PCF1_VALVE2_PIN}, // Valve 2 on pcf1 (index 0), pin 1
    {0, PCF1_VALVE3_PIN}  // Valve 3 on pcf1 (index 0), pin 2
};