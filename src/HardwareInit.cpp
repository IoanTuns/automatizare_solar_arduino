#include "HardwareInit.h"

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
  Serial.println("FAILED!");
  Serial.println("WARNING: PCF8574 not found after 5 attempts. Continuing without PCF8574...");
}

// Initialize RTC with retry
void HardwareInit::initializeRTC(RTC_DS3231& rtc) {
  Serial.print("Initializing RTC...");
  
  int attempts = 0;
  const int maxAttempts = 5;
  
  while (attempts < maxAttempts) {
    if (rtc.begin()) {
      // RTC found, check if time needs to be set
      if (rtc.lostPower()) {
        Serial.println("RTC lost power, setting time...");
        rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
      }
      Serial.println("OK");
      return; // Success, exit function
    }
    
    attempts++;
    Serial.print(".");
    delay(1000); // Wait 1 second before retry
  }
  
  // All attempts failed
  Serial.println("FAILED!");
  Serial.println("WARNING: RTC not found after 5 attempts. Continuing without RTC...");
}