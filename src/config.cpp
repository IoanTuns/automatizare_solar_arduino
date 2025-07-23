#include <Arduino.h>
#include "config.h" // Include the header with function prototypes

String doorStatus[NUM_OF_DOORS] = { "closed", "closed" };
String trapStatus = "stopped";
String pumpStatus[NUM_WATER_PUMPS] = { "off", "off", "off" };
String fanStatus[NUM_FANS] = { "off", "off" };
String valveStatus[NUM_WATER_VALVES] = { "closed", "closed", "closed" };
String soilStatus[NUM_SOIL_SENSORS] = { "normal", "normal", "normal" };
String rainStatus = "dry";
String climateStatus = "normal";

// Trap control loop
// Open trap
void openTrap() {
  digitalWrite(TRAP_UP_PIN, HIGH);
  digitalWrite(TRAP_DOWN_PIN, LOW);
  delay(1000); // Run motor for 1 second
  digitalWrite(TRAP_UP_PIN, LOW);
  digitalWrite(TRAP_DOWN_PIN, LOW);
  Serial.println("Trap opened");
}

// Close trap
void closeTrap() {
  digitalWrite(TRAP_UP_PIN, LOW);
  digitalWrite(TRAP_DOWN_PIN, HIGH);
  delay(1000); // Run motor for 1 second
  digitalWrite(TRAP_UP_PIN, LOW);
  digitalWrite(TRAP_DOWN_PIN, LOW);
  Serial.println("Trap closed");
}
void trapUp() {
    digitalWrite(TRAP_UP_PIN, HIGH);
    digitalWrite(TRAP_DOWN_PIN, LOW);
    // Optionally stop pin logic if needed
}

void trapDown() {
    digitalWrite(TRAP_UP_PIN, LOW);
    digitalWrite(TRAP_DOWN_PIN, HIGH);
    // Optionally stop pin logic if needed
}

void stopTrap() {
    digitalWrite(TRAP_UP_PIN, LOW);
    digitalWrite(TRAP_DOWN_PIN, LOW);
    // Optionally set a STOP pin if your hardware requires it
}