#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>
#include <DHT.h>

const int WEB_SERVER_PORT  = 80;

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

// ================== DHT Sensor Pins ==================
const uint8_t DHTTYPE = DHT22;
const int TEMP_SENSOR_PINS[NUM_TEMP_SENSORS] = {2, 3}; // DHT22 sensors
const int TEMP_SENSOR_PIN_INT = TEMP_SENSOR_PINS[0];
const int TEMP_SENSOR_PIN_EXT = TEMP_SENSOR_PINS[1];

// ================== Door Motor Pins ==================
const int DOOR_PINS[NUM_OF_DOORS * 2] = {13, 14, 15, 16}; // 2 pins per door

// ================== Door Limit Switch Pins ==================
const int DOOR_LIMIT_OPEN_PINS[NUM_OF_DOORS]   = {17, 18};
const int DOOR_LIMIT_CLOSED_PINS[NUM_OF_DOORS] = {19, 20};

// ================== Trap Motor Pins ==================
const int TRAP_UP_PIN   = 21;
const int TRAP_DOWN_PIN = 22;
const int TRAP_PINS[NUM_TOP_TRAPS * 2] = {TRAP_UP_PIN, TRAP_DOWN_PIN};

// ================== Multiplexer Pins ==================
const int MUX_S0_PIN = 23;
const int MUX_S1_PIN = 24;
const int MUX_S2_PIN = 25;
const int MUX_S3_PIN = 26;
const int MUX_SIG_PIN = A0;

// ================== PCF8574 I2C Expander ==================
const uint8_t PCF_ADDR = 0x20;
const int PCF_SDA_PIN = A4; // I2C Data
const int PCF_SCL_PIN = A5; // I2C Clock

// --- PCF8574 Pin Assignments ---
const int PCF_VALVE1_PIN = 0;
const int PCF_VALVE2_PIN = 1;
const int PCF_VALVE3_PIN = 2;
const int PCF_PUMP1_PIN  = 3;
const int PCF_PUMP2_PIN  = 4;
const int PCF_PUMP3_PIN  = 5;
const int PCF_FAN1_PIN   = 6;
const int PCF_FAN2_PIN   = 7;

// --- PCF8574 Pin Arrays ---
const int PCF_VALVE_PINS[NUM_WATER_VALVES] = {PCF_VALVE1_PIN, PCF_VALVE2_PIN, PCF_VALVE3_PIN};
const int PCF_PUMP_PINS[NUM_WATER_PUMPS]   = {PCF_PUMP1_PIN, PCF_PUMP2_PIN, PCF_PUMP3_PIN};
const int PCF_FAN_PINS[NUM_FANS]           = {PCF_FAN1_PIN, PCF_FAN2_PIN};

// ================== Other Sensor Channels (Multiplexer) ==================
const int SOIL_SENSOR1_MUX_CH = 0;
const int SOIL_SENSOR2_MUX_CH = 1;
const int SOIL_SENSOR3_MUX_CH = 2;
const int FLOW_METER1_MUX_CH  = 3;
const int FLOW_METER2_MUX_CH  = 4;
const int FLOW_METER3_MUX_CH  = 5;
const int RAIN_SENSOR_MUX_CH  = 6;

// ================== Threshold Values ==================
const int   SOIL_THRESHOLD = 600;
const float FAN_TEMP       = 28.0;
const float FAN_HUM        = 75.0;

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