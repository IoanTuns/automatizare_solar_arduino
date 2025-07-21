#include <Arduino.h>
#include <WiFiS3.h>
#include <DHT.h>
#include <Wire.h>
#include <RTClib.h>
#include "SolarWebServer.h"
#include <Adafruit_PCF8574.h>
// Custom secrest config file
#include "secrets.h" 
#include "config.h"

// Constants for array sizes
const int NUM_SOIL_SENSORS = 3;
const int NUM_VALVES = 6;

// Senzori de umiditate sol
const int soilPins[3] = { A0, A1, A2 };

// Liniile de comandă relee pentru cele 3 pompe
const int pumpPins[3] = { 2, 3, 4 };

const int valvePins[6]= { 4, 5, 6, 7, 8, 9 };
const int PUMP_PIN    = 10;
const int FAN_PIN     = 11;

// Praguri
const int   SOIL_THRESHOLD  = 600;
const float FAN_TEMP        = 28.0;
const float FAN_HUM         = 75.0;

///////please enter your sensitive data in the Secret tab/secrets.h
char ssid[] = SECRET_SSID;    // your network SSID (name)
char pass[] = SECRET_PASS;    // your network password (use for WPA, or use as key for WEP)

//// I2C PCF8574 configuration
#define PCF_ADDR 0x20


// ——— PIN-OUT & PRAGURI ———
#define DHTPIN_INT   2
#define DHTPIN_EXT   3
#define DHTTYPE      DHT22

// DHT & server
DHT dhtInt(DHTPIN_INT, DHTTYPE);
DHT dhtExt(DHTPIN_EXT, DHTTYPE);

// web server instance
SolarWebServer webServer;

// RTC instance
RTC_DS3231 rtc;

// PCF(I2C) extender instance
Adafruit_PCF8574 pcf;

// aici definești openTrap(), closeTrap(), openDoor(), closeDoor()
// și inițializezi pinii:
void openTrap()   { /* ... mișcă actuatorul pentru trapă ... */ }
void closeTrap()  { /* ... */ }
void openDoor()   { /* ... */ }
void closeDoor()  { /* ... */ }

/**
 * @brief This function initializes the hardware components and sets up the necessary configurations.
 *
 * This function performs the following tasks:
 * - Initializes the serial communication with a baud rate of 115200.
 * - Sets the pin modes for the pump and fan pins as OUTPUT.
 * - Begins the DHT sensors for internal and external temperature and humidity measurements.
 * - Initializes the I2C communication for the RTC and PCF8574 I2C extender.
 * - Configures the PCF8574 pins as OUTPUT and sets their initial state to HIGH.
 * - Sets the pin modes for the valve pins as OUTPUT and sets their initial state to HIGH.
 * - Sets the pin modes for the PUMP_PIN and FAN_PIN as OUTPUT and sets their initial state to LOW.
 * - Begins the web server with the provided SSID and password.
 * - Initializes the RTC and checks if it lost power. If so, sets the current date and time.
 *
 * @return void
 */
void setup() {
  Serial.begin(115200);
  pinMode(PUMP1_PIN, OUTPUT);
  pinMode(PUMP2_PIN, OUTPUT);
  pinMode(PUMP3_PIN, OUTPUT);
  pinMode(FAN_PIN,   OUTPUT);
  dhtInt.begin();
  dhtExt.begin();
  Wire.begin();       // SDA=D18, SCL=D19 pe UNO R4 WiFi
  pcf.begin(PCF_ADDR);        // inițializează cu adresa PCF_ADDR

  if (!pcf.begin()) {
    Serial.println("Eroare PCF8574");
    while (1);
  }

  // configurează toţi pinii extinși
  for (uint8_t i = 0; i < 8; i++) {
    pcf.pinMode(i, OUTPUT);
    pcf.digitalWrite(i, HIGH);
  }
  pcf.write(); 

  // pini relee zone udare
  for (int i=0; i<6; i++){
    pinMode(valvePins[i], OUTPUT);
    digitalWrite(valvePins[i], HIGH);
  }
  pinMode(PUMP_PIN, OUTPUT); digitalWrite(PUMP_PIN, LOW);
  pinMode(FAN_PIN, OUTPUT);  digitalWrite(FAN_PIN, LOW);

  // WiFi + web
  webServer.begin(ssid, pass);

  // RTC
  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    // while (1);
  }
  if (rtc.lostPower()) {
    Serial.println("RTC lost power, lets set the time!");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
       // Following line sets the RTC with an explicit date & time
    rtc.adjust(DateTime(2025, 1, 01, 01, 0, 0)); // YYYY,MM,DD,hh,mm,ss
  }
}

void loop() {
  // citește senzori
  float tInt = dhtInt.readTemperature();
  float hInt = dhtInt.readHumidity();
  float tExt = dhtExt.readTemperature();
  float hExt = dhtExt.readHumidity();

  // udare + pompă
  bool anyDry = false;
  for (int i=0; i<6; i++){
    int m = analogRead(soilPins[i]);
    digitalWrite(valvePins[i], m>SOIL_THRESHOLD ? LOW : HIGH);
    if (m>SOIL_THRESHOLD) anyDry = true;
  }
  digitalWrite(PUMP_PIN, anyDry);

  // ventilator
  digitalWrite(FAN_PIN, (tInt>FAN_TEMP || hInt>FAN_HUM));

  // servește web
  // Serial.println("call for webServer.handleClient");
  webServer.handleClient(tInt, hInt, tExt, hExt);

  DateTime now = rtc.now();
  // Exemplu de afişare:
  Serial.print("Data/Ora: ");
  Serial.print(now.day());   Serial.print('/');
  Serial.print(now.month()); Serial.print('/');
  Serial.print(now.year());  Serial.print(" ");
  Serial.print(now.hour());  Serial.print(':');
  Serial.print(now.minute());Serial.print(':');
  Serial.println(now.second());

  delay(1000);

}

