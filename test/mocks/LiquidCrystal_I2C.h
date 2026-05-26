/**
 * @file LiquidCrystal_I2C.h
 * @brief Stub for the LiquidCrystal_I2C LCD driver used in native tests.
 */
#pragma once

#include <cstdint>
#include <cstring>

class LiquidCrystal_I2C {
public:
    LiquidCrystal_I2C(uint8_t addr, uint8_t cols, uint8_t rows)
        : _addr(addr), _cols(cols), _rows(rows), _cursor_col(0), _cursor_row(0),
          _backlight(false), _initialized(false) {
        memset(_printBuffer, 0, sizeof(_printBuffer));
    }

    void init()           { _initialized = true; }
    void backlight()      { _backlight = true; }
    void noBacklight()    { _backlight = false; }
    void clear()          { memset(_printBuffer, 0, sizeof(_printBuffer)); _cursor_col = 0; _cursor_row = 0; }
    void setCursor(uint8_t col, uint8_t row) { _cursor_col = col; _cursor_row = row; }
    void print(const char* msg) { strncpy(_printBuffer, msg, sizeof(_printBuffer) - 1); }

    // Accessors for test inspection
    uint8_t getAddr()     const { return _addr; }
    uint8_t getCols()     const { return _cols; }
    uint8_t getRows()     const { return _rows; }
    bool    isInitialized() const { return _initialized; }
    bool    backlightOn() const { return _backlight; }
    uint8_t getCursorCol() const { return _cursor_col; }
    uint8_t getCursorRow() const { return _cursor_row; }
    const char* lastPrint() const { return _printBuffer; }

private:
    uint8_t _addr, _cols, _rows;
    uint8_t _cursor_col, _cursor_row;
    bool    _backlight;
    bool    _initialized;
    char    _printBuffer[128];
};