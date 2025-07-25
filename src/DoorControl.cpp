#include "DoorControl.h"
#include "config.h"

// The constructor initializes the reference to the PCF expander.
DoorControl::DoorControl(Adafruit_PCF8574& pcf) : _pcf(pcf) {}

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
            Serial.println(" opened");
        } else {
            doorStatus[doorIndex] = "error";
            Serial.println(" error");
        }
    } else {
        Serial.println(" status unknown (no sensor)");
    }
}

void DoorControl::close(int doorIndex) {
    if (doorIndex < 0 || doorIndex >= NUM_OF_DOORS) {
        Serial.println("Door index out of range!");
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
    if (hasClosedSensor && isFullyOpen(doorIndex)) {
        Serial.println("Cannot close door while it is open!");
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
            Serial.println(" closed");
        } else {
            doorStatus[doorIndex] = "error";
            Serial.println(" error");
        }
    } else {
        Serial.println(" status unknown (no sensor)");
        // Do not change doorStatus if no sensor
    }
}

void DoorControl::up(int doorIndex) {
    if (doorIndex >= 0 && doorIndex < NUM_OF_DOORS) {
        int pin1 = PCF2_DOOR_PINS[doorIndex * 2];
        int pin2 = PCF2_DOOR_PINS[doorIndex * 2 + 1];
        _pcf.digitalWrite(pin1, LOW);  // Relay ON (active LOW)
        _pcf.digitalWrite(pin2, HIGH); // Relay OFF
        Serial.print("Door ");
        Serial.print(doorIndex + 1);
        Serial.println(" moving up");
    }
}

void DoorControl::down(int doorIndex) {
    if (doorIndex >= 0 && doorIndex < NUM_OF_DOORS) {
        int pin1 = PCF2_DOOR_PINS[doorIndex * 2];
        int pin2 = PCF2_DOOR_PINS[doorIndex * 2 + 1];
        _pcf.digitalWrite(pin1, HIGH); // Relay OFF
        _pcf.digitalWrite(pin2, LOW);  // Relay ON (active LOW)
        Serial.print("Door ");
        Serial.print(doorIndex + 1);
        Serial.println(" moving down");
    }
}

void DoorControl::stop(int doorIndex) {
    if (doorIndex >= 0 && doorIndex < NUM_OF_DOORS) {
        int pin1 = PCF2_DOOR_PINS[doorIndex * 2];
        int pin2 = PCF2_DOOR_PINS[doorIndex * 2 + 1];
        _pcf.digitalWrite(pin1, HIGH); // Relay OFF
        _pcf.digitalWrite(pin2, HIGH); // Relay OFF
        Serial.print("Door ");
        Serial.print(doorIndex + 1);
        Serial.println(" stopped");
    }
}