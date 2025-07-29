#include "TrapControl.h"
#include "config.h"

const int MAX_TRAP_ERRORS = 3;

TrapControl::TrapControl(Adafruit_PCF8574& pcf) : _pcf(pcf) {
    _errorCount = 0;
    _isDisabled = false;
    trapStatus = "stopped"; // Initialize status
}

bool TrapControl::isFullyOpen() {
    return digitalRead(TRAP_LIMIT_OPEN_PIN) == LOW;
}

bool TrapControl::isFullyClosed() {
    return digitalRead(TRAP_LIMIT_CLOSED_PIN) == LOW;
}

void TrapControl::open() {
    if (_isDisabled) {
        Serial.println("Trap is disabled due to repeated errors.");
        return;
    }

    if (isFullyOpen()) {
        trapStatus = "open";
        Serial.println("Trap already open");
        return;
    }

    _pcf.digitalWrite(PCF2_TRAP_UP_PIN, LOW);
    _pcf.digitalWrite(PCF2_TRAP_DOWN_PIN, HIGH);

    unsigned long start = millis();
    while (!isFullyOpen() && millis() - start < TRAP_MOVE_TIMEOUT_MS) {
        delay(10);
    }
    _pcf.digitalWrite(PCF2_TRAP_UP_PIN, HIGH);
    _pcf.digitalWrite(PCF2_TRAP_DOWN_PIN, HIGH);

    if (isFullyOpen()) {
        trapStatus = "open";
        _errorCount = 0;
        Serial.println("Trap opened");
    } else {
        trapStatus = "error";
        _errorCount++;
        Serial.print("Trap error on open. (Count: ");
        Serial.print(_errorCount);
        Serial.println(")");
        if (_errorCount >= MAX_TRAP_ERRORS) {
            _isDisabled = true;
            trapStatus = "disabled";
            Serial.println("!!! Trap has been disabled after 3 consecutive errors. !!!");
        }
    }
}

void TrapControl::close() {
    if (_isDisabled) {
        Serial.println("Trap is disabled due to repeated errors.");
        return;
    }

    if (isFullyClosed()) {
        trapStatus = "closed";
        Serial.println("Trap already closed");
        return;
    }

    _pcf.digitalWrite(PCF2_TRAP_UP_PIN, HIGH);
    _pcf.digitalWrite(PCF2_TRAP_DOWN_PIN, LOW);

    unsigned long start = millis();
    while (!isFullyClosed() && millis() - start < TRAP_MOVE_TIMEOUT_MS) {
        delay(10);
    }
    _pcf.digitalWrite(PCF2_TRAP_UP_PIN, HIGH);
    _pcf.digitalWrite(PCF2_TRAP_DOWN_PIN, HIGH);

    if (isFullyClosed()) {
        trapStatus = "closed";
        _errorCount = 0;
        Serial.println("Trap closed");
    } else {
        trapStatus = "error";
        _errorCount++;
        Serial.print("Trap error on close. (Count: ");
        Serial.print(_errorCount);
        Serial.println(")");
        if (_errorCount >= MAX_TRAP_ERRORS) {
            _isDisabled = true;
            trapStatus = "disabled";
            Serial.println("!!! Trap has been disabled after 3 consecutive errors. !!!");
        }
    }
}

void TrapControl::up() {
    if (_isDisabled) {
        Serial.println("Trap is disabled. Cannot move up.");
        return;
    }
    _pcf.digitalWrite(PCF2_TRAP_UP_PIN, LOW);
    _pcf.digitalWrite(PCF2_TRAP_DOWN_PIN, HIGH);
    trapStatus = "moving";
    Serial.println("Trap moving up");
}

void TrapControl::down() {
    if (_isDisabled) {
        Serial.println("Trap is disabled. Cannot move down.");
        return;
    }
    _pcf.digitalWrite(PCF2_TRAP_UP_PIN, HIGH);
    _pcf.digitalWrite(PCF2_TRAP_DOWN_PIN, LOW);
    trapStatus = "moving";
    Serial.println("Trap moving down");
}

void TrapControl::stop() {
    _pcf.digitalWrite(PCF2_TRAP_UP_PIN, HIGH);
    _pcf.digitalWrite(PCF2_TRAP_DOWN_PIN, HIGH);
    if (trapStatus == "moving") {
        trapStatus = "stopped";
    }
    Serial.println("Trap stopped");
}

void TrapControl::resetErrors() {
    Serial.println("Resetting error state for Trap.");
    _errorCount = 0;
    _isDisabled = false;
    if (isFullyClosed()) {
        trapStatus = "closed";
    } else if (isFullyOpen()) {
        trapStatus = "open";
    } else {
        trapStatus = "stopped";
    }
}