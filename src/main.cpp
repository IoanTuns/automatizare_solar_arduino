#include <Arduino.h>
#include <WiFiS3.h> // For Arduino UNO R4 WiFi
#include <DHT.h>
#include <Wire.h> // For I2C communication
#include <RTClib.h> // For RTC DS3231
#include <Adafruit_PCF8574.h> // For I/O Expander
#include <SD.h> // For SD card module
// Include your custom headers
#include "SolarWebServer.h"
#include "SystemDisplay.h"
#include "SensorData.h" // This file MUST be updated with new sensor data members
#include "HardwareInit.h"
#include "secrets.h"
#include "config.h" // This file MUST be updated with all new pin definitions
#include "DoorControl.h"    // Needs to be updated to use pcf2
#include "PumpControl.h"    // Needs to be updated to use pcf1
#include "ValveControl.h"   // Needs to be updated to use pcf1
#include "IrrigationControl.h"
#include "ClimateControl.h" // Needs to be updated to use pcf1
#include "TrapControl.h"

// ======================================================================
// GLOBAL VARIABLES & INSTANCES (UPDATED)
// ======================================================================

// WiFi credentials (from secrets.h)
char ssid[] = SECRET_SSID;
char pass[] = SECRET_PASS;

// DHT instances using config definitions
DHT dhtInt(TEMP_SENSOR_PIN_INT, DHTTYPE);
DHT dhtExt(TEMP_SENSOR_PIN_EXT, DHTTYPE);

// RTC instance
RTC_DS3231 rtc; // RTC status is now a global in config.h/cpp

// PCF8574 instances (TWO are now required based on config)
// pcf1 for Valves, Pumps, Fans
// pcf2 for Door and Trap Motors
Adafruit_PCF8574 pcf1;
Adafruit_PCF8574 pcf2;

// Flow meter pulse counters (volatile for ISRs)
volatile unsigned long flowPulseCount[NUM_WATER_FLOW_METERS] = {0, 0, 0};
// Flow rate calculation constants (adjust these based on your specific flow sensors)
// Example for YF-S201: approx. 450 pulses per liter
const float FLOW_CALIBRATION_FACTOR[NUM_WATER_FLOW_METERS] = {450.0, 450.0, 450.0}; // Pulses per liter


// ======================================================================
// ISRs for Water Flow Sensors (NEW)
// These functions are called by interrupts. Keep them short and non-blocking.
// RISING is a common trigger for pulse-based flow sensors.
void flowMeterISR0() {
  flowPulseCount[0]++;
}
void flowMeterISR1() {
  flowPulseCount[1]++;
}
void flowMeterISR2() {
  flowPulseCount[2]++;
}

// ======================================================================
// CLASS INSTANCES (UPDATED with Dependency Injection)
// ======================================================================
// We now create the control objects by passing them the hardware controllers
// they need. This is a robust and modular design.

// Create an array of PCF expander pointers to pass to controllers
Adafruit_PCF8574* pcf_expanders[] = {&pcf1, &pcf2};

DoorControl doorControl(pcf2); // Doors are on pcf2
PumpControl pumpControl(pcf1); // Pumps are on pcf1
ValveControl valveControl(pcf_expanders); // Valves can be on pcf1 or pcf2
ClimateControl climateControl(pcf1); // Fans are on pcf1
TrapControl trapControl(pcf2); // Trap is on pcf2
IrrigationControl irrigationControl(valveControl, pumpControl); // Irrigation needs valves and pumps

// Web server instance (now requires control objects)
SolarWebServer webServer(WEB_SERVER_PORT, pumpControl, doorControl, climateControl, valveControl, trapControl, irrigationControl);


// ======================================================================
// MUX Control Function (No Change in logic, relies on config.h pins)
// ======================================================================
void selectMuxChannel(int channel) {
  digitalWrite(MUX_S0_PIN, channel & 0x01);
  digitalWrite(MUX_S1_PIN, (channel >> 1) & 0x01);
  digitalWrite(MUX_S2_PIN, (channel >> 2) & 0x01);
  digitalWrite(MUX_S3_PIN, (channel >> 3) & 0x01); // MUX_S3_PIN might be optional if using 8-channel MUX
}

// ======================================================================
// Read All Sensors (UPDATED: Flow meters dedicated, Rain sensor multiplexed)
// ======================================================================
SensorData readSensors() {
  SensorData data;
  data.tempInt = dhtInt.readTemperature();
  data.humInt = dhtInt.readHumidity();
  data.tempExt = dhtExt.readTemperature();
  data.humExt = dhtExt.readHumidity();

  bool all_mux_sensors_valid = true;

  // Read Soil Moisture Sensors via Multiplexer
  for (int i = 0; i < NUM_SOIL_SENSORS; i++) {
    selectMuxChannel(SOIL_SENSOR1_MUX_CH + i); // Assuming channels are sequential from base
    delay(MUX_SETTLE_DELAY_MS); // Small delay for settling
    int rawValue = analogRead(MUX_SIG_PIN);
    if (rawValue < SOIL_SENSOR_MIN_VALID || rawValue > SOIL_SENSOR_MAX_VALID) {
      data.soilMoisture[i] = -1; // Use -1 to indicate an error/disconnected state
      all_mux_sensors_valid = false;
    } else {
      data.soilMoisture[i] = rawValue;
    }
  }

  // Read Rain Sensor via Multiplexer
  selectMuxChannel(RAIN_SENSOR_MUX_CH);
  delay(MUX_SETTLE_DELAY_MS);
  int rawRainValue = analogRead(MUX_SIG_PIN); // Assuming analog rain sensor
  if (rawRainValue < RAIN_SENSOR_MIN_VALID || rawRainValue > RAIN_SENSOR_MAX_VALID) {
    data.rainSensorValue = -1; // Error value
    all_mux_sensors_valid = false;
  } else {
    data.rainSensorValue = rawRainValue;
  }

  // Calculate Water Flow Rates from accumulated pulses (NEW)
  // This calculates L/min based on time since last sensor read
  static unsigned long lastFlowCalcTime = 0;
  unsigned long currentTime = millis();
  unsigned long deltaTime = currentTime - lastFlowCalcTime;

  if (deltaTime > 0) { // Avoid division by zero
    for (int i = 0; i < NUM_WATER_FLOW_METERS; i++) {
      // Temporarily disable interrupts for a consistent read and reset
      noInterrupts();
      unsigned long currentPulses = flowPulseCount[i];
      flowPulseCount[i] = 0; // Reset pulse counter
      interrupts();

      // Calculate flow rate in Liters per minute (L/min)
      // (pulses / calibration_factor) gives Liters
      // (Liters / (deltaTime_in_minutes)) gives L/min
      // deltaTime_in_minutes = deltaTime_in_ms / 60000.0
      data.flowRate[i] = (currentPulses / FLOW_CALIBRATION_FACTOR[i]) / (deltaTime / 60000.0);
    }
    lastFlowCalcTime = currentTime;
  } else { // If deltaTime is 0, no new calculation, just ensure flowRate is 0
      for (int i = 0; i < NUM_WATER_FLOW_METERS; i++) {
          data.flowRate[i] = 0.0;
      }
  }

  bool dht_valid = !isnan(data.tempInt) && !isnan(data.humInt) &&
                   !isnan(data.tempExt) && !isnan(data.humExt);

  // Add a sanity check: a reading of exactly 0.0 for both temp and humidity is often a sign of a failed read.
  if ((data.tempInt == 0.0 && data.humInt == 0.0) || (data.tempExt == 0.0 && data.humExt == 0.0)) {
    dht_valid = false;
  }
  
  data.valid = dht_valid && all_mux_sensors_valid;
  // You might add validation for other sensors here if needed (e.g., range checks)
  return data;
}

// Helper function to format date/time parts with a leading zero
String formatTwoDigits(int number) {
  if (number < 10) {
    return "0" + String(number);
  }
  return String(number);
}

String getFormattedDateTime(const DateTime& dt) {
    return String(dt.year()) + "-" + formatTwoDigits(dt.month()) + "-" + formatTwoDigits(dt.day()) +
           " " + formatTwoDigits(dt.hour()) + ":" + formatTwoDigits(dt.minute()) + ":" +
           formatTwoDigits(dt.second());
}

// Function to log data to SD card (UPDATED: Added flow and rain data)
void logData(const SensorData& data) {
  // Use the global sdStatus flag from config.h/cpp
  if (!sdStatus) {
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
    // Add Water Flow Meter Data (NEW)
    for (int i = 0; i < NUM_WATER_FLOW_METERS; i++) {
        dataFile.print(",");
        dataFile.print(data.flowRate[i]);
    }
    // Add Rain Sensor Data (NEW)
    dataFile.print(",");
    dataFile.print(data.rainSensorValue); // Log the analog value from rain sensor

    dataFile.println();
    dataFile.close();
  } else {
    Serial.println("Error opening datalog.csv");
  }
}

// ======================================================================
// setup() function (UPDATED: PCF initialization, Class considerations)
// ======================================================================
void setup() {
  Serial.begin(115200);
  delay(2000); // Small delay to ensure serial is ready

  Serial.println("Starting Solar Irrigation System...");

  dhtInt.begin();
  dhtExt.begin();

  // Initialize I2C bus
  Wire.begin();
  delay(2000); // Small delay to ensure serial is ready

  // --- I2C Scanner for Diagnostics ---
  // This will help identify which I2C devices are responding.
  Serial.println("Scanning I2C bus...");
  i2cScanResults = ""; // Clear previous results for web display
  byte error, address;
  int nDevices = 0;
  for (address = 1; address < 127; address++) {
    Wire.beginTransmission(address);
    error = Wire.endTransmission();
    if (error == 0) {
      String deviceLine = "Found device at 0x";
      if (address < 16) {
        deviceLine += "0";
      }
      deviceLine += String(address, HEX);

      // Add descriptive name for known devices
      deviceLine += " (";
      switch (address) {
        case PCF1_ADDR:
          deviceLine += "PCF8574_1 - Valves/Pumps/Fans";
          break;
        case PCF2_ADDR:
          deviceLine += "PCF8574_2 - Doors/Trap";
          break;
        case RTC_CLOCK_ADDR:
          deviceLine += "RTC Module Clock (DS3231)";
          break;
        case RTC_EEPROM_ADDR:
          deviceLine += "RTC Module EEPROM";
          break;
        default:
          deviceLine += "Unknown Device";
          break;
      }
      deviceLine += ")";

      Serial.println(deviceLine);
      i2cScanResults += deviceLine + "<br>"; // Append for HTML display
      nDevices++;
    } else if (error == 4) {
      String errorLine = "Unknown error at address 0x";
      if (address < 16) {
        errorLine += "0";
      }
      errorLine += String(address, HEX);
      Serial.println(errorLine);
    }
  }
  if (nDevices == 0) {
    String noDeviceMsg = "No I2C devices found. Please check wiring and power.";
    Serial.println(noDeviceMsg);
    i2cScanResults = noDeviceMsg;
  }
  Serial.println("I2C scan complete.\n");

  // Initialize both PCF8574 expanders based on their addresses from config.h
  Serial.print("Initializing PCF8574_1 at 0x"); Serial.println(PCF1_ADDR, HEX);
  pcf1Status = pcf1.begin(PCF1_ADDR);
  if (!pcf1Status) {
    Serial.println("ERROR: PCF8574_1 not found! Check wiring.");
  } else {
    Serial.println("PCF8574_1 OK. Setting initial pin states...");
    // Set all PCF8574 pins to OUTPUT if they control relays, and set an initial safe state.
    // Assuming HIGH means relay OFF (common for active-low relays).
    for (int i = 0; i < 8; i++) {
      pcf1.pinMode(i, OUTPUT);
      pcf1.digitalWrite(i, HIGH);
    }
  }

  Serial.print("Initializing PCF8574_2 at 0x"); Serial.println(PCF2_ADDR, HEX);
  pcf2Status = pcf2.begin(PCF2_ADDR);
  if (!pcf2Status) {
    Serial.println("ERROR: PCF8574_2 not found! Check wiring.");
  } else {
    Serial.println("PCF8574_2 OK. Setting initial pin states...");
    for (int i = 0; i < 8; i++) {
      pcf2.pinMode(i, OUTPUT);
      pcf2.digitalWrite(i, HIGH);
    }
  }

  // Initialize RTC and SD card, updating global status flags
  DateTime startTime = HardwareInit::initializeRTC(rtc, true);
  rtcStatus = startTime.isValid();
  sdStatus = HardwareInit::initializeSD(SD_CS_PIN);

  // Validate the multiplexer using the new test function
  muxStatus = HardwareInit::validateMux();

  // Initialize trap limit switch pins
  pinMode(TRAP_LIMIT_OPEN_PIN, INPUT_PULLUP);
  pinMode(TRAP_LIMIT_CLOSED_PIN, INPUT_PULLUP);

  webServer.begin(ssid, pass);
  HardwareInit::initializePins(); // Call our customized pin initialization

  Serial.println("=== Setup Complete ===");
  Serial.println("System ready for operation");
  Serial.println("=====================");
}

// ======================================================================
// loop() function (UPDATED: Sensor data handling for WebServer/Display)
// ======================================================================
void loop() {
  static unsigned long lastSensorRead = 0;
  static SensorData sensors;

  // Create a formatted date/time string, handling RTC errors
  String rtcTimeString;
  if (rtcStatus) {
    rtcTimeString = getFormattedDateTime(rtc.now());
  } else {
    rtcTimeString = "RTC Error: Not Found";
  }
  if (millis() - lastSensorRead > 2000) { // Read all sensors every 2 seconds
    sensors = readSensors();
    lastSensorRead = millis();
    if (!sensors.valid) {
      Serial.println("WARNING: Invalid DHT sensor readings!");
    } else {
      logData(sensors);
    }
  }

  // Only act on valid sensor data
  if (sensors.valid) {
    irrigationControl.control(sensors);
    climateControl.control(sensors);
    // Note: Door control is event-driven via the web server, not continuous in the loop.
  }

  // Web server handling: Pass all relevant sensor data
  if (WiFi.status() == WL_CONNECTED) {
    // The handleClient function now takes the full sensor data struct
    webServer.handleClient(sensors, rtcTimeString);
  }

  // System display update: Pass all relevant sensor data
  // Pass the RTC status and the pre-formatted time string
  SystemDisplay::displayStatus(sensors, rtcStatus, rtcTimeString);

  delay(100); // Small delay to prevent busy-looping
}