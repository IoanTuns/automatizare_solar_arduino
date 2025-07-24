#include <RTClib.h>
#include "HardwareInit.h"
#include <SD.h>

void HardwareInit::initializePins() {
  Serial.println("Initializing pins...");
  Serial.print("NUM_OF_DOORS: ");
  for (int i = 0; i < NUM_OF_DOORS * 2; i++) {
    Serial.print("Setting pin: ");
    pinMode(DOOR_PINS[i], OUTPUT);
    digitalWrite(DOOR_PINS[i], LOW);
  }
  pinMode(TRAP_UP_PIN, OUTPUT);
  pinMode(TRAP_DOWN_PIN, OUTPUT);
  digitalWrite(TRAP_UP_PIN, LOW);
  digitalWrite(TRAP_DOWN_PIN, LOW);
  Serial.println("Pins initialized successfully");
  return; // Pins initialized successfully
}

// Initialize PCF8574 I2C expander with retry
void HardwareInit::initializePCF(Adafruit_PCF8574& pcf) {
  Serial.print("Initializing PCF8574...");
  
  int attempts = 0;
  const int maxAttempts = 5;
  
  while (attempts < maxAttempts) {
    if (pcf.begin(PCF_ADDR)) {
      // Configure all PCF pins as OUTPUT and set HIGH (relays off)
      for (uint8_t i = 0; i < 8; i++) {
        pcf.pinMode(i, OUTPUT);
        pcf.digitalWrite(i, HIGH);
      }
      Serial.println("OK");
      return; // Success, exit function
    }
    
    attempts++;
    Serial.print(".");
    delay(1000); // Wait 1 second before retry
  }
  
  // All attempts failed
  Serial.println("PCF FAILED!");
  Serial.println("WARNING: PCF8574 not found after 5 attempts. Continuing without PCF8574...");
}

// Initialize RTC with retry
DateTime HardwareInit::initializeRTC(RTC_DS3231& rtc, bool setTime) {
  Serial.print("Initializing RTC...");
  int attempts = 0;
  const int maxAttempts = 5;
  while (attempts < maxAttempts) {
    if (rtc.begin()) {
      if (rtc.lostPower() || setTime) {
        Serial.println("RTC lost power or setTime requested, setting time...");
        rtc.adjust(DateTime(__DATE__, __TIME__));
      }
      Serial.println("RTC OK");
      // DateTime rtcTime = rtc.now();
      // Serial.print("RTC Current Time: ");
      // Serial.print(rtcTime.hour());
      // Serial.print(":");
      // Serial.print(rtcTime.minute());
      // Serial.print(":");
      // Serial.println(rtcTime.second());
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