#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>
#include <DHT.h>

#define WEB_SERVER_PORT 80

// ================== System Capacity Constants ==================
const int NUM_SOIL_SENSORS      = 3;
const int NUM_OF_DOORS          = 2;
const int NUM_WATER_VALVES      = 3;
const int NUM_WATER_PUMPS       = 3;
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
const uint8_t DHTTYPE = DHT22;
const int TEMP_SENSOR_PINS[NUM_TEMP_SENSORS] = {2, 3}; // DHT22 sensors
const int TEMP_SENSOR_PIN_INT = TEMP_SENSOR_PINS[0];
const int TEMP_SENSOR_PIN_EXT = TEMP_SENSOR_PINS[1];

// --- Door Control ---
const int DOOR_PINS[] = {13, 14, 15, 16}; // 2 pins per door, unique pins
const int DOOR1_PIN_OPEN = DOOR_PINS[0];
const int DOOR1_PIN_CLOSE = DOOR_PINS[1];
const int DOOR2_PIN_OPEN = DOOR_PINS[2];
const int DOOR2_PIN_CLOSE = DOOR_PINS[3];

// --- Limit Switches ---
const int DOOR_LIMIT_OPEN_PINS[NUM_OF_DOORS]   = {17, 18}; // Unique pins
const int DOOR_LIMIT_CLOSED_PINS[NUM_OF_DOORS] = {19, 20}; // Unique pins
const int DOOR1_LIMIT_PIN_OPEN = DOOR_LIMIT_OPEN_PINS[0];
const int DOOR1_LIMIT_PIN_CLOSED = DOOR_LIMIT_CLOSED_PINS[0];
const int DOOR2_LIMIT_PIN_OPEN = DOOR_LIMIT_OPEN_PINS[1];
const int DOOR2_LIMIT_PIN_CLOSED = DOOR_LIMIT_CLOSED_PINS[1];

// --- Trap Control ---
const int TRAP_UP_PIN   = 21; // Unique pin
const int TRAP_DOWN_PIN = 22; // Unique pin
const int TRAP_PINS[NUM_TOP_TRAPS * 2] = {TRAP_UP_PIN, TRAP_DOWN_PIN};

// --- Multiplexer Channel Assignments ---
// Soil moisture sensors (channels 0–2)
const int SOIL_SENSOR1_MUX_CH = 0;
const int SOIL_SENSOR2_MUX_CH = 1;
const int SOIL_SENSOR3_MUX_CH = 2;

// Water flow meters (channels 3–5)
const int FLOW_METER1_MUX_CH = 3;
const int FLOW_METER2_MUX_CH = 4;
const int FLOW_METER3_MUX_CH = 5;

// Rain sensor (channel 6)
const int RAIN_SENSOR_MUX_CH = 6;

// --- Remove old analog pin assignments for these sensors ---
// const int SOIL_PINS[NUM_SOIL_SENSORS] = {A0, A1, A2};
// const int SOIL_SENSOR1_PIN = SOIL_PINS[0];
// const int SOIL_SENSOR2_PIN = SOIL_PINS[1];
// const int SOIL_SENSOR3_PIN = SOIL_PINS[2];
// const int FLOW_METER_PINS[NUM_WATER_FLOW_METERS] = {A3, A4, A5};
// const int FLOW_METER1_PIN = FLOW_METER_PINS[0];
// const int FLOW_METER2_PIN = FLOW_METER_PINS[1];
// const int FLOW_METER3_PIN = FLOW_METER_PINS[2];
// const int RAIN_SENSOR_PINS[NUM_RAIN_SENSORS] = {A5};

// --- CD74HC4067 16-Channel Analog/Digital Multiplexer ---
// S0–S3: Channel select pins (connect to any available digital pins)
const int MUX_S0_PIN = 23; // Example pin, change as needed
const int MUX_S1_PIN = 24;
const int MUX_S2_PIN = 25;
const int MUX_S3_PIN = 26;

// SIG: Multiplexer signal pin (connect to an analog pin on Arduino)
const int MUX_SIG_PIN = A0; // Example: use A0 if available

// ================== Threshold Values ==================
const int   SOIL_THRESHOLD = 600;
const float FAN_TEMP       = 28.0;
const float FAN_HUM        = 75.0;

// ================== PCF8574 I2C Expander ==================
const uint8_t PCF_ADDR = 0x20;
// --- PCF8574 I2C Expander Connection Pins ---
const int PCF_SDA_PIN = A4; // I2C Data
const int PCF_SCL_PIN = A5; // I2C Clock
// --- PCF8574 Pin Assignments ---
const int PCF_VALVE1_PIN = 0; // Valve 1 on PCF8574 pin 0
const int PCF_VALVE2_PIN = 1; // Valve 2 on PCF8574 pin 1
const int PCF_VALVE3_PIN = 2; // Valve 3 on PCF8574 pin 2
const int PCF_PUMP1_PIN  = 3; // Pump 1 on PCF8574 pin 3
const int PCF_PUMP2_PIN  = 4; // Pump 2 on PCF8574 pin 4
const int PCF_PUMP3_PIN  = 5; // Pump 3 on PCF8574 pin 5
const int PCF_FAN1_PIN   = 6; // Fan 1 on PCF8574 pin 6
const int PCF_FAN2_PIN   = 7; // Fan 2 on PCF8574 pin 7

// --- RTC DS3231 ---
// Uses I2C bus: SDA (A4), SCL (A5)

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