#ifndef CONFIG_H
#define CONFIG_H
// config.h

// Web server port
#define WEB_SERVER_PORT 80

// I2C PCF8574 configuration
#define PCF_ADDR 0x20

// DHT sensor configuration
#define DHTPIN_INT   2
#define DHTPIN_EXT   3
#define DHTTYPE      DHT22

// System capacity constants
const int NUM_SOIL_SENSORS = 3;
const int NUM_OF_DOORS = 2;
const int NUM_WATER_VALVES = 3;
const int NUM_WATER_PUMPS = 3;
const int NUM_WATER_FLOW_CONTROLS = 3;
const int NUM_FANS = 2;
const int NUM_TEMP_SENSORS = 2;
const int NUM_RAIN_SENSORS = 1;
const int NUM_TOP_TRAPS = 1;

// Soil moisture sensor pins (analog)
const int SOIL_PINS[NUM_SOIL_SENSORS] = { A0, A1, A2 };

// Water pump pins
const int PUMP_PINS[NUM_WATER_PUMPS] = { 2, 3, 4 };
const int PUMP1_PIN = PUMP_PINS[0];  // 2
const int PUMP2_PIN = PUMP_PINS[1];  // 3
const int PUMP3_PIN = PUMP_PINS[2];  // 4

// Water valve pins
const int VALVE_PINS[NUM_WATER_VALVES] = { 4, 5, 6 };

// Main pump and fan pins
const int MAIN_PUMP_PIN = 10;
const int FAN_PINS[NUM_FANS] = { 11, 12 };
const int FAN_PIN = FAN_PINS[0];  // Primary fan

// Door control pins (assuming 2 doors)
const int DOOR_PINS[NUM_OF_DOORS * 2] = { 7, 8, 9, 13 };  // Each door needs 2 pins for motor control

// Trap control pins
const int TRAP_IN1 = 12;
const int TRAP_IN2 = 13;
const int TRAP_PINS[NUM_TOP_TRAPS * 2] = { TRAP_IN1, TRAP_IN2 };

// Temperature sensor pins (DHT22)
const int TEMP_SENSOR_PINS[NUM_TEMP_SENSORS] = { DHTPIN_INT, DHTPIN_EXT };

// Rain sensor pins
const int RAIN_SENSOR_PINS[NUM_RAIN_SENSORS] = { A3 };

// Threshold values
const int SOIL_THRESHOLD = 600;
const float FAN_TEMP = 28.0;
const float FAN_HUM = 75.0;

// PCF8574 pin assignments (I2C expander)
const int PCF_VALVE_START = 0;  // PCF pins 0-2 for additional valves
const int PCF_PUMP_START = 3;   // PCF pins 3-5 for additional pumps
const int PCF_FAN_START = 6;    // PCF pins 6-7 for additional fans

// Function prototypes
void openTrap();
void closeTrap();
void openDoor();
void closeDoor();
void openDoor(int doorIndex);
void closeDoor(int doorIndex);
void controlFan(int fanIndex, bool state);
void controlPump(int pumpIndex, bool state);
void controlValve(int valveIndex, bool state);

#endif // CONFIG_H