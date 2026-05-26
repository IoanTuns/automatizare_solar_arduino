#ifndef SECURECREDENTIALS_H
#define SECURECREDENTIALS_H

#include <Arduino.h>

/**
 * @class CredentialsStorage
 * @brief Manages storing and retrieving WiFi credentials from EEPROM.
 * 
 * This class provides a simple, non-encrypted way to persist WiFi credentials
 * across device restarts.
 */
class CredentialsStorage {
public:
    /**
     * @brief Stores WiFi credentials in EEPROM.
     * @param ssid The WiFi network SSID.
     * @param pass The WiFi network password.
     * @return true if storage was successful, false otherwise.
     */
    static bool storeCredentials(const char* ssid, const char* pass);

    /**
     * @brief Loads WiFi credentials from EEPROM.
     * @param ssid Buffer to store the loaded SSID.
     * @param pass Buffer to store the loaded password.
     * @return true if credentials were loaded successfully, false otherwise.
     */
    static bool loadCredentials(char* ssid, char* pass);
};

#endif // SECURECREDENTIALS_H