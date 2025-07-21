#include <Arduino.h>
#include <WiFiS3.h>
#include <DHT.h>
#include <Wire.h>
#include <RTClib.h>
#include <Adafruit_PCF8574.h>
#include "SolarWebServer.h"
#include "SystemDisplay.h"
#include "SensorData.h"  // Include the shared header
#include "secrets.h" 
#include "config.h"

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

// Initialize all pins
void initializePins() {
  Serial.println("Initializing pins...");
  
  // Initialize pump pins
  for (int i = 0; i < NUM_WATER_PUMPS; i++) {
    pinMode(PUMP_PINS[i], OUTPUT);
    digitalWrite(PUMP_PINS[i], LOW);
  }
  
  // Initialize valve pins
  for (int i = 0; i < NUM_WATER_VALVES; i++) {
    pinMode(VALVE_PINS[i], OUTPUT);
    digitalWrite(VALVE_PINS[i], HIGH); // Relays normally closed
  }
  
  // Initialize fan pins
  for (int i = 0; i < NUM_FANS; i++) {
    pinMode(FAN_PINS[i], OUTPUT);
    digitalWrite(FAN_PINS[i], LOW);
  }
  
  // Initialize door pins
  for (int i = 0; i < NUM_OF_DOORS * 2; i++) {
    pinMode(DOOR_PINS[i], OUTPUT);
    digitalWrite(DOOR_PINS[i], LOW);
  }
  
  // Initialize trap pins
  pinMode(TRAP_IN1, OUTPUT);
  pinMode(TRAP_IN2, OUTPUT);
  digitalWrite(TRAP_IN1, LOW);
  digitalWrite(TRAP_IN2, LOW);
  
  // Main pump
  pinMode(MAIN_PUMP_PIN, OUTPUT);
  digitalWrite(MAIN_PUMP_PIN, LOW);
  
  Serial.println("Pins initialized successfully");
}

// Initialize PCF8574 I2C expander with retry
void initializePCF() {
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
void initializeRTC() {
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

// Read all sensors
SensorData readSensors() {
  SensorData data;
  
  // Read DHT sensors
  data.tempInt = dhtInt.readTemperature();
  data.humInt = dhtInt.readHumidity();
  data.tempExt = dhtExt.readTemperature();
  data.humExt = dhtExt.readHumidity();
  
  // Read soil moisture sensors
  for (int i = 0; i < NUM_SOIL_SENSORS; i++) {
    data.soilMoisture[i] = analogRead(SOIL_PINS[i]);
  }
  
  // Validate DHT readings
  data.valid = !isnan(data.tempInt) && !isnan(data.humInt) && 
               !isnan(data.tempExt) && !isnan(data.humExt);
  
  return data;
}

// Control individual pump
void controlPump(int pumpIndex, bool state) {
  if (pumpIndex >= 0 && pumpIndex < NUM_WATER_PUMPS) {
    digitalWrite(PUMP_PINS[pumpIndex], state ? HIGH : LOW);
  }
}

// Control individual valve
void controlValve(int valveIndex, bool state) {
  if (valveIndex >= 0 && valveIndex < NUM_WATER_VALVES) {
    digitalWrite(VALVE_PINS[valveIndex], state ? LOW : HIGH); // Inverted for relay
  }
}

// Control individual fan
void controlFan(int fanIndex, bool state) {
  if (fanIndex >= 0 && fanIndex < NUM_FANS) {
    digitalWrite(FAN_PINS[fanIndex], state ? HIGH : LOW);
  }
}

// Irrigation control based on soil moisture
bool controlIrrigation(const SensorData& sensors) {
  bool anyIrrigating = false;
  
  for (int i = 0; i < NUM_SOIL_SENSORS && i < NUM_WATER_VALVES; i++) {
    bool needsWater = sensors.soilMoisture[i] > SOIL_THRESHOLD;
    controlValve(i, needsWater);
    
    if (needsWater) {
      anyIrrigating = true;
      Serial.print("Zone ");
      Serial.print(i + 1);
      Serial.print(" irrigation ON (moisture: ");
      Serial.print(sensors.soilMoisture[i]);
      Serial.println(")");
    }
  }
  
  // Control main pump based on any irrigation activity
  digitalWrite(MAIN_PUMP_PIN, anyIrrigating ? HIGH : LOW);
  
  return anyIrrigating;
}

// Climate control
void controlClimate(const SensorData& sensors) {
  if (!sensors.valid) return;
  
  // Primary fan control based on temperature and humidity
  bool fanNeeded = (sensors.tempInt > FAN_TEMP) || (sensors.humInt > FAN_HUM);
  controlFan(0, fanNeeded);
  
  // Secondary fan for extreme conditions
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

// Open trap
void openTrap() {
  digitalWrite(TRAP_IN1, HIGH);
  digitalWrite(TRAP_IN2, LOW);
  delay(1000); // Run motor for 1 second
  digitalWrite(TRAP_IN1, LOW);
  digitalWrite(TRAP_IN2, LOW);
  Serial.println("Trap opened");
}

// Close trap
void closeTrap() {
  digitalWrite(TRAP_IN1, LOW);
  digitalWrite(TRAP_IN2, HIGH);
  delay(1000); // Run motor for 1 second
  digitalWrite(TRAP_IN1, LOW);
  digitalWrite(TRAP_IN2, LOW);
  Serial.println("Trap closed");
}

// Open specific door
void openDoor(int doorIndex) {
  if (doorIndex >= 0 && doorIndex < NUM_OF_DOORS) {
    int pin1 = DOOR_PINS[doorIndex * 2];
    int pin2 = DOOR_PINS[doorIndex * 2 + 1];
    
    digitalWrite(pin1, HIGH);
    digitalWrite(pin2, LOW);
    delay(2000); // Run for 2 seconds
    digitalWrite(pin1, LOW);
    digitalWrite(pin2, LOW);
    
    Serial.print("Door ");
    Serial.print(doorIndex + 1);
    Serial.println(" opened");
  }
}

// Close specific door
void closeDoor(int doorIndex) {
  if (doorIndex >= 0 && doorIndex < NUM_OF_DOORS) {
    int pin1 = DOOR_PINS[doorIndex * 2];
    int pin2 = DOOR_PINS[doorIndex * 2 + 1];
    
    digitalWrite(pin1, LOW);
    digitalWrite(pin2, HIGH);
    delay(2000); // Run for 2 seconds
    digitalWrite(pin1, LOW);
    digitalWrite(pin2, LOW);
    
    Serial.print("Door ");
    Serial.print(doorIndex + 1);
    Serial.println(" closed");
  }
}

// Initialize WiFi with retry and better error handling
bool initializeWiFi() {
  Serial.print("Initializing WiFi...");
  
  // Check for the WiFi module
  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println("FAILED!");
    Serial.println("ERROR: WiFi module not found!");
    return false;
  }
  
  // Check firmware version
  String fv = WiFi.firmwareVersion();
  Serial.print("WiFi firmware version: ");
  Serial.println(fv);
  
  int attempts = 0;
  const int maxAttempts = 10;
  
  while (attempts < maxAttempts) {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    
    int status = WiFi.begin(ssid, pass);
    
    // Wait for connection with timeout
    unsigned long startTime = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - startTime < 10000) {
      delay(500);
      Serial.print(".");
    }
    
    if (WiFi.status() == WL_CONNECTED) {
      Serial.println();
      Serial.println("WiFi connected successfully!");
      Serial.print("IP address: ");
      Serial.println(WiFi.localIP());
      Serial.print("SSID: ");
      Serial.println(WiFi.SSID());
      return true;
    }
    
    attempts++;
    Serial.println();
    Serial.print("Connection failed, attempt ");
    Serial.print(attempts);
    Serial.print("/");
    Serial.println(maxAttempts);
    
    // Print connection status for debugging
    switch (WiFi.status()) {
      case WL_NO_SSID_AVAIL:
        Serial.println("ERROR: SSID not available");
        break;
      case WL_CONNECT_FAILED:
        Serial.println("ERROR: Connection failed");
        break;
      case WL_CONNECTION_LOST:
        Serial.println("ERROR: Connection lost");
        break;
      case WL_DISCONNECTED:
        Serial.println("ERROR: Disconnected");
        break;
      default:
        Serial.print("ERROR: Unknown status: ");
        Serial.println(WiFi.status());
        break;
    }
    
    delay(2000); // Wait before retry
  }
  
  Serial.println("FAILED!");
  Serial.println("WARNING: WiFi connection failed after all attempts. Continuing without WiFi...");
  return false;
}

void setup() {
  Serial.begin(115200);
  Serial.println("Garden Automation System Starting...");
  
  // Initialize hardware
  initializePins();
  
  // Initialize sensors
  dhtInt.begin();
  dhtExt.begin();
  
  // Initialize I2C
  Wire.begin();
  initializePCF();
  initializeRTC();
  
  // Initialize WiFi and web server - ALWAYS start if WiFi connects
  Serial.println("Starting WiFi...");
  bool wifiConnected = initializeWiFi();
  
  if (wifiConnected) {
    Serial.println("Starting web server...");
    webServer.begin(); // Start web server regardless of sensor status
    Serial.print("Web server ready! Visit: http://");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("Web server not started - WiFi connection failed");
  }
  
  Serial.println("=== Setup Complete ===");
  Serial.println("System ready for operation");
  Serial.println("=====================");
}

void loop() {
  static unsigned long lastSensorRead = 0;
  static SensorData sensors;
  
  // Read sensors every 2 seconds
  if (millis() - lastSensorRead > 2000) {
    sensors = readSensors();
    lastSensorRead = millis();
    
    if (!sensors.valid) {
      Serial.println("WARNING: Invalid sensor readings!");
    }
  }
  
  // Control systems only if sensors are valid
  if (sensors.valid) {
    controlIrrigation(sensors);
    controlClimate(sensors);
  }
  
  // Handle web server ALWAYS if WiFi is connected (even with invalid sensors)
  if (WiFi.status() == WL_CONNECTED) {
    webServer.handleClient(sensors.tempInt, sensors.humInt, 
                          sensors.tempExt, sensors.humExt);
  }
  
  // Display status periodically
  SystemDisplay::displayStatus(sensors, rtc);
  
  delay(100);
}