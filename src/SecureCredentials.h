#ifndef SECURE_CREDENTIALS_H
#define SECURE_CREDENTIALS_H

#include <Arduino.h>

/**
 * @brief Secure credential storage class for WiFi credentials
 * 
 * This class provides secure storage and retrieval of WiFi credentials
 * using EEPROM with basic encryption and integrity checking.
 */
class SecureCredentials {
public:
    // Constants - defined inline to avoid duplicate definition errors
    static const int EEPROM_SIZE = 512;
    static const uint16_t MAGIC_NUMBER = 0xA5C3;
    static const uint8_t CURRENT_VERSION = 1;
    static const uint8_t MAX_SSID_LENGTH = 32;
    static const uint8_t MAX_PASSWORD_LENGTH = 63;

    /**
     * @brief Credential header structure for EEPROM storage
     */
    struct CredentialHeader {
        uint16_t magic;
        uint8_t version;
        uint8_t ssidLength;
        uint8_t passwordLength;
        uint16_t checksum;
    };

    /**
     * @brief Initialize EEPROM for credential storage
     */
    static void init();

    /**
     * @brief Store WiFi credentials securely in EEPROM
     * @param ssid WiFi SSID (max 32 chars)
     * @param password WiFi password (max 63 chars)
     * @return true if stored successfully, false otherwise
     */
    static bool storeCredentials(const char* ssid, const char* password);

    /**
     * @brief Load WiFi credentials from EEPROM
     * @param ssid Buffer to store SSID (must be at least 33 bytes)
     * @param password Buffer to store password (must be at least 64 bytes)
     * @return true if loaded successfully, false otherwise
     */
    static bool loadCredentials(char* ssid, char* password);

    /**
     * @brief Clear stored credentials from EEPROM
     */
    static void clearCredentials();

    /**
     * @brief Check if valid credentials are stored
     * @return true if valid credentials exist, false otherwise
     */
    static bool hasValidCredentials();
};

#endif // SECURE_CREDENTIALS_H