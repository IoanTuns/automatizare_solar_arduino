/**
 * @file test_main.cpp
 * @brief Unit tests for WebAuthentication — new code added in this PR.
 *
 * Tests cover:
 *  - Session extraction from HTTP requests
 *  - Auth challenge format
 *  - Lockout state logic
 *  - Login credential validation
 *  - Session lifecycle (create, validate, invalidate, clear)
 *  - Cleanup of expired sessions
 *  - Base64 encode / decode roundtrip (exercised via validateBasicAuth)
 */

#include <unity.h>

// Pull in all mocks before any project headers
#include "Arduino.h"
#include "EEPROM.h"
#include "SHA256.h"
#include "secrets.h"
#include "config.h"

// Global mock definitions (compiled once per test binary)
unsigned long _mock_millis = 0;
_SerialStub   Serial;
_WireStub     Wire;
_EEPROMMock   EEPROM;
Adafruit_PCF8574 pcf1, pcf2;

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
bool rtcStatus=false, sdStatus=false, pcf1Status=false, pcf2Status=false, muxStatus=false;
char WEB_USERNAME[MAX_USERNAME_LENGTH + 1] = "admin";
char WEB_PASSWORD[MAX_PASSWORD_LENGTH + 1] = "password";
const PinMapping VALVE_PINS[NUM_WATER_VALVES] = {{0,0},{0,1},{0,2}};
const IrrigationZone IRRIGATION_ZONES[NUM_IRRIGATION_ZONES] = {{0,0,0},{1,1,1},{2,2,2}};
volatile unsigned long flowPulseCount[NUM_WATER_FLOW_METERS] = {0,0,0};
void selectMuxChannel(int) {}
void flowMeterISR0() {} void flowMeterISR1() {} void flowMeterISR2() {}

// Now include the actual source under test.
// We include the .cpp directly so that the static members are compiled in.
#include "../../src/WebAuthentication.h"
#include "../../src/WebAuthentication.cpp"

// ============================================================
// Helper: reset auth state between tests
// ============================================================
static void resetAuth() {
    _mock_millis = 0;
    WebAuthentication::init("admin", "password");
    WebAuthentication::clearAllSessions();
    // Reset lockout by directly re-initialising
    WebAuthentication::init("admin", "password");
}

// ============================================================
// setUp / tearDown called by Unity before/after every test
// ============================================================
void setUp() {
    resetAuth();
}

void tearDown() {}

// ============================================================
// extractSessionFromRequest tests
// ============================================================

void test_extract_session_present() {
    String req = "GET / HTTP/1.1\r\nCookie: sessionId=abcdef1234567890abcdef1234567890\r\n\r\n";
    String id = WebAuthentication::extractSessionFromRequest(req);
    TEST_ASSERT_EQUAL_STRING("abcdef1234567890abcdef1234567890", id.c_str());
}

void test_extract_session_no_cookie_header() {
    String req = "GET / HTTP/1.1\r\nAccept: text/html\r\n\r\n";
    String id = WebAuthentication::extractSessionFromRequest(req);
    TEST_ASSERT_EQUAL_STRING("", id.c_str());
}

void test_extract_session_cookie_header_without_session_id() {
    String req = "GET / HTTP/1.1\r\nCookie: theme=dark\r\n\r\n";
    String id = WebAuthentication::extractSessionFromRequest(req);
    TEST_ASSERT_EQUAL_STRING("", id.c_str());
}

void test_extract_session_multiple_cookies_session_id_last() {
    String req = "GET / HTTP/1.1\r\nCookie: lang=en; sessionId=zzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzz\r\n\r\n";
    String id = WebAuthentication::extractSessionFromRequest(req);
    TEST_ASSERT_EQUAL_STRING("zzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzz", id.c_str());
}

void test_extract_session_terminated_by_semicolon() {
    // Session is followed by another cookie value
    String req = "GET / HTTP/1.1\r\nCookie: sessionId=AAAABBBBCCCCDDDDAAAABBBBCCCCDDDD; other=val\r\n\r\n";
    String id = WebAuthentication::extractSessionFromRequest(req);
    TEST_ASSERT_EQUAL_STRING("AAAABBBBCCCCDDDDAAAABBBBCCCCDDDD", id.c_str());
}

// ============================================================
// generateAuthChallenge tests
// ============================================================

void test_auth_challenge_starts_with_401() {
    String challenge = WebAuthentication::generateAuthChallenge();
    TEST_ASSERT_TRUE(challenge.startsWith("HTTP/1.1 401 Unauthorized"));
}

void test_auth_challenge_contains_www_authenticate() {
    String challenge = WebAuthentication::generateAuthChallenge();
    TEST_ASSERT_TRUE(challenge.indexOf("WWW-Authenticate") != -1);
}

void test_auth_challenge_contains_realm() {
    String challenge = WebAuthentication::generateAuthChallenge();
    TEST_ASSERT_TRUE(challenge.indexOf("realm=") != -1);
}

// ============================================================
// isLockedOut / lockout logic tests
// ============================================================

void test_not_locked_out_initially() {
    TEST_ASSERT_FALSE(WebAuthentication::isLockedOut());
}

void test_lockout_after_max_failed_attempts() {
    // Attempt MAX_FAILED_ATTEMPTS wrong logins to trigger lockout
    for (int i = 0; i < MAX_FAILED_ATTEMPTS; i++) {
        WebAuthentication::login("admin", "wrongpassword");
    }
    TEST_ASSERT_TRUE(WebAuthentication::isLockedOut());
}

void test_lockout_clears_after_duration() {
    for (int i = 0; i < MAX_FAILED_ATTEMPTS; i++) {
        WebAuthentication::login("admin", "wrongpassword");
    }
    TEST_ASSERT_TRUE(WebAuthentication::isLockedOut());

    // Advance fake time past LOCKOUT_DURATION
    _mock_millis = LOCKOUT_DURATION + 1;
    TEST_ASSERT_FALSE(WebAuthentication::isLockedOut());
}

void test_login_while_locked_out_fails() {
    for (int i = 0; i < MAX_FAILED_ATTEMPTS; i++) {
        WebAuthentication::login("admin", "wrongpassword");
    }
    // Even with correct credentials, login should fail during lockout
    bool result = WebAuthentication::login("admin", "password");
    TEST_ASSERT_FALSE(result);
}

// ============================================================
// login() tests
// ============================================================

void test_login_correct_credentials_succeeds() {
    bool result = WebAuthentication::login("admin", "password");
    TEST_ASSERT_TRUE(result);
}

void test_login_wrong_password_fails() {
    bool result = WebAuthentication::login("admin", "wrongpassword");
    TEST_ASSERT_FALSE(result);
}

void test_login_wrong_username_fails() {
    bool result = WebAuthentication::login("hacker", "password");
    TEST_ASSERT_FALSE(result);
}

void test_login_empty_credentials_fail() {
    bool result = WebAuthentication::login("", "");
    TEST_ASSERT_FALSE(result);
}

void test_login_resets_failed_attempts_on_success() {
    // Generate some failures
    WebAuthentication::login("admin", "wrong");
    WebAuthentication::login("admin", "wrong");
    // Successful login resets counter
    WebAuthentication::login("admin", "password");
    // Now 4 more failures should not lock out (counter was reset)
    for (int i = 0; i < MAX_FAILED_ATTEMPTS - 1; i++) {
        WebAuthentication::login("admin", "wrong");
    }
    TEST_ASSERT_FALSE(WebAuthentication::isLockedOut());
}

// ============================================================
// Session lifecycle tests
// ============================================================

void test_create_session_returns_non_empty_string() {
    String id = WebAuthentication::createSession();
    TEST_ASSERT_TRUE(id.length() > 0);
}

void test_create_session_returns_correct_length() {
    String id = WebAuthentication::createSession();
    TEST_ASSERT_EQUAL_INT(SESSION_ID_LENGTH, (int)id.length());
}

void test_validate_session_valid_id_succeeds() {
    String id = WebAuthentication::createSession();
    TEST_ASSERT_TRUE(WebAuthentication::validateSession(id));
}

void test_validate_session_wrong_id_fails() {
    WebAuthentication::createSession(); // create one
    // Build a fake 32-char ID that won't match
    String fakeId = "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX";
    TEST_ASSERT_FALSE(WebAuthentication::validateSession(fakeId));
}

void test_validate_session_wrong_length_fails() {
    // IDs that are not exactly SESSION_ID_LENGTH chars must be rejected
    TEST_ASSERT_FALSE(WebAuthentication::validateSession("tooshort"));
    TEST_ASSERT_FALSE(WebAuthentication::validateSession(""));
}

void test_validate_session_expired_fails() {
    String id = WebAuthentication::createSession();
    // Advance time past SESSION_TIMEOUT
    _mock_millis = SESSION_TIMEOUT + 1;
    TEST_ASSERT_FALSE(WebAuthentication::validateSession(id));
}

void test_validate_session_updates_last_activity() {
    String id = WebAuthentication::createSession();
    // Advance time to just before expiry and validate
    _mock_millis = SESSION_TIMEOUT - 1000;
    TEST_ASSERT_TRUE(WebAuthentication::validateSession(id));
    // Advance another chunk — total would exceed original timeout but last
    // activity was refreshed, so session should still be valid
    _mock_millis = SESSION_TIMEOUT - 1000 + (SESSION_TIMEOUT - 1000);
    TEST_ASSERT_TRUE(WebAuthentication::validateSession(id));
}

void test_invalidate_session_removes_it() {
    String id = WebAuthentication::createSession();
    TEST_ASSERT_TRUE(WebAuthentication::validateSession(id));
    WebAuthentication::invalidateSession(id);
    TEST_ASSERT_FALSE(WebAuthentication::validateSession(id));
}

void test_invalidate_nonexistent_session_no_crash() {
    // Must not crash or corrupt state
    WebAuthentication::invalidateSession("nonexistentidXXXXXXXXXXXXXXXXX");
    // Sessions created after must still work
    String id = WebAuthentication::createSession();
    TEST_ASSERT_TRUE(WebAuthentication::validateSession(id));
}

void test_clear_all_sessions_invalidates_existing() {
    String id1 = WebAuthentication::createSession();
    String id2 = WebAuthentication::createSession();
    WebAuthentication::clearAllSessions();
    TEST_ASSERT_FALSE(WebAuthentication::validateSession(id1));
    TEST_ASSERT_FALSE(WebAuthentication::validateSession(id2));
}

void test_max_sessions_creates_up_to_limit() {
    WebAuthentication::clearAllSessions();
    String ids[MAX_SESSIONS];
    for (int i = 0; i < MAX_SESSIONS; i++) {
        ids[i] = WebAuthentication::createSession();
        TEST_ASSERT_EQUAL_INT(SESSION_ID_LENGTH, (int)ids[i].length());
    }
}

// ============================================================
// cleanupExpiredSessions tests
// ============================================================

void test_cleanup_removes_expired_session() {
    String id = WebAuthentication::createSession();
    _mock_millis = SESSION_TIMEOUT + 1;
    WebAuthentication::cleanupExpiredSessions();
    TEST_ASSERT_FALSE(WebAuthentication::validateSession(id));
}

void test_cleanup_preserves_active_sessions() {
    String id = WebAuthentication::createSession();
    // Advance time but not past timeout
    _mock_millis = SESSION_TIMEOUT / 2;
    WebAuthentication::cleanupExpiredSessions();
    TEST_ASSERT_TRUE(WebAuthentication::validateSession(id));
}

// ============================================================
// getAuthStatus tests
// ============================================================

void test_get_auth_status_contains_username_configured() {
    String status = WebAuthentication::getAuthStatus();
    TEST_ASSERT_TRUE(status.indexOf("Username configured: Yes") != -1);
}

void test_get_auth_status_shows_failed_attempts() {
    WebAuthentication::login("admin", "wrong");
    String status = WebAuthentication::getAuthStatus();
    TEST_ASSERT_TRUE(status.indexOf("Failed attempts: 1") != -1);
}

// ============================================================
// validateBasicAuth — exercises base64Decode internally
// ============================================================

void test_validate_basic_auth_correct_credentials() {
    // "admin:password" base64-encoded = "YWRtaW46cGFzc3dvcmQ="
    bool result = WebAuthentication::validateBasicAuth("Basic YWRtaW46cGFzc3dvcmQ=");
    TEST_ASSERT_TRUE(result);
}

void test_validate_basic_auth_wrong_password() {
    // "admin:wrong" base64-encoded = "YWRtaW46d3Jvbmc="
    bool result = WebAuthentication::validateBasicAuth("Basic YWRtaW46d3Jvbmc=");
    TEST_ASSERT_FALSE(result);
}

void test_validate_basic_auth_missing_basic_prefix_fails() {
    bool result = WebAuthentication::validateBasicAuth("Bearer sometoken");
    TEST_ASSERT_FALSE(result);
}

void test_validate_basic_auth_no_colon_fails() {
    // base64("adminnocodon") — no colon in decoded string
    bool result = WebAuthentication::validateBasicAuth("Basic YWRtaW5ub2NvbG9u");
    TEST_ASSERT_FALSE(result);
}

void test_validate_basic_auth_while_locked_out_fails() {
    for (int i = 0; i < MAX_FAILED_ATTEMPTS; i++) {
        WebAuthentication::login("admin", "wrongpassword");
    }
    // Even correct credentials should fail during lockout
    bool result = WebAuthentication::validateBasicAuth("Basic YWRtaW46cGFzc3dvcmQ=");
    TEST_ASSERT_FALSE(result);
}

// ============================================================
// Edge / regression cases
// ============================================================

void test_create_session_while_locked_out_returns_empty() {
    for (int i = 0; i < MAX_FAILED_ATTEMPTS; i++) {
        WebAuthentication::login("admin", "wrongpassword");
    }
    String id = WebAuthentication::createSession();
    TEST_ASSERT_EQUAL_STRING("", id.c_str());
}

void test_case_sensitive_username() {
    bool result = WebAuthentication::login("Admin", "password"); // capital A
    TEST_ASSERT_FALSE(result);
}

void test_case_sensitive_password() {
    bool result = WebAuthentication::login("admin", "Password"); // capital P
    TEST_ASSERT_FALSE(result);
}

// ============================================================
// main
// ============================================================
int main(int argc, char** argv) {
    UNITY_BEGIN();

    // extractSessionFromRequest
    RUN_TEST(test_extract_session_present);
    RUN_TEST(test_extract_session_no_cookie_header);
    RUN_TEST(test_extract_session_cookie_header_without_session_id);
    RUN_TEST(test_extract_session_multiple_cookies_session_id_last);
    RUN_TEST(test_extract_session_terminated_by_semicolon);

    // generateAuthChallenge
    RUN_TEST(test_auth_challenge_starts_with_401);
    RUN_TEST(test_auth_challenge_contains_www_authenticate);
    RUN_TEST(test_auth_challenge_contains_realm);

    // isLockedOut / lockout
    RUN_TEST(test_not_locked_out_initially);
    RUN_TEST(test_lockout_after_max_failed_attempts);
    RUN_TEST(test_lockout_clears_after_duration);
    RUN_TEST(test_login_while_locked_out_fails);

    // login()
    RUN_TEST(test_login_correct_credentials_succeeds);
    RUN_TEST(test_login_wrong_password_fails);
    RUN_TEST(test_login_wrong_username_fails);
    RUN_TEST(test_login_empty_credentials_fail);
    RUN_TEST(test_login_resets_failed_attempts_on_success);

    // Session lifecycle
    RUN_TEST(test_create_session_returns_non_empty_string);
    RUN_TEST(test_create_session_returns_correct_length);
    RUN_TEST(test_validate_session_valid_id_succeeds);
    RUN_TEST(test_validate_session_wrong_id_fails);
    RUN_TEST(test_validate_session_wrong_length_fails);
    RUN_TEST(test_validate_session_expired_fails);
    RUN_TEST(test_validate_session_updates_last_activity);
    RUN_TEST(test_invalidate_session_removes_it);
    RUN_TEST(test_invalidate_nonexistent_session_no_crash);
    RUN_TEST(test_clear_all_sessions_invalidates_existing);
    RUN_TEST(test_max_sessions_creates_up_to_limit);

    // cleanupExpiredSessions
    RUN_TEST(test_cleanup_removes_expired_session);
    RUN_TEST(test_cleanup_preserves_active_sessions);

    // getAuthStatus
    RUN_TEST(test_get_auth_status_contains_username_configured);
    RUN_TEST(test_get_auth_status_shows_failed_attempts);

    // validateBasicAuth
    RUN_TEST(test_validate_basic_auth_correct_credentials);
    RUN_TEST(test_validate_basic_auth_wrong_password);
    RUN_TEST(test_validate_basic_auth_missing_basic_prefix_fails);
    RUN_TEST(test_validate_basic_auth_no_colon_fails);
    RUN_TEST(test_validate_basic_auth_while_locked_out_fails);

    // Edge / regression
    RUN_TEST(test_create_session_while_locked_out_returns_empty);
    RUN_TEST(test_case_sensitive_username);
    RUN_TEST(test_case_sensitive_password);

    return UNITY_END();
}
