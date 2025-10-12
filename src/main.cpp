#include <Arduino.h>
#include <WiFiS3.h> // For Arduino UNO R4 WiFi
#include <DHT.h>
#include <Wire.h> // For I2C communication
#include <RTClib.h> // For RTC DS3231
#include <Adafruit_PCF8574.h> // For I/O Expander
#include <SD.h> // For SD card module
#include <EEPROM.h> // Required for EEPROM operations
// Include your custom headers
#include "SolarWebServer.h"
#include "SystemDisplay.h"
#include "SecureCredentials.h"
#include "WebAuthentication.h"
#include "LcdDisplay.h"
#include <ArduinoMDNS.h>
#include "HardwareInit.h"
#include "secrets.h"

// LCD instance
LcdDisplay lcd(LCD_I2C_ADDR, LCD_COLS, LCD_ROWS);
WiFiUDP udp;
MDNS mdns(udp);

// --- Global Variables ---
char ssid[33];
char pass[65];

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

void updateLcdDisplay(const SensorData& sensors, const String& rtcTimeString) {
    static unsigned long lastLcdUpdate = 0;
    if (millis() - lastLcdUpdate > DISPLAY_UPDATE_INTERVAL_MS) {
        lcd.clear();
        lcd.printAt(0, 0, rtcTimeString.c_str());

        String tempLine = "In:" + String(sensors.tempInt, 1) + "C Out:" + String(sensors.tempExt, 1) + "C";
        lcd.printAt(0, 1, tempLine.c_str());

        String soilLine = "S1:" + soilStatus[0] + " S2:" + soilStatus[1] + " S3:" + soilStatus[2];
        lcd.printAt(0, 2, soilLine.c_str());

        String statusLine = "Rain:" + rainStatus + " Fans:" + fanStatus[0];
        lcd.printAt(0, 3, statusLine.c_str());
        lastLcdUpdate = millis();
    }
}

void startMDNS() {
  byte mac[6];
  WiFi.macAddress(mac);
  String hostname = "greenhouse-";
  hostname += String(mac[4], HEX);
  hostname += String(mac[5], HEX);

  if (mdns.begin(hostname.c_str())) {
    Serial.println("mDNS responder started: http://" + hostname + ".local");
    mdns.addServiceRecord("http", 80, MDNSServiceTCP);
  } else {
    Serial.println("Error setting up MDNS responder!");
  }
}

// ======================================================================
// setup() function (UPDATED: PCF initialization, Class considerations)
// ======================================================================
void setup() {
    Serial.begin(115200);
    while (!Serial && millis() < 2000); // Wait for serial port to connect
    Serial.println(F("\n--- System Initializing ---"));
    // Initialize LCD
    lcd.init();
    lcd.printAt(0, 0, "Greenhouse Control");
    lcd.printAt(0, 1, "System Starting...");

    // Initialize hardware components
    HardwareInit::initializeHardware();

    // Initialize authentication system with admin credentials
    WebAuthentication::init(DEFAULT_WEB_USERNAME, DEFAULT_WEB_PASSWORD);

    // TEMPORARY: Clear EEPROM for testing/resetting credentials
    Serial.println(F("WARNING: Clearing EEPROM credentials!"));
    for (int i = 0; i < 99; i++) { // Clear up to the end of password storage
        EEPROM.write(i, 0xFF); // Write 0xFF to indicate empty/erased
    }
    Serial.println(F("EEPROM cleared. Please re-upload without this code after first boot."));

    // Attempt to load credentials from EEPROM
    if (!CredentialsStorage::loadCredentials(ssid, pass)) {
        Serial.println(F("INFO: No credentials in EEPROM, using secrets.h"));
        strncpy(ssid, WIFI_SSID, sizeof(ssid) - 1);
        strncpy(pass, WIFI_PASS, sizeof(pass) - 1);
        ssid[sizeof(ssid) - 1] = '\0';
        pass[sizeof(pass) - 1] = '\0';
    } else {
        Serial.println(F("INFO: Loaded credentials from EEPROM."));
    }

    // Initialize Web Server with credentials
    webServer.begin(ssid, pass);

    if (WiFi.status() == WL_CONNECTED) {
        startMDNS();
    }

    Serial.println(F("--- System Ready ---"));
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

    // Web server and mDNS handling
    if (WiFi.status() == WL_CONNECTED) {
        webServer.handleClient(sensors, rtcTimeString);
    }

    // System display update: Pass all relevant sensor data
    SystemDisplay::displayStatus(sensors, rtcStatus, rtcTimeString);
    updateLcdDisplay(sensors, rtcTimeString);

    delay(100); // Small delay to prevent busy-looping
}
