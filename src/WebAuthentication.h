#ifndef WEBAUTHENTICATION_H
#define WEBAUTHENTICATION_H

#include <Arduino.h>
#include <SHA256.h>
#include "config.h"

#define MAX_SESSIONS 5
#define SESSION_ID_LENGTH 32
#define SESSION_TIMEOUT 600000 // 10 minutes
#define MAX_FAILED_ATTEMPTS 5
#define LOCKOUT_DURATION 300000 // 5 minutes

class WebAuthentication {
public:
    static void init(const char* username, const char* password);
    static void initWithDefaults();
    static String getAuthStatus();
    static String createSession();
    static bool validateSession(const String& sessionId);
    static bool validateBasicAuth(const String& authHeader);
    static bool login(const String& username, const String& password);
    static void cleanupExpiredSessions();
    static String extractSessionFromRequest(const String& request);
    static String generateAuthChallenge();
    static bool isLockedOut();
    static void invalidateSession(const String& sessionId);
    static void clearAllSessions();

private:
    struct Session {
        char sessionId[SESSION_ID_LENGTH + 1];
        unsigned long lastActivity;
        bool isValid;
    };

    static Session _sessions[MAX_SESSIONS];
    static int _failedAttempts;
    static unsigned long _lockoutTime;
    static char _username[MAX_USERNAME_LENGTH + 1];
    static char _password[MAX_PASSWORD_LENGTH + 1];

    static String hashPassword(const String& password);
    static int findSessionSlot();
    static void generateSessionId(char* buffer);
    static String base64Encode(const String& input);
    static String base64Decode(const String& input);
};

#endif // WEBAUTHENTICATION_H