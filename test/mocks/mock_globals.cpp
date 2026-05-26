/**
 * @file mock_globals.cpp
 * @brief Definitions for mock/stub global objects used across native tests.
 *
 * Include this compilation unit (or its header) in each native test target.
 */
#include "Arduino.h"
#include "EEPROM.h"
#include "Adafruit_PCF8574.h"
#include "config.h"

// Arduino time stub — tests can write to this to simulate millis()
unsigned long _mock_millis = 0;

// Arduino Serial stub
_SerialStub Serial;

// Arduino Wire stub
_WireStub Wire;

// EEPROM mock
_EEPROMMock EEPROM;

// PCF8574 stubs
Adafruit_PCF8574 pcf1;
Adafruit_PCF8574 pcf2;

// Global status strings
String fanStatus[NUM_FANS]              = {"off", "off"};
String doorStatus[NUM_OF_DOORS]         = {"closed", "closed"};
String trapStatus                       = "stopped";
String pumpStatus[NUM_WATER_PUMPS]      = {"off", "off", "off"};
String valveStatus[NUM_WATER_VALVES]    = {"closed", "closed", "closed"};
String soilStatus[NUM_SOIL_SENSORS]     = {"invalid", "invalid", "invalid"};
String rainStatus                       = "dry";
String flowStatus[NUM_WATER_FLOW_METERS]= {"off", "off", "off"};
String mainFlowStatus                   = "off";
String i2cScanResults                   = "";

// Hardware status flags
bool rtcStatus  = false;
bool sdStatus   = false;
bool pcf1Status = false;
bool pcf2Status = false;
bool muxStatus  = false;

// Web credentials
char WEB_USERNAME[MAX_USERNAME_LENGTH + 1] = "admin";
char WEB_PASSWORD[MAX_PASSWORD_LENGTH + 1] = "password";

// Valve pin mapping
const PinMapping VALVE_PINS[NUM_WATER_VALVES] = {
    {0, 0}, {0, 1}, {0, 2}
};

// Irrigation zones
const IrrigationZone IRRIGATION_ZONES[NUM_IRRIGATION_ZONES] = {
    {0, 0, 0}, {1, 1, 1}, {2, 2, 2}
};

// Flow pulse counts
volatile unsigned long flowPulseCount[NUM_WATER_FLOW_METERS] = {0, 0, 0};

// MUX stub
void selectMuxChannel(int) {}

// Flow meter ISR stubs
void flowMeterISR0() {}
void flowMeterISR1() {}
void flowMeterISR2() {}