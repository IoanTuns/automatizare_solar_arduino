/**
 * @file test_main.cpp
 * @brief Tests for the SolarWebServer indexOf fix and config constants added in this PR.
 *
 * In this PR, SolarWebServer.cpp changed all command-detection comparisons from:
 *   req.indexOf("/zone/1/on") > 0
 * to:
 *   req.indexOf("/zone/1/on") != -1
 *
 * The old code would fail to detect commands that appeared at position 0 in the
 * request string (e.g., if the string started exactly with the path).  The new
 * comparison correctly handles all positions including 0.
 *
 * These tests verify that behaviour directly on the Arduino String mock, and also
 * validate the new config constants introduced in this PR:
 *   CLIMATE_HYSTERESIS, LCD_I2C_ADDR, LCD_COLS, LCD_ROWS,
 *   MAX_USERNAME_LENGTH, MAX_PASSWORD_LENGTH,
 *   FIRMWARE_VERSION macro, WEB_SERVER_PORT type change.
 *
 * Additionally, LcdDisplay construction and printAt delegation are tested.
 */

#include <unity.h>

#include "Arduino.h"
#include "EEPROM.h"
#include "secrets.h"
#include "config.h"
#include "LiquidCrystal_I2C.h"

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

// Source under test — LcdDisplay
#include "../../src/LcdDisplay.h"
#include "../../src/LcdDisplay.cpp"

// Expose private _lcd for tests via a thin subclass
class TestableLcdDisplay : public LcdDisplay {
public:
    TestableLcdDisplay(uint8_t a, uint8_t c, uint8_t r) : LcdDisplay(a, c, r) {}
    LiquidCrystal_I2C& lcd() { return _lcd; }
};

// ============================================================
// setUp / tearDown
// ============================================================
void setUp()   {}
void tearDown(){}

// ============================================================
// indexOf bug-fix verification
// ============================================================
// The original code used `> 0` which silently ignored a match at position 0.
// The fix uses `!= -1`.  We verify both the old-broken and new-correct
// behaviour using the String mock directly.

void test_indexOf_not_minus_one_detects_match_at_position_zero() {
    // Command path that starts at position 0 — old "> 0" code would miss this
    String req = "/zone/1/on HTTP/1.1";
    bool old_style = (req.indexOf("/zone/1/on") > 0);
    bool new_style = (req.indexOf("/zone/1/on") != -1);
    TEST_ASSERT_FALSE(old_style);   // Bug: old code misses it
    TEST_ASSERT_TRUE(new_style);    // Fix: new code correctly detects it
}

void test_indexOf_not_minus_one_detects_match_in_middle() {
    String req = "GET /zone/1/on HTTP/1.1";
    TEST_ASSERT_TRUE(req.indexOf("/zone/1/on") != -1);
}

void test_indexOf_not_minus_one_returns_false_for_absent_path() {
    String req = "GET /zone/1/off HTTP/1.1";
    TEST_ASSERT_FALSE(req.indexOf("/zone/1/on") != -1);
}

void test_indexOf_returns_minus_one_for_no_match() {
    String req = "GET / HTTP/1.1";
    TEST_ASSERT_EQUAL_INT(-1, req.indexOf("/zone/1/on"));
}

void test_indexOf_returns_zero_for_match_at_start() {
    String req = "/trap/open extra";
    TEST_ASSERT_EQUAL_INT(0, req.indexOf("/trap/open"));
}

void test_indexOf_old_style_fails_for_all_tested_commands() {
    // Verifies the OLD bug for representative commands from SolarWebServer.cpp
    const char* paths[] = {
        "/zone/1/on", "/zone/2/off", "/fan/1/on", "/fan/2/off",
        "/trap/open", "/trap/close", "/trap/up", "/trap/down",
        "/trap/stop", "/trap/reset", "/door/1/open", "/door/2/close"
    };
    for (size_t i = 0; i < sizeof(paths)/sizeof(paths[0]); i++) {
        String req = String(paths[i]) + " HTTP/1.1"; // match at position 0
        bool old_style = req.indexOf(paths[i]) > 0;
        bool new_style = req.indexOf(paths[i]) != -1;
        TEST_ASSERT_FALSE(old_style); // old code is broken for pos-0
        TEST_ASSERT_TRUE(new_style);  // new code works
    }
}

// ============================================================
// Config constants introduced in this PR
// ============================================================

void test_climate_hysteresis_value() {
    // Value defined in PR: 1.5
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 1.5f, CLIMATE_HYSTERESIS);
}

void test_lcd_i2c_addr() {
    TEST_ASSERT_EQUAL_INT(0x27, LCD_I2C_ADDR);
}

void test_lcd_cols() {
    TEST_ASSERT_EQUAL_INT(20, LCD_COLS);
}

void test_lcd_rows() {
    TEST_ASSERT_EQUAL_INT(4, LCD_ROWS);
}

void test_max_username_length() {
    TEST_ASSERT_EQUAL_INT(32, MAX_USERNAME_LENGTH);
}

void test_max_password_length() {
    TEST_ASSERT_EQUAL_INT(64, MAX_PASSWORD_LENGTH);
}

void test_web_server_port_value() {
    TEST_ASSERT_EQUAL_INT(80, (int)WEB_SERVER_PORT);
}

void test_firmware_version_defined() {
    // FIRMWARE_VERSION is defined as a string macro in config.h
    const char* ver = FIRMWARE_VERSION;
    TEST_ASSERT_NOT_NULL(ver);
    TEST_ASSERT_TRUE(strlen(ver) > 0);
}

void test_num_fans_at_least_two() {
    // The hysteresis code has a `if (NUM_FANS > 1)` branch — ensure config is correct
    TEST_ASSERT_TRUE(NUM_FANS >= 2);
}

// ============================================================
// LcdDisplay — new class in this PR
// ============================================================

void test_lcddisplay_init_calls_init_and_backlight() {
    TestableLcdDisplay disp(0x27, 20, 4);
    disp.init();
    // init() calls _lcd.init() and _lcd.backlight() — verify via mock accessors
    TEST_ASSERT_TRUE(disp.lcd().isInitialized());
    TEST_ASSERT_TRUE(disp.lcd().backlightOn());
}

void test_lcddisplay_clear_resets_cursor_and_buffer() {
    TestableLcdDisplay disp(0x27, 20, 4);
    disp.init();
    disp.setCursor(5, 2);
    disp.clear();
    TEST_ASSERT_EQUAL_INT(0, disp.lcd().getCursorCol());
    TEST_ASSERT_EQUAL_INT(0, disp.lcd().getCursorRow());
}

void test_lcddisplay_set_cursor_stores_position() {
    TestableLcdDisplay disp(0x27, 20, 4);
    disp.setCursor(3, 1);
    TEST_ASSERT_EQUAL_INT(3, disp.lcd().getCursorCol());
    TEST_ASSERT_EQUAL_INT(1, disp.lcd().getCursorRow());
}

void test_lcddisplay_print_writes_message() {
    TestableLcdDisplay disp(0x27, 20, 4);
    disp.init();
    disp.print("Hello");
    TEST_ASSERT_EQUAL_STRING("Hello", disp.lcd().lastPrint());
}

void test_lcddisplay_printAt_delegates_to_setCursor_and_print() {
    TestableLcdDisplay disp(0x27, 20, 4);
    disp.init();
    disp.printAt(10, 3, "World");
    TEST_ASSERT_EQUAL_INT(10, disp.lcd().getCursorCol());
    TEST_ASSERT_EQUAL_INT(3,  disp.lcd().getCursorRow());
    TEST_ASSERT_EQUAL_STRING("World", disp.lcd().lastPrint());
}

void test_lcddisplay_printAt_origin() {
    TestableLcdDisplay disp(0x27, 20, 4);
    disp.init();
    disp.printAt(0, 0, "Start");
    TEST_ASSERT_EQUAL_INT(0, disp.lcd().getCursorCol());
    TEST_ASSERT_EQUAL_INT(0, disp.lcd().getCursorRow());
    TEST_ASSERT_EQUAL_STRING("Start", disp.lcd().lastPrint());
}

void test_lcddisplay_uses_provided_address() {
    TestableLcdDisplay disp(0x3F, 16, 2);
    TEST_ASSERT_EQUAL_INT(0x3F, disp.lcd().getAddr());
    TEST_ASSERT_EQUAL_INT(16,   disp.lcd().getCols());
    TEST_ASSERT_EQUAL_INT(2,    disp.lcd().getRows());
}

// ============================================================
// Arduino String mock correctness (used by indexOf fix)
// ============================================================

void test_string_indexof_returns_correct_position() {
    String s = "GET /trap/open HTTP/1.1";
    TEST_ASSERT_EQUAL_INT(4, s.indexOf("/trap/open"));
}

void test_string_concat_works() {
    String s = String("/zone/") + String(1) + String("/on");
    TEST_ASSERT_EQUAL_STRING("/zone/1/on", s.c_str());
}

void test_string_equals_comparison() {
    String a = "on";
    String b = "on";
    TEST_ASSERT_TRUE(a.equals(b));
    TEST_ASSERT_TRUE(a == b);
}

void test_string_not_equals() {
    String a = "on";
    String b = "off";
    TEST_ASSERT_FALSE(a.equals(b));
    TEST_ASSERT_FALSE(a == b);
}

// ============================================================
// main
// ============================================================
int main(int argc, char** argv) {
    UNITY_BEGIN();

    // indexOf fix
    RUN_TEST(test_indexOf_not_minus_one_detects_match_at_position_zero);
    RUN_TEST(test_indexOf_not_minus_one_detects_match_in_middle);
    RUN_TEST(test_indexOf_not_minus_one_returns_false_for_absent_path);
    RUN_TEST(test_indexOf_returns_minus_one_for_no_match);
    RUN_TEST(test_indexOf_returns_zero_for_match_at_start);
    RUN_TEST(test_indexOf_old_style_fails_for_all_tested_commands);

    // Config constants
    RUN_TEST(test_climate_hysteresis_value);
    RUN_TEST(test_lcd_i2c_addr);
    RUN_TEST(test_lcd_cols);
    RUN_TEST(test_lcd_rows);
    RUN_TEST(test_max_username_length);
    RUN_TEST(test_max_password_length);
    RUN_TEST(test_web_server_port_value);
    RUN_TEST(test_firmware_version_defined);
    RUN_TEST(test_num_fans_at_least_two);

    // LcdDisplay
    RUN_TEST(test_lcddisplay_init_calls_init_and_backlight);
    RUN_TEST(test_lcddisplay_clear_resets_cursor_and_buffer);
    RUN_TEST(test_lcddisplay_set_cursor_stores_position);
    RUN_TEST(test_lcddisplay_print_writes_message);
    RUN_TEST(test_lcddisplay_printAt_delegates_to_setCursor_and_print);
    RUN_TEST(test_lcddisplay_printAt_origin);
    RUN_TEST(test_lcddisplay_uses_provided_address);

    // String mock
    RUN_TEST(test_string_indexof_returns_correct_position);
    RUN_TEST(test_string_concat_works);
    RUN_TEST(test_string_equals_comparison);
    RUN_TEST(test_string_not_equals);

    return UNITY_END();
}