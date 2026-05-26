/**
 * @file DHT.h
 * @brief Stub for DHT temperature/humidity sensor used in native tests.
 */
#pragma once

#include <cstdint>

#define DHT22 22
#define DHT11 11

class DHT {
public:
    DHT(uint8_t /*pin*/, uint8_t /*type*/) {}
    void begin() {}
    float readTemperature(bool /*fahrenheit*/ = false) { return 25.0f; }
    float readHumidity() { return 60.0f; }
    bool isnan(float v) { return v != v; }
};