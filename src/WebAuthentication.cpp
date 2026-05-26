#include "WebAuthentication.h"
#include <Arduino.h>
#include "secrets.h"
#include "config.h"
#include <SHA256.h> // Include a library for SHA-256 hashing

// Static member definitions
WebAuthentication::Session WebAuthentication::_sessions[MAX_SESSIONS];
int WebAuthentication::_failedAttempts = 0;
unsigned long WebAuthentication::_lockoutTime = 0;
char WebAuthentication::_username[MAX_USERNAME_LENGTH + 1] = {0};
char WebAuthentication::_password[MAX_PASSWORD_LENGTH + 1] = {0};

/**
 * @brief Initialize authentication system with credentials
 * @param username Admin username
 * @param password Admin password
 */
void WebAuthentication::init(const char* username, const char* password) {
    strncpy(_username, username, MAX_USERNAME_LENGTH);
    _username[MAX_USERNAME_LENGTH] = '\0';

    String hashed = hashPassword(password);
    strncpy(_password, hashed.c_str(), MAX_PASSWORD_LENGTH);
    _password[MAX_PASSWORD_LENGTH] = '\0';
    
    // Initialize sessions
    for (int i = 0; i < MAX_SESSIONS; i++) {
        _sessions[i].isValid = false;
        _sessions[i].lastActivity = 0;
        memset(_sessions[i].sessionId, 0, sizeof(_sessions[i].sessionId));
    }
}

/**
 * @brief Initialize authentication system with default credentials from secrets.h
 */
void WebAuthentication::initWithDefaults() {
    init(DEFAULT_WEB_USERNAME, DEFAULT_WEB_PASSWORD);
    Serial.println("[Auth] Initialized with default credentials");
    Serial.print("[Auth] Default username: ");
    Serial.println(DEFAULT_WEB_USERNAME);
    Serial.println("[Auth] Default password: (hashed and hidden for security)");
}

/**
 * @brief Get current authentication status for debugging
 * @return String with current auth configuration info
 */
String WebAuthentication::getAuthStatus() {
    String status = "Authentication Status:\n";
    status += "Username configured: " + String(_username[0] != '\0' ? "Yes" : "No") + "\n";
    status += "Password configured: " + String(_password[0] != '\0' ? "Yes" : "No") + "\n";
    status += "Active sessions: ";
    
    int activeSessions = 0;
    for (int i = 0; i < MAX_SESSIONS; i++) {
        if (_sessions[i].isValid) {
            activeSessions++;
        }
    }
    status += String(activeSessions) + "/" + String(MAX_SESSIONS) + "\n";
    status += "Failed attempts: " + String(_failedAttempts) + "/" + String(MAX_FAILED_ATTEMPTS) + "\n";
    status += "Locked out: " + String(isLockedOut() ? "Yes" : "No");
    
    return status;
}

/**
 * @brief Create a new session
 * @return Session ID string
 */
String WebAuthentication::createSession() {
    if (isLockedOut()) {
        return "";
    }
    
    int slot = findSessionSlot();
    if (slot < 0) return "";
    
    generateSessionId(_sessions[slot].sessionId);
    _sessions[slot].isValid = true;
    _sessions[slot].lastActivity = millis();
    
    return String(_sessions[slot].sessionId);
}

/**
 * @brief Validate a session ID
 * @param sessionId Session ID to validate
 * @return true if valid, false otherwise
 */
bool WebAuthentication::validateSession(const String& sessionId) {
    if (sessionId.length() != SESSION_ID_LENGTH) return false;
    
    for (int i = 0; i < MAX_SESSIONS; i++) {
        if (_sessions[i].isValid && sessionId.equals(_sessions[i].sessionId)) {
            // Check if session has expired
            if (millis() - _sessions[i].lastActivity > SESSION_TIMEOUT) {
                _sessions[i].isValid = false;
                return false;
            }
            
            // Update last activity
            _sessions[i].lastActivity = millis();
            return true;
        }
    }
    return false;
}

/**
 * @brief Validate Basic Authentication header
 * @param authHeader Authorization header value
 * @return true if valid, false otherwise
 */
bool WebAuthentication::validateBasicAuth(const String& authHeader) {
    if (isLockedOut()) {
        return false;
    }
    
    if (!authHeader.startsWith("Basic ")) {
        _failedAttempts++;
        if (_failedAttempts >= MAX_FAILED_ATTEMPTS) {
            _lockoutTime = millis();
        }
        return false;
    }
    
    String encoded = authHeader.substring(6);
    String decoded = base64Decode(encoded);
    
    int colonIndex = decoded.indexOf(':');
    if (colonIndex < 0) {
        _failedAttempts++;
        if (_failedAttempts >= MAX_FAILED_ATTEMPTS) {
            _lockoutTime = millis();
        }
        return false;
    }
    
    String username = decoded.substring(0, colonIndex);
    String password = decoded.substring(colonIndex + 1);
    
    if (username.equals(_username) && hashPassword(password).equals(_password)) {
        _failedAttempts = 0; // Reset on successful auth
        return true;
    }
    
    _failedAttempts++;
    if (_failedAttempts >= MAX_FAILED_ATTEMPTS) {
        _lockoutTime = millis();
    }
    return false;
}

/**
 * @brief Validate username and password from a login form
 * @param username The username to validate
 * @param password The password to validate
 * @return true if credentials are valid, false otherwise
 */
bool WebAuthentication::login(const String& username, const String& password) {
    if (isLockedOut()) {
        Serial.println("[Auth] System is locked out.");
        return false;
    }

    String hashedPassword = hashPassword(password);
    Serial.print("[Auth] Entered username: ");
    Serial.println(username);
    Serial.print("[Auth] Entered password (hashed): ");
    Serial.println(hashedPassword);
    Serial.print("[Auth] Stored username: ");
    Serial.println(_username);
    Serial.print("[Auth] Stored password (hashed): ");
    Serial.println(_password);

    // Compare provided credentials with stored credentials
    if (username.equals(_username) && hashedPassword.equals(_password)) {
        _failedAttempts = 0; // Reset on successful auth
        return true;
    }

    // Handle failed attempt
    _failedAttempts++;
    if (_failedAttempts >= MAX_FAILED_ATTEMPTS) {
        _lockoutTime = millis();
        Serial.println("[Auth] Too many failed login attempts. Locked out.");
    }
    return false;
}

/**
 * @brief Clean up expired sessions
 */
void WebAuthentication::cleanupExpiredSessions() {
    unsigned long currentTime = millis();
    for (int i = 0; i < MAX_SESSIONS; i++) {
        if (_sessions[i].isValid && (currentTime - _sessions[i].lastActivity > SESSION_TIMEOUT)) {
            _sessions[i].isValid = false;
            memset(_sessions[i].sessionId, 0, sizeof(_sessions[i].sessionId));
        }
    }
}

/**
 * @brief Extract session ID from HTTP request
 * @param request HTTP request string
 * @return Session ID if found, empty string otherwise
 */
String WebAuthentication::extractSessionFromRequest(const String& request) {
    int cookieStart = request.indexOf("Cookie: ");
    if (cookieStart >= 0) {
        int sessionStart = request.indexOf("sessionId=", cookieStart);
        if (sessionStart >= 0) {
            sessionStart += 10; // Length of "sessionId="
            int sessionEnd = request.indexOf(';', sessionStart);
            if (sessionEnd < 0) {
                sessionEnd = request.indexOf("\r", sessionStart);
            }
            if (sessionEnd > sessionStart) {
                return request.substring(sessionStart, sessionEnd);
            }
        }
    }
    return "";
}

/**
 * @brief Generate HTTP authentication challenge
 * @return HTTP 401 response with authentication challenge
 */
String WebAuthentication::generateAuthChallenge() {
    return "HTTP/1.1 401 Unauthorized\r\n"
           "WWW-Authenticate: Basic realm=\"Greenhouse System\"\r\n"
           "Content-Type: text/html\r\n"
           "Connection: close\r\n\r\n"
           "<html><body><h1>401 Unauthorized</h1><p>Authentication required.</p></body></html>";
}

/**
 * @brief Check if authentication is currently locked out
 * @return true if locked out, false otherwise
 */
bool WebAuthentication::isLockedOut() {
    if (_lockoutTime == 0) return false;
    return (millis() - _lockoutTime) < LOCKOUT_DURATION;
}

/**
 * @brief Invalidate a session
 * @param sessionId Session ID to invalidate
 */
void WebAuthentication::invalidateSession(const String& sessionId) {
    for (int i = 0; i < MAX_SESSIONS; i++) {
        if (_sessions[i].isValid && sessionId.equals(_sessions[i].sessionId)) {
            _sessions[i].isValid = false;
            memset(_sessions[i].sessionId, 0, sizeof(_sessions[i].sessionId));
            break;
        }
    }
}

/**
 * @brief Clear all active sessions
 */
void WebAuthentication::clearAllSessions() {
    for (int i = 0; i < MAX_SESSIONS; i++) {
        _sessions[i].isValid = false;
        memset(_sessions[i].sessionId, 0, sizeof(_sessions[i].sessionId));
        _sessions[i].lastActivity = 0;
    }
}

/**
 * @brief Simple hash function for passwords (basic implementation)
 * @param password Password to hash
 * @return Hashed password
 */
String WebAuthentication::hashPassword(const String& password) {
    SHA256 sha256;
    sha256.update(password.c_str(), password.length());
    uint8_t hash[32];
    sha256.finalize(hash, sizeof(hash));
    String hashString = "";
    for (int i = 0; i < 32; i++) { // SHA-256 produces a 32-byte hash
        if (hash[i] < 16) hashString += "0"; // Add leading zero for single digit
        hashString += String(hash[i], HEX);
    }
    return hashString;
}

/**
 * @brief Find an available session slot
 * @return Index of available slot, -1 if none available
 */
int WebAuthentication::findSessionSlot() {
    for (int i = 0; i < MAX_SESSIONS; i++) {
        if (!_sessions[i].isValid || 
            (millis() - _sessions[i].lastActivity > SESSION_TIMEOUT)) {
            return i;
        }
    }
    return -1;
}

/**
 * @brief Generate a random session ID
 * @param buffer Buffer to store session ID (must be SESSION_ID_LENGTH + 1 bytes)
 */
void WebAuthentication::generateSessionId(char* buffer) {
    const char* chars = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    for (int i = 0; i < SESSION_ID_LENGTH; i++) {
        buffer[i] = chars[random(0, strlen(chars))];
    }
    buffer[SESSION_ID_LENGTH] = '\0';
}

/**
 * @brief Simple Base64 encode implementation
 * @param input String to encode
 * @return Base64 encoded string
 */
String WebAuthentication::base64Encode(const String& input) {
    // Simple base64 implementation for basic auth
    const char* chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/=";
    String encoded = "";
    int val = 0, valb = -6;
    
    for (char c : input) {
        val = (val << 8) + c;
        valb += 8;
        while (valb >= 0) {
            encoded += chars[(val >> valb) & 0x3F];
            valb -= 6;
        }
    }
    if (valb > -6) {
        encoded += chars[((val << 8) >> (valb + 8)) & 0x3F];
    }
    while (encoded.length() % 4) {
        encoded += '=';
    }
    return encoded;
}

/**
 * @brief Simple Base64 decode implementation
 * @param input Base64 encoded string
 * @return Decoded string
 */
String WebAuthentication::base64Decode(const String& input) {
    // Simple base64 decode implementation
    String decoded = "";
    int val = 0, valb = -8;
    
    for (char c : input) {
        if (c == '=') break;
        
        int index = -1;
        if (c >= 'A' && c <= 'Z') index = c - 'A';
        else if (c >= 'a' && c <= 'z') index = c - 'a' + 26;
        else if (c >= '0' && c <= '9') index = c - '0' + 52;
        else if (c == '+') index = 62;
        else if (c == '/') index = 63;
        
        if (index == -1) continue;
        
        val = (val << 6) + index;
        valb += 6;
        if (valb >= 0) {
            decoded += char((val >> valb) & 0xFF);
            valb -= 8;
        }
    }
    return decoded;
}

// Example usage:
