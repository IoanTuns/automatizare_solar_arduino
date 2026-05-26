/**
 * @file EEPROM.h
 * @brief EEPROM mock backed by a flat byte array for native unit testing.
 */
#pragma once

#include <cstdint>
#include <cstring>

static const int MOCK_EEPROM_SIZE = 512;
static uint8_t _eeprom_data[MOCK_EEPROM_SIZE] = {0};

struct _EEPROMMock {
    uint8_t read(int addr) {
        if (addr < 0 || addr >= MOCK_EEPROM_SIZE) return 0xFF;
        return _eeprom_data[addr];
    }
    void write(int addr, uint8_t val) {
        if (addr < 0 || addr >= MOCK_EEPROM_SIZE) return;
        _eeprom_data[addr] = val;
    }
    /** Reset the entire EEPROM to 0xFF (unprogrammed state). */
    void reset() {
        memset(_eeprom_data, 0xFF, sizeof(_eeprom_data));
    }
};

extern _EEPROMMock EEPROM;