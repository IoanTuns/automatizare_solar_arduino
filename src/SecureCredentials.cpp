#include "SecureCredentials.h"
#include <EEPROM.h>

/**
 * @brief Initialize EEPROM for credential storage
 */
void SecureCredentials::init() {
    // Arduino UNO R4 WiFi doesn't need EEPROM.begin() or commit()
    // EEPROM is automatically available
}

/**
 * @brief Store WiFi credentials securely in EEPROM
 * @param ssid WiFi SSID (max 32 chars)
 * @param password WiFi password (max 63 chars)
 * @return true if stored successfully, false otherwise
 */
bool SecureCredentials::storeCredentials(const char* ssid, const char* password) {
    if (!ssid || !password) return false;
    
    // Validate input lengths
    if (strlen(ssid) > MAX_SSID_LENGTH || strlen(password) > MAX_PASSWORD_LENGTH) {
        return false;
    }
    
    CredentialHeader header;
    header.magic = MAGIC_NUMBER;
    header.version = CURRENT_VERSION;
    header.ssidLength = strlen(ssid);
    header.passwordLength = strlen(password);
    
    // Calculate simple checksum
    header.checksum = 0;
    for (size_t i = 0; i < strlen(ssid); i++) {
        header.checksum += ssid[i];
    }
    for (size_t i = 0; i < strlen(password); i++) {
        header.checksum += password[i];
    }
    
    // Write header
    uint16_t addr = 0;
    EEPROM.put(addr, header);
    addr += sizeof(CredentialHeader);
    
    // Write SSID
    for (size_t i = 0; i < header.ssidLength; i++) {
        EEPROM.write(addr++, ssid[i]);
    }
    
    // Write password
    for (size_t i = 0; i < header.passwordLength; i++) {
        EEPROM.write(addr++, password[i]);
    }
    
    return true;
}

/**
 * @brief Load WiFi credentials from EEPROM
 * @param ssid Buffer to store SSID (must be at least 33 bytes)
 * @param password Buffer to store password (must be at least 64 bytes)
 * @return true if loaded successfully, false otherwise
 */
bool SecureCredentials::loadCredentials(char* ssid, char* password) {
    if (!ssid || !password) return false;
    
    CredentialHeader header;
    uint16_t addr = 0;
    
    // Read header
    EEPROM.get(addr, header);
    addr += sizeof(CredentialHeader);
    
    // Validate header
    if (header.magic != MAGIC_NUMBER || header.version != CURRENT_VERSION) {
        return false;
    }
    
    // Validate lengths
    if (header.ssidLength > MAX_SSID_LENGTH || header.passwordLength > MAX_PASSWORD_LENGTH) {
        return false;
    }
    
    // Read SSID
    for (size_t i = 0; i < header.ssidLength; i++) {
        ssid[i] = EEPROM.read(addr++);
    }
    ssid[header.ssidLength] = '\0';
    
    // Read password
    for (size_t i = 0; i < header.passwordLength; i++) {
        password[i] = EEPROM.read(addr++);
    }
    password[header.passwordLength] = '\0';
    
    // Verify checksum
    uint16_t calculatedChecksum = 0;
    for (size_t i = 0; i < header.ssidLength; i++) {
        calculatedChecksum += ssid[i];
    }
    for (size_t i = 0; i < header.passwordLength; i++) {
        calculatedChecksum += password[i];
    }
    
    if (calculatedChecksum != header.checksum) {
        // Clear buffers on checksum failure
        memset(ssid, 0, MAX_SSID_LENGTH + 1);
        memset(password, 0, MAX_PASSWORD_LENGTH + 1);
        return false;
    }
    
    return true;
}

/**
 * @brief Clear stored credentials from EEPROM
 */
void SecureCredentials::clearCredentials() {
    // Overwrite the magic number to invalidate stored data
    uint16_t invalidMagic = 0x0000;
    EEPROM.put(0, invalidMagic);
    
    // Optionally clear more data for security
    for (uint16_t i = 0; i < sizeof(CredentialHeader) + MAX_SSID_LENGTH + MAX_PASSWORD_LENGTH; i++) {
        EEPROM.write(i, 0x00);
    }
}

/**
 * @brief Check if valid credentials are stored
 * @return true if valid credentials exist, false otherwise
 */
bool SecureCredentials::hasValidCredentials() {
    CredentialHeader header;
    EEPROM.get(0, header);
    
    return (header.magic == MAGIC_NUMBER && header.version == CURRENT_VERSION);
}