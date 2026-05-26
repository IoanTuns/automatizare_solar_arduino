#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>
#include <Adafruit_PCF8574.h>
#include <DHT.h>

// Firmware and Build Info (used in System Info modal)
#define FIRMWARE_VERSION "1.1.0"
#define BUILD_ENV_NAME "uno_r4_wifi"

// ================== Serial & Web Server ==================
const long SERIAL_BAUD_RATE = 115200;
const uint16_t WEB_SERVER_PORT  = 80;

// ================== Timing Configuration (in milliseconds) ==================
const unsigned long SENSOR_READ_INTERVAL_MS = 2000;
const unsigned long DISPLAY_UPDATE_INTERVAL_MS = 10000;
const unsigned long WIFI_CONNECT_TIMEOUT_MS = 30000;
const uint8_t MAIN_LOOP_DELAY_MS = 100;
const unsigned long MUX_SETTLE_DELAY_MS = 15;

// ================== SD Card Pin ==================
// The default SPI CS pin is 10. This must not conflict with other hardware.
const int SD_CS_PIN = 10;

// ================== Pin Mapping Struct ==================
struct PinMapping {
    uint8_t pcfIndex; // 0 for pcf1, 1 for pcf2
    uint8_t pin;
};


// ================== System Capacity Constants (No Change) ==================
const int NUM_SOIL_SENSORS        = 3;
const int NUM_OF_DOORS            = 2; // Restore to 2 doors as requested.
const int NUM_WATER_VALVES        = 3; // REDUCED: from 5 to 3 to free up 2 pins on PCF2 for trap limit switches.
const int NUM_WATER_PUMPS         = 3; 
const int NUM_WATER_FLOW_METERS   = 3; // NOW DEDICATED PINS, NOT MULTIPLEXED
const int NUM_FANS                = 2;
const uint8_t NUM_TEMP_SENSORS        = 2;
const uint8_t NUM_RAIN_SENSORS        = 1;
const uint8_t NUM_TOP_TRAPS           = 1; // Assume 2 pins (relays) for trap motor UP/DOWN


// ================== DHT Sensor Pins (Direct Arduino Digital Inputs) ==================
const uint8_t DHTTYPE = 22; // 22 is the value for DHT22/AM2302
const uint8_t TEMP_SENSOR_PINS[NUM_TEMP_SENSORS] = {2, 3}; // Digital pins D2, D3
const uint8_t TEMP_SENSOR_PIN_INT = TEMP_SENSOR_PINS[0]; // Indoor DHT22
const uint8_t TEMP_SENSOR_PIN_EXT = TEMP_SENSOR_PINS[1]; // Outdoor DHT22


// ================== Door Motor Pins (Controlled via PCF8574_2) ==================
// Each linear actuator (door) typically needs 2 relay channels for extend/retract.
// These will be controlled by the SECOND PCF8574 (PCF8574_2).
const int PCF2_DOOR1_EXTEND_PIN  = 0; // PCF8574_2 Pin 0
const int PCF2_DOOR1_RETRACT_PIN = 1; // PCF8574_2 Pin 1
const int PCF2_DOOR2_EXTEND_PIN  = 2; // PCF8574_2 Pin 2
const int PCF2_DOOR2_RETRACT_PIN = 3; // PCF8574_2 Pin 3
const int PCF2_DOOR_PINS[NUM_OF_DOORS * 2] = {
  PCF2_DOOR1_EXTEND_PIN, PCF2_DOOR1_RETRACT_PIN,
  PCF2_DOOR2_EXTEND_PIN, PCF2_DOOR2_RETRACT_PIN
};
extern String doorStatus[NUM_OF_DOORS];
// Door operation timeouts
const unsigned long DOOR_MOVE_TIMEOUT_MS = 5000;
const unsigned long DOOR_CLOSE_DELAY_MS = 2000;



// ================== Door Limit Switch Pins (Direct Arduino Digital Inputs) ==================
// Using available general-purpose digital pins on the Arduino.
// This configuration assumes 2 limit switches per door (one for open, one for closed),
// totaling 4 limit switches for the 2 doors.
const uint8_t DOOR_LIMIT_OPEN_PINS[NUM_OF_DOORS]   = {4, 5};  // Digital Pin D4 for Door 1 Open, D5 for Door 2 Open
const uint8_t DOOR_LIMIT_CLOSED_PINS[NUM_OF_DOORS] = {6, 7};  // Digital Pin D6 for Door 1 Closed, D7 for Door 2 Closed


// ================== Trap Motor Pins (Controlled via PCF8574_2) ==================
// Assuming 2 pins for UP/DOWN control of the trap motor.
// These will be controlled by the SECOND PCF8574 (PCF8574_2).
const int PCF2_TRAP_UP_PIN   = 4; // PCF8574_2 Pin 4
const int PCF2_TRAP_DOWN_PIN = 5; // PCF8574_2 Pin 5
const int PCF2_TRAP_PINS[NUM_TOP_TRAPS * 2] = {PCF2_TRAP_UP_PIN, PCF2_TRAP_DOWN_PIN};
extern String trapStatus;
const unsigned long TRAP_MOVE_TIMEOUT_MS = 5000; // Timeout for trap movement


// ================== Trap Limit Switch Pins (Controlled via PCF8574_2) ==================
// To resolve pin conflicts on the Arduino, the trap limit switches are moved
// to the second I/O expander (pcf2), which has spare pins.
// You must physically connect the trap limit switches to pins P6 and P7 on the
// PCF8574 module at address 0x21.
const int PCF2_TRAP_LIMIT_OPEN_PIN   = 6; // PCF8574_2 Pin 6
const int PCF2_TRAP_LIMIT_CLOSED_PIN = 7; // PCF8574_2 Pin 7

// ================== Multiplexer Pins (Direct Arduino Digital/Analog) ==================
// Using available general-purpose digital pins for the multiplexer's select lines.
// Analog pins A2 and A3 are used as digital I/O to avoid conflict with SPI pins (10, 11, 12, 13).
// Analog pin A0 (D14) is used for the multiplexer's signal output.
const int MUX_S0_PIN = 8;  // Digital Pin D8
const int MUX_S1_PIN = 9;  // Digital Pin D9
const int MUX_S2_PIN = A2; // Using Analog Pin A2 (D16) as Digital I/O
const int MUX_S3_PIN = A3; // Using Analog Pin A3 (D17) as Digital I/O

// NOTE: The MUX 'EN' (Enable) pin must be tied to GND for the chip to be active.
const int MUX_SIG_PIN = A0; // Analog Pin A0 (D14) - Input from multiplexer's common output

// MUX Test Channels for validation
const int MUX_TEST_GND_CH = 15; // MUX Channel connected to GND
const int MUX_TEST_VCC_CH = 14; // MUX Channel connected to 5V


// ================== PCF8574 I2C Expanders ==================
// You need TWO PCF8574 modules to control 16 relay channels + door/trap motors.
// Ensure you set the address jumpers on each PCF8574 module to be unique.
const uint8_t PCF1_ADDR = 0x20; // Default I2C address for the first PCF8574
const uint8_t PCF2_ADDR = 0x21; // I2C address for the second PCF8574 (adjust jumpers on module)

// Forward declaration of PCF8574 to reduce header bloat.
// Files that actually use the PCF objects should include Adafruit_PCF8574.h.
class Adafruit_PCF8574;

// For classes, dependency injection will be used.
extern Adafruit_PCF8574 pcf1; 
extern Adafruit_PCF8574 pcf2;

// ================== RTC I2C Address ==================
// The DS3231 RTC module typically has two I2C devices.
const uint8_t RTC_CLOCK_ADDR = 0x68;  // The DS3231 clock chip itself.
const uint8_t RTC_EEPROM_ADDR = 0x51; // The EEPROM often included. (0x57 is also common).
                                      // Your previous config used 0x51.


// --- PCF8574_1 Pin Assignments (Valves, Pumps, Fans) ---
// These assignments use the 8 pins of the first PCF8574.
const int PCF1_VALVE1_PIN = 0; // PCF8574_1 Pin 0
const int PCF1_VALVE2_PIN = 1; // PCF8574_1 Pin 1
const int PCF1_VALVE3_PIN = 2; // PCF8574_1 Pin 2
extern String valveStatus[NUM_WATER_VALVES];

const int PCF1_PUMP1_PIN  = 3; // PCF8574_1 Pin 3
const int PCF1_PUMP2_PIN  = 4; // PCF8574_1 Pin 4
const int PCF1_PUMP3_PIN  = 5; // PCF8574_1 Pin 5
extern String pumpStatus[NUM_WATER_PUMPS];

const int PCF1_FAN1_PIN   = 6; // PCF8574_1 Pin 6
const int PCF1_FAN2_PIN   = 7; // PCF8574_1 Pin 7
extern String fanStatus[NUM_FANS];

// --- PCF8574_1 Pin Arrays ---
// --- Pin Mapping for Devices on PCF Expanders ---
// This new structure allows devices to be on either PCF expander.
// pcfIndex: 0 for pcf1, 1 for pcf2
extern const PinMapping VALVE_PINS[NUM_WATER_VALVES];

// --- Legacy Pin Arrays (for devices only on pcf1) ---
const int PCF1_PUMP_PINS[NUM_WATER_PUMPS]   = {PCF1_PUMP1_PIN, PCF1_PUMP2_PIN, PCF1_PUMP3_PIN};
const int PCF1_FAN_PINS[NUM_FANS]           = {PCF1_FAN1_PIN, PCF1_FAN2_PIN};


// ================== Water Flow Sensor Pins (Direct Arduino Digital Inputs) ==================
// These pins should be configured for interrupts in your setup() to accurately count pulses.
const int WATER_FLOW_METER_PINS[NUM_WATER_FLOW_METERS] = {12, 13, A1}; // Digital Pin D12, D13, Analog Pin A1 (D15)
extern String flowStatus[NUM_WATER_FLOW_METERS];
extern String mainFlowStatus;

// Flow meter pulse counting variables
extern volatile unsigned long flowPulseCount[NUM_WATER_FLOW_METERS];


// ================== Other Sensor Channels (Multiplexer to MUX_SIG_PIN=A0) ==================
// Now only includes Soil Sensors and Rain Sensor.
const int SOIL_SENSOR1_MUX_CH = 0;
const int SOIL_SENSOR2_MUX_CH = 1;
const int SOIL_SENSOR3_MUX_CH = 2;
extern String soilStatus[NUM_SOIL_SENSORS];

const int RAIN_SENSOR_MUX_CH  = 3; // Rain sensor now on MUX channel 3
extern String rainStatus;

extern String i2cScanResults;

// ================== Hardware Status Flags ==================
extern bool rtcStatus;
extern bool sdStatus;
extern bool pcf1Status;
extern bool pcf2Status;
extern bool muxStatus;

// ================== Irrigation Zone Configuration ==================
// This structure defines an independent irrigation zone, linking a pump,
// a valve, and a soil moisture sensor together.
const int NUM_IRRIGATION_ZONES = 3; // Must match NUM_SOIL_SENSORS for 1-to-1 mapping

struct IrrigationZone {
    const int pumpIndex;
    const int valveIndex;
    const int soilSensorIndex;
};

extern const IrrigationZone IRRIGATION_ZONES[NUM_IRRIGATION_ZONES];

// ================== Threshold Values ==================
// NOTE: The logic has been inverted to handle sensors where a LOWER reading means DRYER soil.
// You must recalibrate these values. For this logic, WET must be a higher value than DRY.
// The pump will turn ON when the reading drops BELOW the DRY threshold.
const int   SOIL_THRESHOLD_DRY = 80; // EXAMPLE: Value below which irrigation starts.
// The pump will turn OFF when the reading rises ABOVE the WET threshold.
const int   SOIL_THRESHOLD_WET = 110; // EXAMPLE: Must be higher than DRY for hysteresis.
const int   RAIN_THRESHOLD_WET = 500; // Example: analog value below which it is considered "raining"

const float FAN_TEMP_PRIMARY   = 28.0;
const float FAN_HUM_PRIMARY    = 75.0;
const float FAN_TEMP_SECONDARY = 33.0; // FAN_TEMP_PRIMARY + 5.0
const float FAN_HUM_SECONDARY  = 85.0; // FAN_HUM_PRIMARY + 10.0
const float CLIMATE_HYSTERESIS = 1.5;  // Degrees/Percent to drop before turning OFF

// ================== Sensor Validity Ranges ==================
// NOTE: Adjusted for the 10kΩ pull-down resistor. Disconnected pins will now read near 0.
const int SOIL_SENSOR_MIN_VALID = 15;   // Min expected analog reading for a connected sensor.
const int SOIL_SENSOR_MAX_VALID = 500;  // Max expected analog reading. Adjust if your "dry" value is higher.
const int RAIN_SENSOR_MIN_VALID = 50;
const int RAIN_SENSOR_MAX_VALID = 950;
// A disconnected analog pin often floats to 0 or 1023. This range helps filter that out.

// ================== Function Prototypes ==================
void selectMuxChannel(int channel);

// ISRs for flow meters, defined in main.cpp
void flowMeterISR0();
void flowMeterISR1();
void flowMeterISR2();

#pragma once

#define MAX_USERNAME_LENGTH 32
#define MAX_PASSWORD_LENGTH 64

extern char WEB_USERNAME[MAX_USERNAME_LENGTH + 1];
extern char WEB_PASSWORD[MAX_PASSWORD_LENGTH + 1];

// ================== LCD Display (I2C) ==================
const uint8_t LCD_I2C_ADDR = 0x27; // Common I2C address for LCDs
const uint8_t LCD_COLS = 20;       // LCD columns
const uint8_t LCD_ROWS = 4;        // LCD rows

#endif // CONFIG_H