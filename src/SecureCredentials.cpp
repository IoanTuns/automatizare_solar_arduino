#include <EEPROM.h>
#include "SecureCredentials.h"

// NOTE: This implementation stores credentials in plaintext.
// EEPROM layout:
// 0: Magic byte (0xAB)
// 1: SSID length (max 32)
// 2-33: SSID
// 34: Password length (max 64)
// 35-99: Password

#define EEPROM_MAGIC_BYTE_ADDR 0
#define EEPROM_SSID_LEN_ADDR   1
#define EEPROM_SSID_VAL_ADDR   2
#define EEPROM_PASS_LEN_ADDR   34
#define EEPROM_PASS_VAL_ADDR   35

#define MAGIC_BYTE 0xAB
#define MAX_SSID_LEN 32
#define MAX_PASS_LEN 64

bool CredentialsStorage::storeCredentials(const char* ssid, const char* pass) {
    EEPROM.write(EEPROM_MAGIC_BYTE_ADDR, MAGIC_BYTE);

    uint8_t ssidLen = strlen(ssid);
    if (ssidLen > MAX_SSID_LEN) ssidLen = MAX_SSID_LEN;
    EEPROM.write(EEPROM_SSID_LEN_ADDR, ssidLen);
    for (int i = 0; i < ssidLen; i++) {
        EEPROM.write(EEPROM_SSID_VAL_ADDR + i, ssid[i]);
    }

    uint8_t passLen = strlen(pass);
    if (passLen > MAX_PASS_LEN) passLen = MAX_PASS_LEN;
    EEPROM.write(EEPROM_PASS_LEN_ADDR, passLen);
    for (int i = 0; i < passLen; i++) {
        EEPROM.write(EEPROM_PASS_VAL_ADDR + i, pass[i]);
    }

    return true; // On UNO R4, writes are immediate.
}

bool CredentialsStorage::loadCredentials(char* ssid, char* pass) {
    if (EEPROM.read(EEPROM_MAGIC_BYTE_ADDR) != MAGIC_BYTE) {
        return false; // No valid credentials stored
    }

    uint8_t ssidLen = EEPROM.read(EEPROM_SSID_LEN_ADDR);
    if (ssidLen == 0 || ssidLen > MAX_SSID_LEN) {
        return false;
    }
    for (int i = 0; i < ssidLen; i++) {
        ssid[i] = EEPROM.read(EEPROM_SSID_VAL_ADDR + i);
    }
    ssid[ssidLen] = '\0';

    uint8_t passLen = EEPROM.read(EEPROM_PASS_LEN_ADDR);
    if (passLen > MAX_PASS_LEN) { // Password can be empty, but not oversized
        return false;
    }
    for (int i = 0; i < passLen; i++) {
        pass[i] = EEPROM.read(EEPROM_PASS_VAL_ADDR + i);
    }
    pass[passLen] = '\0';

    return true;
}