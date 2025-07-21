#include "SystemDisplay.h"

// Initialize static member
unsigned long SystemDisplay::lastDisplay = 0;

void SystemDisplay::displayStatus(const SensorData& sensors, RTC_DS3231& rtc) {
  if (millis() - lastDisplay < 10000) return; // Display every 10 seconds
  
  Serial.println("=== System Status ===");
  
  // Display WiFi IP address
  if (WiFi.status() == WL_CONNECTED) {
    Serial.print("WiFi IP: ");
    Serial.println(WiFi.localIP());
    Serial.print("WiFi SSID: ");
    Serial.println(WiFi.SSID());
  } else {
    Serial.println("WiFi: Not connected");
  }
  
  if (sensors.valid) {
    Serial.print("Interior: ");
    Serial.print(sensors.tempInt);
    Serial.print("°C, ");
    Serial.print(sensors.humInt);
    Serial.println("%");
    
    Serial.print("Exterior: ");
    Serial.print(sensors.tempExt);
    Serial.print("°C, ");
    Serial.print(sensors.humExt);
    Serial.println("%");
    
    Serial.print("Soil moisture: ");
    for (int i = 0; i < NUM_SOIL_SENSORS; i++) {
      Serial.print(sensors.soilMoisture[i]);
      if (i < NUM_SOIL_SENSORS - 1) Serial.print(", ");
    }
    Serial.println();
  } else {
    Serial.println("Sensor readings invalid!");
  }
  
  // Display RTC time if available
  DateTime now = rtc.now();
  Serial.print("Time: ");
  Serial.print(now.day());
  Serial.print("/");
  Serial.print(now.month());
  Serial.print("/");
  Serial.print(now.year());
  Serial.print(" ");
  Serial.print(now.hour());
  Serial.print(":");
  Serial.print(now.minute());
  Serial.print(":");
  Serial.println(now.second());
  
  Serial.println("====================");
  lastDisplay = millis();
}