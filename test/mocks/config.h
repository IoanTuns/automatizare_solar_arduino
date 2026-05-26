/**
 * @file config.h
 * @brief Stripped-down config header for native unit tests.
 *
 * Provides only the constants and types actually used by the files under
 * test, without pulling in hardware-specific Arduino libraries that are
 * unavailable on the native platform.
 */
#pragma once

#include "Arduino.h"
#include "Adafruit_PCF8574.h"
#include "DHT.h"

// ================== Firmware Info ==================
#define FIRMWARE_VERSION "1.2.0"
#define BUILD_ENV_NAME "native_test"

// ================== Serial & Web Server ==================
const long     SERIAL_BAUD_RATE = 115200;
const uint16_t WEB_SERVER_PORT  = 80;

// ================== System Capacity Constants ==================
const int NUM_SOIL_SENSORS      = 3;
const int NUM_OF_DOORS          = 2;
const int NUM_WATER_VALVES      = 3;
const int NUM_WATER_PUMPS       = 3;
const int NUM_WATER_FLOW_METERS = 3;
const int NUM_FANS              = 2;
const int NUM_IRRIGATION_ZONES  = 3;

const uint8_t NUM_TEMP_SENSORS = 2;
const uint8_t NUM_RAIN_SENSORS = 1;
const uint8_t NUM_TOP_TRAPS    = 1;

// ================== DHT ==================
const uint8_t DHTTYPE = 22;

// ================== Fan Thresholds ==================
const float FAN_TEMP_PRIMARY   = 28.0f;
const float FAN_HUM_PRIMARY    = 75.0f;
const float FAN_TEMP_SECONDARY = 33.0f;
const float FAN_HUM_SECONDARY  = 85.0f;
const float CLIMATE_HYSTERESIS = 1.5f;

// ================== PCF Addresses ==================
const uint8_t PCF1_ADDR = 0x20;
const uint8_t PCF2_ADDR = 0x21;

// ================== PCF1 Fan Pins ==================
const int PCF1_FAN1_PIN = 6;
const int PCF1_FAN2_PIN = 7;
const int PCF1_FAN_PINS[NUM_FANS] = {PCF1_FAN1_PIN, PCF1_FAN2_PIN};

// ================== PCF1 Valve/Pump Pins ==================
const int PCF1_VALVE1_PIN = 0;
const int PCF1_VALVE2_PIN = 1;
const int PCF1_VALVE3_PIN = 2;
const int PCF1_PUMP1_PIN  = 3;
const int PCF1_PUMP2_PIN  = 4;
const int PCF1_PUMP3_PIN  = 5;

// ================== PCF Extern Objects ==================
extern Adafruit_PCF8574 pcf1;
extern Adafruit_PCF8574 pcf2;

// ================== Global Status Strings ==================
extern String fanStatus[NUM_FANS];
extern String doorStatus[NUM_OF_DOORS];
extern String trapStatus;
extern String pumpStatus[NUM_WATER_PUMPS];
extern String valveStatus[NUM_WATER_VALVES];
extern String soilStatus[NUM_SOIL_SENSORS];
extern String rainStatus;
extern String flowStatus[NUM_WATER_FLOW_METERS];
extern String mainFlowStatus;
extern String i2cScanResults;

// ================== Hardware Status Flags ==================
extern bool rtcStatus;
extern bool sdStatus;
extern bool pcf1Status;
extern bool pcf2Status;
extern bool muxStatus;

// ================== Web Auth Limits ==================
#define MAX_USERNAME_LENGTH 32
#define MAX_PASSWORD_LENGTH 64

extern char WEB_USERNAME[MAX_USERNAME_LENGTH + 1];
extern char WEB_PASSWORD[MAX_PASSWORD_LENGTH + 1];

// ================== LCD ==================
const uint8_t LCD_I2C_ADDR = 0x27;
const uint8_t LCD_COLS     = 20;
const uint8_t LCD_ROWS     = 4;

// ================== Timing ==================
const unsigned long SENSOR_READ_INTERVAL_MS    = 2000;
const unsigned long DISPLAY_UPDATE_INTERVAL_MS = 10000;
const unsigned long WIFI_CONNECT_TIMEOUT_MS    = 30000;
const uint8_t       MAIN_LOOP_DELAY_MS         = 100;

// ================== Sensor Validity ==================
const int SOIL_SENSOR_MIN_VALID = 15;
const int SOIL_SENSOR_MAX_VALID = 500;
const int RAIN_SENSOR_MIN_VALID = 50;
const int RAIN_SENSOR_MAX_VALID = 950;

// ================== Pin Mapping ==================
struct PinMapping {
    uint8_t pcfIndex;
    uint8_t pin;
};

extern const PinMapping VALVE_PINS[NUM_WATER_VALVES];

struct IrrigationZone {
    const int pumpIndex;
    const int valveIndex;
    const int soilSensorIndex;
};

extern const IrrigationZone IRRIGATION_ZONES[NUM_IRRIGATION_ZONES];

// ================== Flow Pulse Counts ==================
extern volatile unsigned long flowPulseCount[NUM_WATER_FLOW_METERS];

// ================== MUX ==================
const int MUX_TEST_GND_CH = 15;
const int MUX_TEST_VCC_CH = 14;
const int SOIL_SENSOR1_MUX_CH = 0;
const int SOIL_SENSOR2_MUX_CH = 1;
const int SOIL_SENSOR3_MUX_CH = 2;
const int RAIN_SENSOR_MUX_CH  = 3;
const int MUX_SIG_PIN = 0; // A0 stub

// ================== Function Prototypes ==================
void selectMuxChannel(int channel);
void flowMeterISR0();
void flowMeterISR1();
void flowMeterISR2();