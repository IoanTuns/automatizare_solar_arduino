#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

#define WEB_SERVER_PORT 80

// ================== System Capacity Constants ==================
const int NUM_SOIL_SENSORS      = 3;
const int NUM_OF_DOORS          = 2;
const int NUM_WATER_VALVES      = 3;
const int NUM_WATER_PUMPS       = 3;
const int NUM_WATER_FLOW_CONTROLS = 3;
const int NUM_WATER_FLOW_METERS = 3;
const int NUM_FANS              = 2;
const int NUM_TEMP_SENSORS      = 2;
const int NUM_RAIN_SENSORS      = 1;
const int NUM_TOP_TRAPS         = 1;

// ================== Status Variables (extern) ==================
extern String doorStatus[NUM_OF_DOORS];
extern String trapStatus;
extern String pumpStatus[NUM_WATER_PUMPS];
extern String fanStatus[NUM_FANS];
extern String valveStatus[NUM_WATER_VALVES];
extern String soilStatus[NUM_SOIL_SENSORS];
extern String rainStatus;
extern String climateStatus;

// ================== Pin Assignments ==================

// --- DHT Sensor ---
#define DHTPIN_INT   2
#define DHTPIN_EXT   3
#define DHTTYPE      DHT22

// --- Soil Moisture Sensors (Analog) ---
const int SOIL_PINS[NUM_SOIL_SENSORS] = { A0, A1, A2 };

// --- Water Pumps ---
const int PUMP_PINS[NUM_WATER_PUMPS] = { 2, 3, 4 };
const int PUMP1_PIN = PUMP_PINS[0];
const int PUMP2_PIN = PUMP_PINS[1];
const int PUMP3_PIN = PUMP_PINS[2];

// --- Water Valves ---
const int VALVE_PINS[NUM_WATER_VALVES] = { 4, 5, 6 };

// --- Main Pump and Fans ---
const int MAIN_PUMP_PIN = 10;
const int FAN_PINS[NUM_FANS] = { 11, 12 };
const int FAN_PIN = FAN_PINS[0]; // Primary fan

// --- Door Control ---
const int DOOR_PINS[NUM_OF_DOORS * 2] = { 7, 8, 9, 13 }; // 2 pins per door
const int DOOR_LIMIT_OPEN_PINS[NUM_OF_DOORS]   = { 22, 24 };
const int DOOR_LIMIT_CLOSED_PINS[NUM_OF_DOORS] = { 23, 25 };

// --- Trap Control ---
const int TRAP_UP_PIN   = 12;
const int TRAP_DOWN_PIN = 13;
const int TRAP_PINS[NUM_TOP_TRAPS * 2] = { TRAP_UP_PIN, TRAP_DOWN_PIN };

// --- Temperature Sensors (DHT22) ---
const int TEMP_SENSOR_PINS[NUM_TEMP_SENSORS] = { DHTPIN_INT, DHTPIN_EXT };

// --- Rain Sensor ---
const int RAIN_SENSOR_PINS[NUM_RAIN_SENSORS] = { A3 };

// ================== Threshold Values ==================
const int   SOIL_THRESHOLD = 600;
const float FAN_TEMP       = 28.0;
const float FAN_HUM        = 75.0;

// ================== PCF8574 I2C Expander ==================
#define PCF_ADDR 0x20
const int PCF_VALVE_START = 0; // PCF pins 0-2 for additional valves
const int PCF_PUMP_START  = 3; // PCF pins 3-5 for additional pumps
const int PCF_FAN_START   = 6; // PCF pins 6-7 for additional fans

// ================== Function Prototypes ==================
void openDoor(int doorIndex);
void closeDoor(int doorIndex);
void stopDoor(int doorIndex);
void doorUp(int doorIndex);
void doorDown(int doorIndex);

void openTrap();
void closeTrap();
void trapUp();
void trapDown();
void stopTrap();

void controlFan(int fanIndex, bool state);
void controlPump(int pumpIndex, bool state);
void controlValve(int valveIndex, bool state);

#endif // CONFIG_H