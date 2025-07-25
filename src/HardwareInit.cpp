#include <RTClib.h>
#include "HardwareInit.h"
#include <SD.h>

void HardwareInit::initializePins() {
  Serial.println("Setting door limit switch pins...");
  for (int i = 0; i < NUM_OF_DOORS; i++) {
    Serial.print("Door "); Serial.print(i+1); Serial.print(" open pin: "); Serial.println(DOOR_LIMIT_OPEN_PINS[i]);
    pinMode(DOOR_LIMIT_OPEN_PINS[i], INPUT_PULLUP);
    Serial.print("Door "); Serial.print(i+1); Serial.print(" closed pin: "); Serial.println(DOOR_LIMIT_CLOSED_PINS[i]);
    pinMode(DOOR_LIMIT_CLOSED_PINS[i], INPUT_PULLUP);
  }

  Serial.println("Setting multiplexer select pins...");
  pinMode(MUX_S0_PIN, OUTPUT);
  pinMode(MUX_S1_PIN, OUTPUT);
  pinMode(MUX_S2_PIN, OUTPUT);
  pinMode(MUX_S3_PIN, OUTPUT); // Only necessary if using a 16-channel MUX or for future expansion

  Serial.println("Setting water flow sensor pins and attaching interrupts...");
  // Attach interrupts for each flow meter pin.
  // Ensure the ISRs (flowMeterISR0, flowMeterISR1, flowMeterISR2) are defined globally.
  // RISING is generally suitable for pulse sensors, but check your sensor's datasheet.
  pinMode(WATER_FLOW_METER_PINS[0], INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(WATER_FLOW_METER_PINS[0]), flowMeterISR0, RISING);
  Serial.print("Flow Meter 1 on pin "); Serial.println(WATER_FLOW_METER_PINS[0]);

  pinMode(WATER_FLOW_METER_PINS[1], INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(WATER_FLOW_METER_PINS[1]), flowMeterISR1, RISING);
  Serial.print("Flow Meter 2 on pin "); Serial.println(WATER_FLOW_METER_PINS[1]);

  pinMode(WATER_FLOW_METER_PINS[2], INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(WATER_FLOW_METER_PINS[2]), flowMeterISR2, RISING);
  Serial.print("Flow Meter 3 on pin "); Serial.println(WATER_FLOW_METER_PINS[2]);

  Serial.println("Pins initialized successfully");
}

// Initialize RTC with retry
DateTime HardwareInit::initializeRTC(RTC_DS3231& rtc, bool setTime) {
  Serial.print("Initializing RTC...");

  // Pre-check to see if a device is actually present at the RTC's I2C address
  Wire.beginTransmission(RTC_ADDR);
  if (Wire.endTransmission() != 0) {
    Serial.println("\nERROR: No device responded at the RTC address (0x" + String(RTC_ADDR, HEX) + ").");
    Serial.println("The device at 0x51 is likely the EEPROM on the module, not the clock chip.");
    Serial.println("Please check RTC module wiring and power.");
    return DateTime((uint32_t)0); // Return invalid time immediately
  }

  int attempts = 0;
  const int maxAttempts = 5;
  while (attempts < maxAttempts) {
    if (rtc.begin()) {
      if (rtc.lostPower() || setTime) {
        Serial.println("RTC lost power or setTime requested, setting time...");
        rtc.adjust(DateTime(__DATE__, __TIME__));
      }
      Serial.println("RTC OK");
      return rtc.now();
    }
    attempts++;
    Serial.print(".");
    delay(1000);
  }
  Serial.println("RTC FAILED!");
  Serial.println("WARNING: RTC not found after 5 attempts. Continuing without RTC...");
  return DateTime((uint32_t)0);
}

bool HardwareInit::initializeSD(int chipSelect) {
  Serial.print("Initializing SD card...");
  bool sdInitialized = SD.begin(chipSelect);
  if (!sdInitialized) {
    Serial.println("SD card initialization failed!");
  } else {
    Serial.println("SD card initialized.");
  }
  return sdInitialized;
}