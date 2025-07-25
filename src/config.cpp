#include "config.h"

// ================== Global State Variables ==================
// These are the actual definitions for the 'extern' variables in config.h
String doorStatus[NUM_OF_DOORS] = { "closed", "closed" };
String trapStatus = "stopped";
String pumpStatus[NUM_WATER_PUMPS] = { "off", "off", "off" };
String fanStatus[NUM_FANS] = { "off", "off" };
String valveStatus[NUM_WATER_VALVES] = { "closed", "closed", "closed" };
String soilStatus[NUM_SOIL_SENSORS] = { "normal", "normal", "normal" };
String rainStatus = "dry";

// Updated trap control functions using PCF8574_2
void trapUp() {
    pcf2.digitalWrite(PCF2_TRAP_UP_PIN, LOW);   // Relay ON
    pcf2.digitalWrite(PCF2_TRAP_DOWN_PIN, HIGH); // Relay OFF
    trapStatus = "up";
}

void trapDown() {
    pcf2.digitalWrite(PCF2_TRAP_UP_PIN, HIGH);  // Relay OFF
    pcf2.digitalWrite(PCF2_TRAP_DOWN_PIN, LOW);  // Relay ON
    trapStatus = "down";
}

void stopTrap() {
    pcf2.digitalWrite(PCF2_TRAP_UP_PIN, HIGH);  // Relay OFF
    pcf2.digitalWrite(PCF2_TRAP_DOWN_PIN, HIGH); // Relay OFF
    trapStatus = "stopped";
}