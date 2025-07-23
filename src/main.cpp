#include <Arduino.h>
#include <WiFiS3.h>
#include <DHT.h>
#include <Wire.h>
#include <RTClib.h>
#include <Adafruit_PCF8574.h>
#include <SD.h>
#include "SolarWebServer.h"
#include "SystemDisplay.h"
#include "SensorData.h"
#include "HardwareInit.h"
#include "secrets.h"
#include "config.h"
#include "DoorControl.h"
#include "PumpControl.h"
#include "ValveControl.h"

// WiFi credentials
char ssid[] = SECRET_SSID;
char pass[] = SECRET_PASS;

// DHT instances using config definitions
DHT dhtInt(DHTPIN_INT, DHTTYPE);
DHT dhtExt(DHTPIN_EXT, DHTTYPE);

// Web server instance
SolarWebServer webServer(WEB_SERVER_PORT);

// RTC and PCF8574 instances
RTC_DS3231 rtc;
Adafruit_PCF8574 pcf;

// Door control instance
DoorControl doorControl;

// Pump and Valve control instances
PumpControl pumpControl;
ValveControl valveControl;

// Initialize all pins
void initializePins() {
  Serial.println("Initializing pins...");
  for (int i = 0; i < NUM_WATER_PUMPS; i++) {
    pinMode(PUMP_PINS[i], OUTPUT);
    digitalWrite(PUMP_PINS[i], LOW);
  }
  for (int i = 0; i < NUM_WATER_VALVES; i++) {
    pinMode(VALVE_PINS[i], OUTPUT);
    digitalWrite(VALVE_PINS[i], HIGH);
  }
  for (int i = 0; i < NUM_FANS; i++) {
    pinMode(FAN_PINS[i], OUTPUT);
    digitalWrite(FAN_PINS[i], LOW);
  }
  for (int i = 0; i < NUM_OF_DOORS * 2; i++) {
    pinMode(DOOR_PINS[i], OUTPUT);
    digitalWrite(DOOR_PINS[i], LOW);
  }
  pinMode(TRAP_UP_PIN, OUTPUT);
  pinMode(TRAP_DOWN_PIN, OUTPUT);
  digitalWrite(TRAP_UP_PIN, LOW);
  digitalWrite(TRAP_DOWN_PIN, LOW);
  pinMode(MAIN_PUMP_PIN, OUTPUT);
  digitalWrite(MAIN_PUMP_PIN, LOW);
  Serial.println("Pins initialized successfully");
}

void controlFan(int fanIndex, bool state) {
    if (fanIndex >= 0 && fanIndex < NUM_FANS) {
        digitalWrite(FAN_PINS[fanIndex], state ? HIGH : LOW);
    }
}
// Read all sensors
SensorData readSensors() {
  SensorData data;
  data.tempInt = dhtInt.readTemperature();
  data.humInt = dhtInt.readHumidity();
  data.tempExt = dhtExt.readTemperature();
  data.humExt = dhtExt.readHumidity();
  for (int i = 0; i < NUM_SOIL_SENSORS; i++) {
    data.soilMoisture[i] = analogRead(SOIL_PINS[i]);
  }
  data.valid = !isnan(data.tempInt) && !isnan(data.humInt) &&
               !isnan(data.tempExt) && !isnan(data.humExt);
  return data;
}

// Irrigation control based on soil moisture
bool controlIrrigation(const SensorData& sensors) {
    bool anyIrrigating = false;
    for (int i = 0; i < NUM_SOIL_SENSORS && i < NUM_WATER_VALVES; i++) {
        bool needsWater = sensors.soilMoisture[i] > SOIL_THRESHOLD;
        valveControl.set(i, needsWater);
        if (needsWater) {
            anyIrrigating = true;
            Serial.print("Zone ");
            Serial.print(i + 1);
            Serial.print(" irrigation ON (moisture: ");
            Serial.print(sensors.soilMoisture[i]);
            Serial.println(")");
        }
    }
    digitalWrite(MAIN_PUMP_PIN, anyIrrigating ? HIGH : LOW);
    return anyIrrigating;
}

// Climate control
void controlClimate(const SensorData& sensors) {
  if (!sensors.valid) return;
  bool fanNeeded = (sensors.tempInt > FAN_TEMP) || (sensors.humInt > FAN_HUM);
  controlFan(0, fanNeeded);
  bool secondaryFanNeeded = (sensors.tempInt > FAN_TEMP + 5.0) || (sensors.humInt > FAN_HUM + 10.0);
  if (NUM_FANS > 1) {
    controlFan(1, secondaryFanNeeded);
  }
  if (fanNeeded) {
    Serial.print("Climate control: Fan ON (T:");
    Serial.print(sensors.tempInt);
    Serial.print("°C, H:");
    Serial.print(sensors.humInt);
    Serial.println("%)");
  }
}

// SD card chip select pin
const int chipSelect = 10;
bool sdInitialized = false;

// Function to log data to SD card
void logData(const SensorData& data, bool sdInitialized) {
  if (!sdInitialized) {
    Serial.println("SD card not initialized. Skipping data logging.");
    return;
  }
  File dataFile = SD.open("datalog.csv", FILE_WRITE);
  if (dataFile) {
    dataFile.print(millis());
    dataFile.print(",");
    dataFile.print(data.tempInt);
    dataFile.print(",");
    dataFile.print(data.humInt);
    dataFile.print(",");
    dataFile.print(data.tempExt);
    dataFile.print(",");
    dataFile.print(data.humExt);
    for (int i = 0; i < NUM_SOIL_SENSORS; i++) {
      dataFile.print(",");
      dataFile.print(data.soilMoisture[i]);
    }
    dataFile.println();
    dataFile.close();
  } else {
    Serial.println("Error opening datalog.csv");
  }
}

void setup() {
  Serial.begin(115200);
  sdInitialized = SD.begin(chipSelect);
  if (!sdInitialized) {
    Serial.println("SD card initialization failed!");
  } else {
    Serial.println("SD card initialized.");
  }
  webServer.begin(ssid, pass);
  initializePins();
  dhtInt.begin();
  dhtExt.begin();
  Wire.begin();
  HardwareInit::initializePCF(pcf);
  HardwareInit::initializeRTC(rtc);
  Serial.println("=== Setup Complete ===");
  Serial.println("System ready for operation");
  Serial.println("=====================");
}

void loop() {
  static unsigned long lastSensorRead = 0;
  static SensorData sensors;
  if (millis() - lastSensorRead > 2000) {
    sensors = readSensors();
    lastSensorRead = millis();
    if (!sensors.valid) {
      Serial.println("WARNING: Invalid sensor readings!");
    } else {
      logData(sensors, sdInitialized);
    }
  }
  if (sensors.valid) {
    controlIrrigation(sensors);
    controlClimate(sensors);
  }
  if (WiFi.status() == WL_CONNECTED) {
    webServer.handleClient(sensors.tempInt, sensors.humInt,
                          sensors.tempExt, sensors.humExt,
                          sensors.soilMoisture);
  }
  SystemDisplay::displayStatus(sensors, rtc);
  delay(100);
}