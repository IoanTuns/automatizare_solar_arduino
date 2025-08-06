#ifndef WEB_AUTHENTICATION_H
#define WEB_AUTHENTICATION_H

#include <Arduino.h>

/**
 * @brief Web authentication class for session management and basic auth
 */
class WebAuthentication {
public:
    // Authentication constants
    static const int MAX_SESSIONS = 5;
    static const int SESSION_ID_LENGTH = 32;
    static const int MAX_FAILED_ATTEMPTS = 5;
    static const unsigned long SESSION_TIMEOUT = 1800000; // 30 minutes in milliseconds
    static const unsigned long LOCKOUT_DURATION = 300000; // 5 minutes in milliseconds
    static const int MAX_USERNAME_LENGTH = 32;
    static const int MAX_PASSWORD_LENGTH = 64;

    /**
     * @brief Session structure for tracking active sessions
     */
    struct Session {
        bool isValid;
        unsigned long lastActivity;
        char sessionId[SESSION_ID_LENGTH + 1];
    };

    /**
     * @brief Initialize authentication with username and password
     * @param username The username for authentication
     * @param password The password for authentication
     */
    static void init(const char* username, const char* password);

    /**
     * @brief Initialize authentication with default credentials from secrets.h
     */
    static void initWithDefaults();

    /**
     * @brief Get current authentication status for debugging
     * @return String with current auth configuration info
     */
    static String getAuthStatus();

    /**
     * @brief Base64 encode utility function
     * @param input String to encode
     * @return Base64 encoded string
     */
    static String base64Encode(const String& input);

    /**
     * @brief Base64 decode utility function
     * @param input Base64 encoded string
     * @return Decoded string
     */
    static String base64Decode(const String& input);

    /**
     * @brief Create a new session after successful authentication
     * @return Session ID string, empty if failed
     */
    static String createSession();

    /**
     * @brief Validate a session ID
     * @param sessionId Session ID to validate
     * @return true if valid, false otherwise
     */
    static bool validateSession(const String& sessionId);

    /**
     * @brief Validate Basic Authentication header
     * @param authHeader Authorization header value
     * @return true if valid, false otherwise
     */
    static bool validateBasicAuth(const String& authHeader);

    /**
     * @brief Validate username and password from a login form
     * @param username The username to validate
     * @param password The password to validate
     * @return true if credentials are valid, false otherwise
     */
    static bool login(const String& username, const String& password);

    /**
     * @brief Generate authentication challenge response
     * @return String containing the HTTP 401 response
     */
    static String generateAuthChallenge();

    /**
     * @brief Check if authentication is currently locked out
     * @return true if locked out, false otherwise
     */
    static bool isLockedOut();

    /**
     * @brief Invalidate a session
     * @param sessionId Session ID to invalidate
     */
    static void invalidateSession(const String& sessionId);

    /**
     * @brief Clear all active sessions
     */
    static void clearAllSessions();

    /**
     * @brief Clean up expired sessions
     */
    static void cleanupExpiredSessions();

    /**
     * @brief Extract session ID from HTTP request
     * @param request HTTP request string
     * @return Session ID if found, empty string otherwise
     */
    static String extractSessionFromRequest(const String& request);

    /**
     * @brief Validate username and password credentials
     * @param username The username to validate
     * @param password The password to validate
     * @return true if credentials are valid, false otherwise
     */
    static bool validateCredentials(const String& username, const String& password);

private:
    static Session _sessions[MAX_SESSIONS];
    static int _failedAttempts;
    static unsigned long _lockoutTime;
    static char _username[MAX_USERNAME_LENGTH + 1];
    static char _password[MAX_PASSWORD_LENGTH + 1];

    static int findSessionSlot();
    static void generateSessionId(char* buffer);
    static String hashPassword(const String& password);
};

#endif // WEB_AUTHENTICATION_H