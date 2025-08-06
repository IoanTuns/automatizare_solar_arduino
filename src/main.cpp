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
#include "SecureCredentials.h" // For secure WiFi credential storage
#include "WebAuthentication.h" // For web authentication
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

// WiFi credentials - now managed through SecureCredentials
char ssid[SecureCredentials::MAX_SSID_LENGTH + 1];
char pass[SecureCredentials::MAX_PASSWORD_LENGTH + 1];

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

  // Initialize all soil moisture values to an error state
  for (int i = 0; i < NUM_SOIL_SENSORS; i++) {
    digitalWrite(MUX_S0_PIN, (i & 1) ? HIGH : LOW);
    digitalWrite(MUX_S1_PIN, (i & 2) ? HIGH : LOW);
    digitalWrite(MUX_S2_PIN, (i & 4) ? HIGH : LOW);
    digitalWrite(MUX_S3_PIN, (i & 8) ? HIGH : LOW);
    delay(10); // Allow multiplexer to settle
    data.soilMoisture[i] = -1; // Default to error state
  }

  // --- Add diagnostic header for MUX reads ---
  Serial.println("--- Reading MUX Channels ---");

  // Read Soil Moisture Sensors via Multiplexer
  for (int i = 0; i < NUM_SOIL_SENSORS; i++) {
    int channel = SOIL_SENSOR1_MUX_CH + i;
    selectMuxChannel(channel); // Assuming channels are sequential from base
    delay(MUX_SETTLE_DELAY_MS); // Small delay for settling

    // --- Dummy read to improve accuracy and prevent crosstalk ---
    analogRead(MUX_SIG_PIN); // First read to charge the ADC's sample-and-hold capacitor
    delay(20); // Further increased delay for ADC to stabilize, especially for high-impedance (disconnected) channels.

    int rawValue = analogRead(MUX_SIG_PIN); // Second, more accurate read

    // --- Add diagnostic print for each channel ---
    Serial.print("  [MUX] Ch ");
    Serial.print(channel);
    Serial.print(" (Soil ");
    Serial.print(i + 1);
    Serial.print("): Raw value = ");
    Serial.println(rawValue);

    // Print valid range for debugging
    Serial.print("    -> Valid range: ");
    Serial.print(SOIL_SENSOR_MIN_VALID);
    Serial.print(" to ");
    Serial.println(SOIL_SENSOR_MAX_VALID);

    // Check for disconnected or invalid sensor
    // A disconnected pin should float outside this range. This check is made more reliable by the hardware pull-down resistor fix.
    if (rawValue < SOIL_SENSOR_MIN_VALID || rawValue > SOIL_SENSOR_MAX_VALID) {
      all_mux_sensors_valid = false;
      Serial.println("    -> Value is outside valid range or disconnected. Marked as invalid.");
    } else {
      data.soilMoisture[i] = rawValue; // Update soil moisture value
    }

    // --- NEW: Reset MUX to a known state (GND) to prevent crosstalk to the NEXT channel ---
    selectMuxChannel(MUX_TEST_GND_CH);
    delay(5); // Increased delay to ensure MUX switches and line settles to GND, helping to discharge the ADC capacitor.
    analogRead(MUX_SIG_PIN); // Dummy read from GND to fully discharge the ADC capacitor.
  }

  // Read Rain Sensor via Multiplexer
  selectMuxChannel(RAIN_SENSOR_MUX_CH);
  delay(MUX_SETTLE_DELAY_MS);
  analogRead(MUX_SIG_PIN); // Dummy read
  delay(10); // Increased delay to match soil sensor reads for consistency
  int rawRainValue = analogRead(MUX_SIG_PIN);

  Serial.print("  [MUX] Ch ");
  Serial.print(RAIN_SENSOR_MUX_CH);
  Serial.print(" (Rain): Raw value = ");
  Serial.println(rawRainValue);

  if (rawRainValue < RAIN_SENSOR_MIN_VALID || rawRainValue > RAIN_SENSOR_MAX_VALID) {
    data.rainSensorValue = -1; // Error value
    all_mux_sensors_valid = false;
    Serial.println("    -> Value is outside valid range. Marked as invalid.");
  } else {
    data.rainSensorValue = rawRainValue;
  }

  // --- NEW: Reset MUX to GND after the final read for good practice ---
  selectMuxChannel(MUX_TEST_GND_CH);
  delay(1);
  analogRead(MUX_SIG_PIN);
  Serial.println("--------------------------");

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

  // Initialize secure credentials system
  SecureCredentials::init();
  
  // Load WiFi credentials from EEPROM
  if (!SecureCredentials::loadCredentials(ssid, pass)) {
    Serial.println("WARNING: No valid WiFi credentials found in EEPROM!");
    Serial.println("Using fallback credentials from secrets.h");
    // Fallback to secrets.h if EEPROM credentials not available
    strncpy(ssid, SECRET_SSID, sizeof(ssid) - 1);
    strncpy(pass, SECRET_PASS, sizeof(pass) - 1);
    ssid[sizeof(ssid) - 1] = '\0';
    pass[sizeof(pass) - 1] = '\0';
  } else {
    Serial.println("WiFi credentials loaded from EEPROM");
  }

  // TODO: Add mDNS support when library is available
  // Note: MDNS functionality commented out until proper library is identified
  
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
    Serial.println("PCF8574_2 OK. Setting initial pin states (Outputs and Inputs)...");
    // Set all pins to a default state first (OUTPUT, HIGH)
    for (int i = 0; i < 8; i++) {
      pcf2.pinMode(i, OUTPUT);
      pcf2.digitalWrite(i, HIGH);
    }
    // Now, specifically set the trap limit switch pins as INPUT_PULLUP
    Serial.println("  - Configuring Trap Limit Switch pins on PCF2 as INPUT_PULLUP.");
    pcf2.pinMode(PCF2_TRAP_LIMIT_OPEN_PIN, INPUT_PULLUP);
    pcf2.pinMode(PCF2_TRAP_LIMIT_CLOSED_PIN, INPUT_PULLUP);
  }

  // Quick MUX test after connecting EN pin
  HardwareInit::quickMuxTest();

  // Initialize RTC and SD card, updating global status flags
  DateTime startTime = HardwareInit::initializeRTC(rtc, true);
  rtcStatus = startTime.isValid();
  sdStatus = HardwareInit::initializeSD(SD_CS_PIN);

  // Validate the multiplexer using the new test function
  muxStatus = HardwareInit::validateMux();

  webServer.begin(ssid, pass);
  
  // Initialize web authentication with default credentials
  WebAuthentication::initWithDefaults();
  
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
    static unsigned long lastMuxCheck = 0;
    static SensorData sensors;

    // Create a formatted date/time string, handling RTC errors
    String rtcTimeString;
    if (rtcStatus) {
        rtcTimeString = getFormattedDateTime(rtc.now());
    } else {
        rtcTimeString = "RTC Error: Not Found";
    }

    // Read all sensors and update the sensor data structure every 2 seconds
    if (millis() - lastSensorRead > 2000) { // Read all sensors every 2 seconds
        sensors = readSensors();
        lastSensorRead = millis();
 
        // Update base status strings from sensor data here.
        // This logic should run for each sensor individually, so it is no longer
        // blocked by the global 'sensors.valid' flag.
        for (int i = 0; i < NUM_SOIL_SENSORS; i++) {
            int rawValue = sensors.soilMoisture[i];
            if (rawValue == -1) {
                soilStatus[i] = "invalid";
            // Logic is now inverted: lower reading means drier soil.
            } else if (rawValue < SOIL_THRESHOLD_DRY) {
                soilStatus[i] = "dry";
            } else if (rawValue < SOIL_THRESHOLD_WET) {
                soilStatus[i] = "normal";
            } else {
                soilStatus[i] = "wet";
            }
        }
        // Update rain status based on the rain sensor value
        if (sensors.rainSensorValue == -1) {
            rainStatus = "invalid";
        } else if (sensors.rainSensorValue < RAIN_THRESHOLD_WET) {
            rainStatus = "raining";
        } else {
            rainStatus = "dry";
        }
 
        if (!sensors.valid) {
            Serial.println("WARNING: One or more sensor readings are invalid!");
        } else {
            logData(sensors);
        }
    }

    // Check MUX every 60 seconds
    if (millis() - lastMuxCheck > 90000) {
        HardwareInit::quickMuxTest();
        lastMuxCheck = millis();
    }

    // Only act on valid sensor data
    if (sensors.valid) {
        irrigationControl.control(sensors);
        climateControl.control(sensors);
    }

    // Web server handling: Pass all relevant sensor data
    if (WiFi.status() == WL_CONNECTED) {
        webServer.handleClient(sensors, rtcTimeString);
    }

    // System display update: Pass all relevant sensor data
    SystemDisplay::displayStatus(sensors, rtcStatus, rtcTimeString);

    delay(100); // Small delay to prevent busy-looping
}