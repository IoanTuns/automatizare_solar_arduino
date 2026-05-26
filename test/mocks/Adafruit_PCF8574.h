/**
 * @file Adafruit_PCF8574.h
 * @brief Stub for Adafruit PCF8574 I2C I/O expander used in native tests.
 */
#pragma once

#include <cstdint>

class Adafruit_PCF8574 {
public:
    Adafruit_PCF8574() : _pinStates(0xFF) {}

    bool begin(uint8_t /*addr*/ = 0x20, void* /*wire*/ = nullptr) { return true; }

    void pinMode(uint8_t pin, uint8_t /*mode*/) { (void)pin; }

    void digitalWrite(uint8_t pin, uint8_t val) {
        if (val == 0) _pinStates &= ~(1u << pin);
        else           _pinStates |=  (1u << pin);
    }

    uint8_t digitalRead(uint8_t pin) {
        return (_pinStates >> pin) & 1u;
    }

private:
    uint8_t _pinStates;
};