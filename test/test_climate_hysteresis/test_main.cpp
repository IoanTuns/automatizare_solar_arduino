/**
 * @file test_main.cpp
 * @brief Unit tests for the ClimateControl hysteresis logic added in this PR.
 *
 * The new hysteresis logic (ClimateControl.cpp) changes how fans are
 * controlled based on current state:
 *
 *  Fan 1 (Primary):
 *   - OFF → ON  when (tempInt > FAN_TEMP_PRIMARY  || humInt > FAN_HUM_PRIMARY)
 *   - ON  → OFF when (tempInt ≤ FAN_TEMP_PRIMARY-HYSTERESIS || humInt ≤ FAN_HUM_PRIMARY-HYSTERESIS)
 *   - ON  → stays ON while (tempInt > FAN_TEMP_PRIMARY-HYSTERESIS) AND
 *                           (humInt  > FAN_HUM_PRIMARY-HYSTERESIS)
 *
 *  Fan 2 (Secondary, temperature-only logic):
 *   - OFF → ON  when (tempInt > FAN_TEMP_SECONDARY)
 *   - ON  → stays ON while (tempInt > FAN_TEMP_SECONDARY-HYSTERESIS)
 *   - ON  → OFF when (tempInt ≤ FAN_TEMP_SECONDARY-HYSTERESIS)
 *
 *  Both fans ignore invalid sensor data (sensors.valid == false).
 */

#include <unity.h>

#include "Arduino.h"
#include "EEPROM.h"
#include "secrets.h"
#include "config.h"

// Global mock definitions for this test binary
unsigned long _mock_millis = 0;
_SerialStub   Serial;
_WireStub     Wire;
_EEPROMMock   EEPROM;
Adafruit_PCF8574 pcf1, pcf2;

String fanStatus[NUM_FANS]               = {"off","off"};
String doorStatus[NUM_OF_DOORS]          = {"closed","closed"};
String trapStatus                        = "stopped";
String pumpStatus[NUM_WATER_PUMPS]       = {"off","off","off"};
String valveStatus[NUM_WATER_VALVES]     = {"closed","closed","closed"};
String soilStatus[NUM_SOIL_SENSORS]      = {"invalid","invalid","invalid"};
String rainStatus                        = "dry";
String flowStatus[NUM_WATER_FLOW_METERS] = {"off","off","off"};
String mainFlowStatus                    = "off";
String i2cScanResults                    = "";
bool rtcStatus=false,sdStatus=false,pcf1Status=false,pcf2Status=false,muxStatus=false;
char WEB_USERNAME[MAX_USERNAME_LENGTH+1] = "admin";
char WEB_PASSWORD[MAX_PASSWORD_LENGTH+1] = "password";
const PinMapping VALVE_PINS[NUM_WATER_VALVES] = {{0,0},{0,1},{0,2}};
const IrrigationZone IRRIGATION_ZONES[NUM_IRRIGATION_ZONES] = {{0,0,0},{1,1,1},{2,2,2}};
volatile unsigned long flowPulseCount[NUM_WATER_FLOW_METERS] = {0,0,0};
void selectMuxChannel(int){}
void flowMeterISR0(){} void flowMeterISR1(){} void flowMeterISR2(){}

#include "../../src/SensorData.h"
#include "../../src/ClimateControl.h"
#include "../../src/ClimateControl.cpp"

// ============================================================
// Helpers
// ============================================================

/** Build a valid SensorData with the supplied temp/hum values. */
static SensorData makeSensors(float tempInt, float humInt,
                               float tempExt = 20.0f, float humExt = 50.0f) {
    SensorData s;
    s.valid    = true;
    s.tempInt  = tempInt;
    s.humInt   = humInt;
    s.tempExt  = tempExt;
    s.humExt   = humExt;
    s.rainSensorValue = 100;
    for (int i = 0; i < NUM_SOIL_SENSORS; i++)      s.soilMoisture[i] = 200;
    for (int i = 0; i < NUM_WATER_FLOW_METERS; i++) s.flowRate[i]     = 0.0f;
    return s;
}

// ============================================================
// setUp / tearDown — reset global fan state and PCF before each test
// ============================================================
static Adafruit_PCF8574 testPcf;
static ClimateControl*  ctrl = nullptr;

void setUp() {
    fanStatus[0] = "off";
    fanStatus[1] = "off";
    testPcf = Adafruit_PCF8574();
    if (ctrl) { delete ctrl; ctrl = nullptr; }
    ctrl = new ClimateControl(testPcf);
}

void tearDown() {
    if (ctrl) { delete ctrl; ctrl = nullptr; }
}

// ============================================================
// Fan 1 (Primary) — turn-on conditions
// ============================================================

void test_fan1_turns_on_when_temp_exceeds_primary() {
    SensorData s = makeSensors(FAN_TEMP_PRIMARY + 0.1f, FAN_HUM_PRIMARY - 5.0f);
    ctrl->control(s);
    TEST_ASSERT_EQUAL_STRING("on", fanStatus[0].c_str());
}

void test_fan1_turns_on_when_hum_exceeds_primary() {
    SensorData s = makeSensors(FAN_TEMP_PRIMARY - 5.0f, FAN_HUM_PRIMARY + 0.1f);
    ctrl->control(s);
    TEST_ASSERT_EQUAL_STRING("on", fanStatus[0].c_str());
}

void test_fan1_stays_off_below_both_thresholds() {
    SensorData s = makeSensors(FAN_TEMP_PRIMARY - 1.0f, FAN_HUM_PRIMARY - 1.0f);
    ctrl->control(s);
    TEST_ASSERT_EQUAL_STRING("off", fanStatus[0].c_str());
}

void test_fan1_stays_off_exactly_at_primary_thresholds() {
    // Exactly at threshold (not above) — should stay off
    SensorData s = makeSensors(FAN_TEMP_PRIMARY, FAN_HUM_PRIMARY);
    ctrl->control(s);
    TEST_ASSERT_EQUAL_STRING("off", fanStatus[0].c_str());
}

// ============================================================
// Fan 1 (Primary) — hysteresis: on → stays on
// ============================================================

void test_fan1_stays_on_when_temp_in_hysteresis_band() {
    fanStatus[0] = "on";
    // Temp is above (primary - hysteresis) and hum is also above (primary - hysteresis)
    float tempInBand = FAN_TEMP_PRIMARY - CLIMATE_HYSTERESIS + 0.1f;
    float humInBand  = FAN_HUM_PRIMARY  - CLIMATE_HYSTERESIS + 0.1f;
    SensorData s = makeSensors(tempInBand, humInBand);
    ctrl->control(s);
    TEST_ASSERT_EQUAL_STRING("on", fanStatus[0].c_str());
}

void test_fan1_stays_on_at_hysteresis_boundary() {
    fanStatus[0] = "on";
    // Exactly at the hysteresis boundary (just above threshold - hysteresis)
    float tempAtBound = FAN_TEMP_PRIMARY - CLIMATE_HYSTERESIS + 0.01f;
    float humAtBound  = FAN_HUM_PRIMARY  - CLIMATE_HYSTERESIS + 0.01f;
    SensorData s = makeSensors(tempAtBound, humAtBound);
    ctrl->control(s);
    TEST_ASSERT_EQUAL_STRING("on", fanStatus[0].c_str());
}

// ============================================================
// Fan 1 (Primary) — hysteresis: on → turns off
// ============================================================

void test_fan1_turns_off_when_temp_drops_below_hysteresis() {
    fanStatus[0] = "on";
    // Temp drops below (primary - hysteresis), hum still above
    float tempBelowBand = FAN_TEMP_PRIMARY - CLIMATE_HYSTERESIS - 0.1f;
    float humAboveBand  = FAN_HUM_PRIMARY  - CLIMATE_HYSTERESIS + 1.0f;
    SensorData s = makeSensors(tempBelowBand, humAboveBand);
    ctrl->control(s);
    // Fan must turn off: condition requires BOTH temp AND hum > (threshold - hysteresis)
    TEST_ASSERT_EQUAL_STRING("off", fanStatus[0].c_str());
}

void test_fan1_turns_off_when_hum_drops_below_hysteresis() {
    fanStatus[0] = "on";
    // Hum drops below (primary - hysteresis), temp still above
    float tempAboveBand = FAN_TEMP_PRIMARY - CLIMATE_HYSTERESIS + 1.0f;
    float humBelowBand  = FAN_HUM_PRIMARY  - CLIMATE_HYSTERESIS - 0.1f;
    SensorData s = makeSensors(tempAboveBand, humBelowBand);
    ctrl->control(s);
    TEST_ASSERT_EQUAL_STRING("off", fanStatus[0].c_str());
}

void test_fan1_turns_off_when_both_drop_below_hysteresis() {
    fanStatus[0] = "on";
    float tempBelow = FAN_TEMP_PRIMARY - CLIMATE_HYSTERESIS - 2.0f;
    float humBelow  = FAN_HUM_PRIMARY  - CLIMATE_HYSTERESIS - 2.0f;
    SensorData s = makeSensors(tempBelow, humBelow);
    ctrl->control(s);
    TEST_ASSERT_EQUAL_STRING("off", fanStatus[0].c_str());
}

// ============================================================
// Fan 2 (Secondary) — temperature-only logic
// ============================================================

void test_fan2_turns_on_when_temp_exceeds_secondary() {
    fanStatus[0] = "off";
    fanStatus[1] = "off";
    SensorData s = makeSensors(FAN_TEMP_SECONDARY + 0.1f, FAN_HUM_PRIMARY - 5.0f);
    ctrl->control(s);
    TEST_ASSERT_EQUAL_STRING("on", fanStatus[1].c_str());
}

void test_fan2_stays_off_below_secondary_threshold() {
    fanStatus[1] = "off";
    SensorData s = makeSensors(FAN_TEMP_SECONDARY - 1.0f, 50.0f);
    ctrl->control(s);
    TEST_ASSERT_EQUAL_STRING("off", fanStatus[1].c_str());
}

void test_fan2_stays_on_within_hysteresis_band() {
    fanStatus[1] = "on";
    float tempInBand = FAN_TEMP_SECONDARY - CLIMATE_HYSTERESIS + 0.1f;
    SensorData s = makeSensors(tempInBand, 50.0f);
    ctrl->control(s);
    TEST_ASSERT_EQUAL_STRING("on", fanStatus[1].c_str());
}

void test_fan2_turns_off_below_hysteresis_band() {
    fanStatus[1] = "on";
    float tempBelowBand = FAN_TEMP_SECONDARY - CLIMATE_HYSTERESIS - 0.1f;
    SensorData s = makeSensors(tempBelowBand, 50.0f);
    ctrl->control(s);
    TEST_ASSERT_EQUAL_STRING("off", fanStatus[1].c_str());
}

void test_fan2_stays_off_exactly_at_secondary_threshold() {
    fanStatus[1] = "off";
    // Exactly at threshold — not above, so stays off
    SensorData s = makeSensors(FAN_TEMP_SECONDARY, 50.0f);
    ctrl->control(s);
    TEST_ASSERT_EQUAL_STRING("off", fanStatus[1].c_str());
}

// ============================================================
// Invalid sensor data guard
// ============================================================

void test_control_ignores_invalid_sensor_data() {
    fanStatus[0] = "off";
    fanStatus[1] = "off";

    SensorData s = makeSensors(99.9f, 99.9f); // extreme values
    s.valid = false;                            // marked invalid
    ctrl->control(s);

    // Fans must NOT change state
    TEST_ASSERT_EQUAL_STRING("off", fanStatus[0].c_str());
    TEST_ASSERT_EQUAL_STRING("off", fanStatus[1].c_str());
}

void test_control_with_valid_flag_true_still_applies_logic() {
    SensorData s = makeSensors(FAN_TEMP_PRIMARY + 1.0f, FAN_HUM_PRIMARY + 1.0f);
    s.valid = true;
    ctrl->control(s);
    TEST_ASSERT_EQUAL_STRING("on", fanStatus[0].c_str());
}

// ============================================================
// controlFan() directly
// ============================================================

void test_controlFan_sets_fanStatus_on() {
    ctrl->controlFan(0, true);
    TEST_ASSERT_EQUAL_STRING("on", fanStatus[0].c_str());
}

void test_controlFan_sets_fanStatus_off() {
    fanStatus[0] = "on";
    ctrl->controlFan(0, false);
    TEST_ASSERT_EQUAL_STRING("off", fanStatus[0].c_str());
}

void test_controlFan_out_of_range_index_no_crash() {
    // Index >= NUM_FANS — must not crash or corrupt fanStatus
    ctrl->controlFan(NUM_FANS, true);
    ctrl->controlFan(-1, true);
    // Valid fan statuses remain unchanged
    TEST_ASSERT_EQUAL_STRING("off", fanStatus[0].c_str());
    TEST_ASSERT_EQUAL_STRING("off", fanStatus[1].c_str());
}

void test_controlFan_second_fan() {
    ctrl->controlFan(1, true);
    TEST_ASSERT_EQUAL_STRING("on", fanStatus[1].c_str());
}

// ============================================================
// Regression: check CLIMATE_HYSTERESIS constant value
// ============================================================

void test_climate_hysteresis_constant_is_positive() {
    TEST_ASSERT_TRUE(CLIMATE_HYSTERESIS > 0.0f);
}

void test_climate_hysteresis_constant_less_than_primary_thresholds() {
    // Hysteresis must be smaller than both primary thresholds to be meaningful
    TEST_ASSERT_TRUE(CLIMATE_HYSTERESIS < FAN_TEMP_PRIMARY);
    TEST_ASSERT_TRUE(CLIMATE_HYSTERESIS < FAN_HUM_PRIMARY);
}

// ============================================================
// main
// ============================================================
int main(int argc, char** argv) {
    UNITY_BEGIN();

    // Fan 1 turn-on
    RUN_TEST(test_fan1_turns_on_when_temp_exceeds_primary);
    RUN_TEST(test_fan1_turns_on_when_hum_exceeds_primary);
    RUN_TEST(test_fan1_stays_off_below_both_thresholds);
    RUN_TEST(test_fan1_stays_off_exactly_at_primary_thresholds);

    // Fan 1 hysteresis: stays on
    RUN_TEST(test_fan1_stays_on_when_temp_in_hysteresis_band);
    RUN_TEST(test_fan1_stays_on_at_hysteresis_boundary);

    // Fan 1 hysteresis: turns off
    RUN_TEST(test_fan1_turns_off_when_temp_drops_below_hysteresis);
    RUN_TEST(test_fan1_turns_off_when_hum_drops_below_hysteresis);
    RUN_TEST(test_fan1_turns_off_when_both_drop_below_hysteresis);

    // Fan 2 (secondary)
    RUN_TEST(test_fan2_turns_on_when_temp_exceeds_secondary);
    RUN_TEST(test_fan2_stays_off_below_secondary_threshold);
    RUN_TEST(test_fan2_stays_on_within_hysteresis_band);
    RUN_TEST(test_fan2_turns_off_below_hysteresis_band);
    RUN_TEST(test_fan2_stays_off_exactly_at_secondary_threshold);

    // Invalid sensor guard
    RUN_TEST(test_control_ignores_invalid_sensor_data);
    RUN_TEST(test_control_with_valid_flag_true_still_applies_logic);

    // controlFan() directly
    RUN_TEST(test_controlFan_sets_fanStatus_on);
    RUN_TEST(test_controlFan_sets_fanStatus_off);
    RUN_TEST(test_controlFan_out_of_range_index_no_crash);
    RUN_TEST(test_controlFan_second_fan);

    // Regression constants
    RUN_TEST(test_climate_hysteresis_constant_is_positive);
    RUN_TEST(test_climate_hysteresis_constant_less_than_primary_thresholds);

    return UNITY_END();
}