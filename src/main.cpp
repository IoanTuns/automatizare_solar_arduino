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
#include "IrrigationControl.h"
#include "ClimateControl.h"

// WiFi credentials
char ssid[] = SECRET_SSID;
char pass[] = SECRET_PASS;

// DHT instances using config definitions
DHT dhtInt(TEMP_SENSOR_PIN_INT, DHTTYPE);
DHT dhtExt(TEMP_SENSOR_PIN_EXT, DHTTYPE);

// Web server instance
SolarWebServer webServer(WEB_SERVER_PORT);

// RTC and PCF8574 instances
RTC_DS3231 rtc;
DateTime rtcTimeObj;
Adafruit_PCF8574 pcf;

// Door control instance
DoorControl doorControl;

// Pump and Valve control instances
PumpControl pumpControl;
ValveControl valveControl;

IrrigationControl irrigationControl(valveControl);
ClimateControl climateControl;


void controlFan(int fanIndex, bool state) {
    if (fanIndex == 0) pcf.digitalWrite(PCF_FAN1_PIN, state ? LOW : HIGH);
    else if (fanIndex == 1) pcf.digitalWrite(PCF_FAN2_PIN, state ? LOW : HIGH);
}

void selectMuxChannel(int channel) {
  digitalWrite(MUX_S0_PIN, channel & 0x01);
  digitalWrite(MUX_S1_PIN, (channel >> 1) & 0x01);
  digitalWrite(MUX_S2_PIN, (channel >> 2) & 0x01);
  digitalWrite(MUX_S3_PIN, (channel >> 3) & 0x01);
}

// Read all sensors
SensorData readSensors() {
  SensorData data;
  data.tempInt = dhtInt.readTemperature();
  data.humInt = dhtInt.readHumidity();
  data.tempExt = dhtExt.readTemperature();
  data.humExt = dhtExt.readHumidity();
  for (int i = 0; i < NUM_SOIL_SENSORS; i++) {
    selectMuxChannel(i); // Implement this function to set S0-S3
    delay(2); // Small delay for settling
    data.soilMoisture[i] = analogRead(MUX_SIG_PIN);
  }
  data.valid = !isnan(data.tempInt) && !isnan(data.humInt) &&
               !isnan(data.tempExt) && !isnan(data.humExt);
  return data;
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
  delay(100); // Small delay to ensure serial is ready
  
  Serial.println("Starting Solar Irrigation System...");
  dhtInt.begin();
  dhtExt.begin();
  // Initialize I2C communication  
  Wire.begin();

  // Initialize RTC and get current time
  
  // HardwareInit::initializePCF(pcf);
  DateTime rtcTime = HardwareInit::initializeRTC(rtc, true);
  Serial.print("RTC Current Time: ");
  Serial.print(rtcTime.hour());
  Serial.print(":");
  Serial.print(rtcTime.minute());
  Serial.print(":");
  Serial.println(rtcTime.second());

  // HardwareInit::initializePins();
  sdInitialized = HardwareInit::initializeSD(chipSelect);

  webServer.begin(ssid, pass);
  
  Serial.println("=== Setup Complete ===");
  Serial.println("System ready for operation");
  Serial.println("=====================");
}

void loop() {
  static unsigned long lastSensorRead = 0;
  static SensorData sensors;

  // Update RTC time string every loop
  DateTime now = rtc.now();
  String rtcTime = String(now.hour()) + ":" +
                   String(now.minute()) + ":" +
                   String(now.second());

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
    irrigationControl.control(sensors);
    climateControl.control(sensors);
  }
  if (WiFi.status() == WL_CONNECTED) {
    webServer.handleClient(sensors.tempInt, sensors.humInt,
                          sensors.tempExt, sensors.humExt,
                          sensors.soilMoisture, rtcTime);
  }
  SystemDisplay::displayStatus(sensors, rtc);
  delay(100);
}