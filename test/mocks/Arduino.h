/**
 * @file Arduino.h
 * @brief Minimal Arduino mock for native unit testing.
 *
 * Provides stubs for Arduino types and functions used in the project
 * so that source files can be compiled and tested on a desktop.
 */
#pragma once

#include <cstdint>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <chrono>
#include <algorithm>

// ---------------------------------------------------------------------------
// Basic Arduino types
// ---------------------------------------------------------------------------
typedef bool boolean;
typedef uint8_t byte;

#ifndef LOW
#define LOW  0
#endif
#ifndef HIGH
#define HIGH 1
#endif
#ifndef INPUT
#define INPUT  0
#endif
#ifndef OUTPUT
#define OUTPUT 1
#endif
#ifndef INPUT_PULLUP
#define INPUT_PULLUP 2
#endif

// ---------------------------------------------------------------------------
// Arduino String class — backed by std::string for desktop tests
// ---------------------------------------------------------------------------
class String {
public:
    String() : _s("") {}
    String(const char* s) : _s(s ? s : "") {}
    String(const std::string& s) : _s(s) {}
    String(int v, int base = 10) {
        char buf[64];
        if (base == 16) snprintf(buf, sizeof(buf), "%x", v);
        else if (base == 8) snprintf(buf, sizeof(buf), "%o", v);
        else snprintf(buf, sizeof(buf), "%d", v);
        _s = buf;
    }
    String(unsigned int v, int base = 10) {
        char buf[64];
        if (base == 16) snprintf(buf, sizeof(buf), "%x", v);
        else snprintf(buf, sizeof(buf), "%u", v);
        _s = buf;
    }
    String(long v, int base = 10) {
        char buf[64];
        if (base == 16) snprintf(buf, sizeof(buf), "%lx", v);
        else snprintf(buf, sizeof(buf), "%ld", v);
        _s = buf;
    }
    String(unsigned long v, int base = 10) {
        char buf[64];
        snprintf(buf, sizeof(buf), "%lu", v);
        _s = buf;
    }
    String(float v, int decimals = 2) {
        char buf[64];
        char fmt[16];
        snprintf(fmt, sizeof(fmt), "%%.%df", decimals);
        snprintf(buf, sizeof(buf), fmt, v);
        _s = buf;
    }
    String(char c) : _s(1, c) {}

    const char* c_str() const { return _s.c_str(); }
    size_t length() const { return _s.length(); }
    bool isEmpty() const { return _s.empty(); }
    bool equals(const String& other) const { return _s == other._s; }
    bool equals(const char* other) const { return _s == (other ? other : ""); }
    bool equalsIgnoreCase(const String& other) const {
        std::string a = _s, b = other._s;
        std::transform(a.begin(), a.end(), a.begin(), ::tolower);
        std::transform(b.begin(), b.end(), b.begin(), ::tolower);
        return a == b;
    }
    bool startsWith(const String& prefix) const {
        return _s.find(prefix._s) == 0;
    }
    bool startsWith(const char* prefix) const {
        return _s.find(prefix) == 0;
    }
    bool endsWith(const String& suffix) const {
        if (_s.length() < suffix._s.length()) return false;
        return _s.rfind(suffix._s) == _s.length() - suffix._s.length();
    }
    int indexOf(const String& sub, int from = 0) const {
        size_t pos = _s.find(sub._s, from);
        return (pos == std::string::npos) ? -1 : (int)pos;
    }
    int indexOf(const char* sub, int from = 0) const {
        if (!sub) return -1;
        size_t pos = _s.find(sub, from);
        return (pos == std::string::npos) ? -1 : (int)pos;
    }
    int indexOf(char c, int from = 0) const {
        size_t pos = _s.find(c, from);
        return (pos == std::string::npos) ? -1 : (int)pos;
    }
    int lastIndexOf(char c) const {
        size_t pos = _s.rfind(c);
        return (pos == std::string::npos) ? -1 : (int)pos;
    }
    String substring(int start) const {
        if (start < 0) start = 0;
        if ((size_t)start >= _s.length()) return String("");
        return String(_s.substr(start));
    }
    String substring(int start, int end) const {
        if (start < 0) start = 0;
        if (end < start) return String("");
        if ((size_t)start >= _s.length()) return String("");
        return String(_s.substr(start, end - start));
    }
    String operator+(const String& other) const { return String(_s + other._s); }
    String operator+(const char* other) const { return String(_s + (other ? other : "")); }
    String operator+(char c) const { return String(_s + c); }
    String& operator+=(const String& other) { _s += other._s; return *this; }
    String& operator+=(const char* other) { if (other) _s += other; return *this; }
    String& operator+=(char c) { _s += c; return *this; }
    bool operator==(const String& other) const { return _s == other._s; }
    bool operator==(const char* other) const { return _s == (other ? other : ""); }
    bool operator!=(const String& other) const { return _s != other._s; }
    bool operator!=(const char* other) const { return _s != (other ? other : ""); }
    char operator[](int i) const { return _s[i]; }
    char charAt(int i) const { return _s[i]; }

    // Iteration (for range-based for loops over chars)
    auto begin() { return _s.begin(); }
    auto end() { return _s.end(); }
    auto begin() const { return _s.begin(); }
    auto end() const { return _s.end(); }

private:
    std::string _s;
};

inline String operator+(const char* lhs, const String& rhs) {
    return String(std::string(lhs ? lhs : "") + rhs.c_str());
}

// ---------------------------------------------------------------------------
// Arduino time functions — controllable in tests
// ---------------------------------------------------------------------------
extern unsigned long _mock_millis;
inline unsigned long millis() { return _mock_millis; }
inline void delay(unsigned long) {}

// ---------------------------------------------------------------------------
// Arduino random
// ---------------------------------------------------------------------------
inline long random(long lo, long hi) {
    if (hi <= lo) return lo;
    return lo + (rand() % (hi - lo));
}
inline long random(long max) { return random(0, max); }

// ---------------------------------------------------------------------------
// Arduino digital I/O stubs
// ---------------------------------------------------------------------------
inline void pinMode(int, int) {}
inline void digitalWrite(int, int) {}
inline int digitalRead(int) { return LOW; }
inline int analogRead(int) { return 0; }
inline void analogWrite(int, int) {}

// ---------------------------------------------------------------------------
// Serial stub
// ---------------------------------------------------------------------------
struct _SerialStub {
    void begin(long) {}
    void print(const char*) {}
    void print(const String& s) {}
    void print(int) {}
    void print(unsigned int) {}
    void print(long) {}
    void print(unsigned long) {}
    void print(float, int = 2) {}
    void print(char) {}
    void println(const char*) {}
    void println(const String&) {}
    void println(int) {}
    void println(unsigned int) {}
    void println(long) {}
    void println(unsigned long) {}
    void println(float, int = 2) {}
    void println(char) {}
    void println() {}
    void flush() {}
};
extern _SerialStub Serial;

// ---------------------------------------------------------------------------
// Wire stub (for I2C)
// ---------------------------------------------------------------------------
struct _WireStub {
    void begin() {}
    void beginTransmission(uint8_t) {}
    uint8_t endTransmission() { return 0; }
    void write(uint8_t) {}
    int requestFrom(uint8_t, uint8_t) { return 0; }
    int available() { return 0; }
    int read() { return -1; }
};
extern _WireStub Wire;

// ---------------------------------------------------------------------------
// PROGMEM stubs (on native, PROGMEM is a no-op)
// ---------------------------------------------------------------------------
#define PROGMEM
#ifndef FPSTR
#define FPSTR(p) (reinterpret_cast<const char*>(p))
#endif

// ---------------------------------------------------------------------------
// HEX / BIN / OCT for String(val, base) is handled by int-constructor above
// ---------------------------------------------------------------------------
#define HEX 16
#define BIN 2
#define OCT 8
#define DEC 10