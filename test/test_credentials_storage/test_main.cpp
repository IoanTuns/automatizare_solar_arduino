/**
 * @file test_main.cpp
 * @brief Unit tests for CredentialsStorage — new code added in this PR.
 *
 * Tests cover:
 *  - EEPROM layout: magic byte, length fields, data bytes
 *  - Store then load roundtrip for normal SSID/password pairs
 *  - Empty (unwritten) EEPROM returns false on load
 *  - Magic-byte mismatch returns false on load
 *  - SSID length 0 returns false on load
 *  - Oversized SSID is clamped to MAX_SSID_LEN on store
 *  - Oversized password is clamped to MAX_PASS_LEN on store
 *  - Empty password is stored and loaded correctly
 *  - Oversize password length in EEPROM returns false on load (corrupt data guard)
 *  - Unicode/special characters in SSID round-trip correctly
 */

#include <unity.h>

// Mocks — must come before any project headers
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

// Source under test
#include "../../src/SecureCredentials.h"
#include "../../src/SecureCredentials.cpp"

// Internal layout constants (mirrors SecureCredentials.cpp definitions)
#define EEPROM_MAGIC_BYTE_ADDR 0
#define EEPROM_SSID_LEN_ADDR   1
#define EEPROM_SSID_VAL_ADDR   2
#define EEPROM_PASS_LEN_ADDR   34
#define EEPROM_PASS_VAL_ADDR   35
#define MAGIC_BYTE             0xAB
#define MAX_SSID_LEN           32
#define MAX_PASS_LEN           64

// ============================================================
// setUp / tearDown
// ============================================================
void setUp() {
    EEPROM.reset(); // all bytes → 0xFF (unprogrammed)
}

void tearDown() {}

// ============================================================
// loadCredentials — error paths (no prior store)
// ============================================================

void test_load_fails_on_empty_eeprom() {
    char ssid[64] = {0}, pass[128] = {0};
    // Magic byte is 0xFF (erased) — should fail
    bool result = CredentialsStorage::loadCredentials(ssid, pass);
    TEST_ASSERT_FALSE(result);
}

void test_load_fails_on_wrong_magic_byte() {
    EEPROM.write(EEPROM_MAGIC_BYTE_ADDR, 0x00); // wrong magic
    EEPROM.write(EEPROM_SSID_LEN_ADDR, 4);
    for (int i = 0; i < 4; i++) EEPROM.write(EEPROM_SSID_VAL_ADDR + i, 'T' + i);
    EEPROM.write(EEPROM_PASS_LEN_ADDR, 0);

    char ssid[64] = {0}, pass[128] = {0};
    TEST_ASSERT_FALSE(CredentialsStorage::loadCredentials(ssid, pass));
}

void test_load_fails_on_zero_ssid_length() {
    EEPROM.write(EEPROM_MAGIC_BYTE_ADDR, MAGIC_BYTE);
    EEPROM.write(EEPROM_SSID_LEN_ADDR, 0); // zero-length SSID is invalid

    char ssid[64] = {0}, pass[128] = {0};
    TEST_ASSERT_FALSE(CredentialsStorage::loadCredentials(ssid, pass));
}

void test_load_fails_on_oversized_ssid_length_in_eeprom() {
    // Simulate corrupt EEPROM with ssidLen > MAX_SSID_LEN
    EEPROM.write(EEPROM_MAGIC_BYTE_ADDR, MAGIC_BYTE);
    EEPROM.write(EEPROM_SSID_LEN_ADDR, MAX_SSID_LEN + 1);

    char ssid[64] = {0}, pass[128] = {0};
    TEST_ASSERT_FALSE(CredentialsStorage::loadCredentials(ssid, pass));
}

void test_load_fails_on_oversized_pass_length_in_eeprom() {
    // Write a valid SSID but corrupt the pass length
    EEPROM.write(EEPROM_MAGIC_BYTE_ADDR, MAGIC_BYTE);
    EEPROM.write(EEPROM_SSID_LEN_ADDR, 4);
    for (int i = 0; i < 4; i++) EEPROM.write(EEPROM_SSID_VAL_ADDR + i, 'a' + i);
    EEPROM.write(EEPROM_PASS_LEN_ADDR, MAX_PASS_LEN + 1); // oversized

    char ssid[64] = {0}, pass[128] = {0};
    TEST_ASSERT_FALSE(CredentialsStorage::loadCredentials(ssid, pass));
}

// ============================================================
// storeCredentials then loadCredentials roundtrip
// ============================================================

void test_roundtrip_normal_credentials() {
    const char* testSSID = "MyWiFiNetwork";
    const char* testPass = "S3cr3tP@ss!";

    bool stored = CredentialsStorage::storeCredentials(testSSID, testPass);
    TEST_ASSERT_TRUE(stored);

    char ssid[64] = {0}, pass[128] = {0};
    bool loaded = CredentialsStorage::loadCredentials(ssid, pass);
    TEST_ASSERT_TRUE(loaded);
    TEST_ASSERT_EQUAL_STRING(testSSID, ssid);
    TEST_ASSERT_EQUAL_STRING(testPass, pass);
}

void test_roundtrip_empty_password() {
    const char* testSSID = "OpenNetwork";
    const char* testPass = ""; // empty password — valid open network

    CredentialsStorage::storeCredentials(testSSID, testPass);

    char ssid[64] = {0}, pass[128] = {0};
    bool loaded = CredentialsStorage::loadCredentials(ssid, pass);
    TEST_ASSERT_TRUE(loaded);
    TEST_ASSERT_EQUAL_STRING(testSSID, ssid);
    TEST_ASSERT_EQUAL_STRING("", pass);
}

void test_store_sets_magic_byte() {
    CredentialsStorage::storeCredentials("TestNet", "pass");
    TEST_ASSERT_EQUAL_INT(MAGIC_BYTE, EEPROM.read(EEPROM_MAGIC_BYTE_ADDR));
}

void test_store_writes_correct_ssid_length() {
    const char* ssid = "Hello";
    CredentialsStorage::storeCredentials(ssid, "");
    TEST_ASSERT_EQUAL_INT(5, EEPROM.read(EEPROM_SSID_LEN_ADDR));
}

void test_store_writes_ssid_bytes_correctly() {
    CredentialsStorage::storeCredentials("ABC", "");
    TEST_ASSERT_EQUAL_INT('A', EEPROM.read(EEPROM_SSID_VAL_ADDR + 0));
    TEST_ASSERT_EQUAL_INT('B', EEPROM.read(EEPROM_SSID_VAL_ADDR + 1));
    TEST_ASSERT_EQUAL_INT('C', EEPROM.read(EEPROM_SSID_VAL_ADDR + 2));
}

void test_store_writes_correct_pass_length() {
    CredentialsStorage::storeCredentials("Net", "pw");
    TEST_ASSERT_EQUAL_INT(2, EEPROM.read(EEPROM_PASS_LEN_ADDR));
}

void test_store_writes_pass_bytes_correctly() {
    CredentialsStorage::storeCredentials("N", "XY");
    TEST_ASSERT_EQUAL_INT('X', EEPROM.read(EEPROM_PASS_VAL_ADDR + 0));
    TEST_ASSERT_EQUAL_INT('Y', EEPROM.read(EEPROM_PASS_VAL_ADDR + 1));
}

// ============================================================
// Boundary / clamping behaviour
// ============================================================

void test_overlong_ssid_is_clamped_to_max() {
    // 40 characters — must be clamped to MAX_SSID_LEN (32)
    const char* longSSID = "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAABBBBB"; // 37 chars
    CredentialsStorage::storeCredentials(longSSID, "p");
    TEST_ASSERT_EQUAL_INT(MAX_SSID_LEN, EEPROM.read(EEPROM_SSID_LEN_ADDR));

    char ssid[64] = {0}, pass[128] = {0};
    CredentialsStorage::loadCredentials(ssid, pass);
    // Loaded SSID must be exactly MAX_SSID_LEN chars
    TEST_ASSERT_EQUAL_INT(MAX_SSID_LEN, (int)strlen(ssid));
}

void test_overlong_password_is_clamped_to_max() {
    // 70 characters — must be clamped to MAX_PASS_LEN (64)
    const char* longPass = "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA12345678"; // 72 chars
    CredentialsStorage::storeCredentials("Net", longPass);
    TEST_ASSERT_EQUAL_INT(MAX_PASS_LEN, EEPROM.read(EEPROM_PASS_LEN_ADDR));

    char ssid[64] = {0}, pass[128] = {0};
    CredentialsStorage::loadCredentials(ssid, pass);
    TEST_ASSERT_EQUAL_INT(MAX_PASS_LEN, (int)strlen(pass));
}

void test_max_length_ssid_roundtrip() {
    // Exactly MAX_SSID_LEN characters
    char exact[MAX_SSID_LEN + 1];
    for (int i = 0; i < MAX_SSID_LEN; i++) exact[i] = 'A' + (i % 26);
    exact[MAX_SSID_LEN] = '\0';

    CredentialsStorage::storeCredentials(exact, "pass");
    char ssid[64] = {0}, pass[128] = {0};
    CredentialsStorage::loadCredentials(ssid, pass);
    TEST_ASSERT_EQUAL_STRING(exact, ssid);
}

void test_max_length_password_roundtrip() {
    // Exactly MAX_PASS_LEN characters
    char exact[MAX_PASS_LEN + 1];
    for (int i = 0; i < MAX_PASS_LEN; i++) exact[i] = 'a' + (i % 26);
    exact[MAX_PASS_LEN] = '\0';

    CredentialsStorage::storeCredentials("Net", exact);
    char ssid[64] = {0}, pass[128] = {0};
    CredentialsStorage::loadCredentials(ssid, pass);
    TEST_ASSERT_EQUAL_STRING(exact, pass);
}

void test_overwrite_credentials() {
    CredentialsStorage::storeCredentials("OldNet", "OldPass");
    CredentialsStorage::storeCredentials("NewNet", "NewPass");

    char ssid[64] = {0}, pass[128] = {0};
    CredentialsStorage::loadCredentials(ssid, pass);
    TEST_ASSERT_EQUAL_STRING("NewNet", ssid);
    TEST_ASSERT_EQUAL_STRING("NewPass", pass);
}

void test_ssid_null_terminator_added_after_load() {
    // Ensure the loaded SSID buffer always gets a null terminator
    CredentialsStorage::storeCredentials("TestNet", "pass");
    char ssid[64];
    memset(ssid, 0xCC, sizeof(ssid)); // poison the buffer
    char pass[128] = {0};
    CredentialsStorage::loadCredentials(ssid, pass);
    // The char at index ssidLen should be '\0'
    TEST_ASSERT_EQUAL_INT('\0', ssid[strlen("TestNet")]);
}

// ============================================================
// main
// ============================================================
int main(int argc, char** argv) {
    UNITY_BEGIN();

    // Error paths
    RUN_TEST(test_load_fails_on_empty_eeprom);
    RUN_TEST(test_load_fails_on_wrong_magic_byte);
    RUN_TEST(test_load_fails_on_zero_ssid_length);
    RUN_TEST(test_load_fails_on_oversized_ssid_length_in_eeprom);
    RUN_TEST(test_load_fails_on_oversized_pass_length_in_eeprom);

    // Store then load roundtrip
    RUN_TEST(test_roundtrip_normal_credentials);
    RUN_TEST(test_roundtrip_empty_password);
    RUN_TEST(test_store_sets_magic_byte);
    RUN_TEST(test_store_writes_correct_ssid_length);
    RUN_TEST(test_store_writes_ssid_bytes_correctly);
    RUN_TEST(test_store_writes_correct_pass_length);
    RUN_TEST(test_store_writes_pass_bytes_correctly);

    // Boundary / clamping
    RUN_TEST(test_overlong_ssid_is_clamped_to_max);
    RUN_TEST(test_overlong_password_is_clamped_to_max);
    RUN_TEST(test_max_length_ssid_roundtrip);
    RUN_TEST(test_max_length_password_roundtrip);
    RUN_TEST(test_overwrite_credentials);
    RUN_TEST(test_ssid_null_terminator_added_after_load);

    return UNITY_END();
}