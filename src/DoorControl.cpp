#include "DoorControl.h"
#include "config.h"

const int MAX_DOOR_ERRORS = 3;

// The constructor initializes the reference to the PCF expander.
DoorControl::DoorControl(Adafruit_PCF8574& pcf) : _pcf(pcf) {
    for (int i = 0; i < NUM_OF_DOORS; ++i) {
        _errorCount[i] = 0;
        _isDisabled[i] = false;
    }
}

bool DoorControl::isFullyOpen(int doorIndex) {
    return digitalRead(DOOR_LIMIT_OPEN_PINS[doorIndex]) == LOW;
}

bool DoorControl::isFullyClosed(int doorIndex) {
    return digitalRead(DOOR_LIMIT_CLOSED_PINS[doorIndex]) == LOW;
}

void DoorControl::open(int doorIndex) {
    if (doorIndex < 0 || doorIndex >= NUM_OF_DOORS) {
        Serial.println("Door index out of range!");
        return;
    }

    if (_isDisabled[doorIndex]) {
        Serial.print("Door ");
        Serial.print(doorIndex + 1);
        Serial.println(" is disabled due to repeated errors.");
        return;
    }

    int openPin = DOOR_LIMIT_OPEN_PINS[doorIndex];
    bool hasOpenSensor = (openPin != -1 && openPin != 255);

    if (hasOpenSensor && isFullyOpen(doorIndex)) {
        doorStatus[doorIndex] = "open";
        Serial.print("Door ");
        Serial.print(doorIndex + 1);
        Serial.println(" already open");
        return;
    }

    int pin1 = PCF2_DOOR_PINS[doorIndex * 2];
    int pin2 = PCF2_DOOR_PINS[doorIndex * 2 + 1];
    _pcf.digitalWrite(pin1, LOW);  // Relay ON (active LOW)
    _pcf.digitalWrite(pin2, HIGH); // Relay OFF

    unsigned long start = millis();
    while ((!hasOpenSensor || !isFullyOpen(doorIndex)) && millis() - start < DOOR_MOVE_TIMEOUT_MS) {
        delay(10);
    }
    _pcf.digitalWrite(pin1, HIGH); // Relay OFF
    _pcf.digitalWrite(pin2, HIGH); // Relay OFF

    Serial.print("Door ");
    Serial.print(doorIndex + 1);
    if (hasOpenSensor) {
        if (isFullyOpen(doorIndex)) {
            doorStatus[doorIndex] = "open";
            _errorCount[doorIndex] = 0; // Reset error count on success
            Serial.println(" opened");
        } else {
            doorStatus[doorIndex] = "error";
            _errorCount[doorIndex]++;
            Serial.print(" error on open. (Count: ");
            Serial.print(_errorCount[doorIndex]);
            Serial.println(")");
            if (_errorCount[doorIndex] >= MAX_DOOR_ERRORS) {
                _isDisabled[doorIndex] = true;
                doorStatus[doorIndex] = "disabled";
                Serial.print("!!! Door ");
                Serial.print(doorIndex + 1);
                Serial.println(" has been disabled after 3 consecutive errors. !!!");
            }
        }
    } else {
        doorStatus[doorIndex] = "unknown";
        Serial.println(" status unknown (no sensor)");
    }
}

void DoorControl::close(int doorIndex) {
    if (doorIndex < 0 || doorIndex >= NUM_OF_DOORS) {
        Serial.println("Door index out of range!");
        return;
    }

    if (_isDisabled[doorIndex]) {
        Serial.print("Door ");
        Serial.print(doorIndex + 1);
        Serial.println(" is disabled due to repeated errors.");
        return;
    }

    // Check if the closed sensor pin is valid (not -1 or 255)
    int closedPin = DOOR_LIMIT_CLOSED_PINS[doorIndex];
    bool hasClosedSensor = (closedPin != -1 && closedPin != 255);

    if (hasClosedSensor && isFullyClosed(doorIndex)) {
        doorStatus[doorIndex] = "closed";
        Serial.print("Door ");
        Serial.print(doorIndex + 1);
        Serial.println(" already closed");
        return;
    }

    int pin1 = PCF2_DOOR_PINS[doorIndex * 2];
    int pin2 = PCF2_DOOR_PINS[doorIndex * 2 + 1];
    _pcf.digitalWrite(pin1, HIGH); // Relay OFF
    _pcf.digitalWrite(pin2, LOW);  // Relay ON (active LOW)

    unsigned long start = millis();
    while ((!hasClosedSensor || !isFullyClosed(doorIndex)) && millis() - start < DOOR_MOVE_TIMEOUT_MS) {
        delay(10);
    }
    delay(DOOR_CLOSE_DELAY_MS);
    _pcf.digitalWrite(pin1, HIGH); // Relay OFF
    _pcf.digitalWrite(pin2, HIGH); // Relay OFF

    Serial.print("Door ");
    Serial.print(doorIndex + 1);
    if (hasClosedSensor) {
        if (isFullyClosed(doorIndex)) {
            doorStatus[doorIndex] = "closed";
            _errorCount[doorIndex] = 0; // Reset error count on success
            Serial.println(" closed");
        } else {
            doorStatus[doorIndex] = "error";
            _errorCount[doorIndex]++;
            Serial.print(" error on close. (Count: ");
            Serial.print(_errorCount[doorIndex]);
            Serial.println(")");
            if (_errorCount[doorIndex] >= MAX_DOOR_ERRORS) {
                _isDisabled[doorIndex] = true;
                doorStatus[doorIndex] = "disabled";
                Serial.print("!!! Door ");
                Serial.print(doorIndex + 1);
                Serial.println(" has been disabled after 3 consecutive errors. !!!");
            }
        }
    } else {
        doorStatus[doorIndex] = "unknown";
        Serial.println(" status unknown (no sensor)");
        // Do not change doorStatus if no sensor
    }
}

void DoorControl::up(int doorIndex) {
    if (doorIndex < 0 || doorIndex >= NUM_OF_DOORS) return;
    if (_isDisabled[doorIndex]) {
        Serial.print("Door ");
        Serial.print(doorIndex + 1);
        Serial.println(" is disabled. Cannot move up.");
        return;
    }

    int pin1 = PCF2_DOOR_PINS[doorIndex * 2];
    int pin2 = PCF2_DOOR_PINS[doorIndex * 2 + 1];
    _pcf.digitalWrite(pin1, LOW);  // Relay ON (active LOW)
    _pcf.digitalWrite(pin2, HIGH); // Relay OFF
    doorStatus[doorIndex] = "moving";
    Serial.print("Door ");
    Serial.print(doorIndex + 1);
    Serial.println(" moving up");
}

void DoorControl::down(int doorIndex) {
    if (doorIndex < 0 || doorIndex >= NUM_OF_DOORS) return;
    if (_isDisabled[doorIndex]) {
        Serial.print("Door ");
        Serial.print(doorIndex + 1);
        Serial.println(" is disabled. Cannot move down.");
        return;
    }

    int pin1 = PCF2_DOOR_PINS[doorIndex * 2];
    int pin2 = PCF2_DOOR_PINS[doorIndex * 2 + 1];
    _pcf.digitalWrite(pin1, HIGH); // Relay OFF
    _pcf.digitalWrite(pin2, LOW);  // Relay ON (active LOW)
    doorStatus[doorIndex] = "moving";
    Serial.print("Door ");
    Serial.print(doorIndex + 1);
    Serial.println(" moving down");
}

void DoorControl::stop(int doorIndex) {
    if (doorIndex < 0 || doorIndex >= NUM_OF_DOORS) return;

    int pin1 = PCF2_DOOR_PINS[doorIndex * 2];
    int pin2 = PCF2_DOOR_PINS[doorIndex * 2 + 1];
    _pcf.digitalWrite(pin1, HIGH); // Relay OFF
    _pcf.digitalWrite(pin2, HIGH); // Relay OFF

    // Only change status if it was actively moving.
    // This prevents overwriting a known state like 'open', 'closed', or 'disabled'.
    if (doorStatus[doorIndex] == "moving") {
        doorStatus[doorIndex] = "stopped";
    }

    Serial.print("Door ");
    Serial.print(doorIndex + 1);
    Serial.println(" stopped");
}

void DoorControl::resetErrors(int doorIndex) {
    if (doorIndex < 0 || doorIndex >= NUM_OF_DOORS) return;

    Serial.print("Resetting error state for Door ");
    Serial.println(doorIndex + 1);

    _errorCount[doorIndex] = 0;
    _isDisabled[doorIndex] = false;

    // After resetting, we don't know the door's true state,
    // so we check the limit switches to set a sensible default.
    if (isFullyClosed(doorIndex)) {
        doorStatus[doorIndex] = "closed";
    } else if (isFullyOpen(doorIndex)) {
        doorStatus[doorIndex] = "open";
    } else {
        doorStatus[doorIndex] = "stopped"; // A neutral state after reset
    }
}