#ifndef HARDWARE_INIT_H
#define HARDWARE_INIT_H

#include <Arduino.h>
#include <Wire.h>
#include <RTClib.h>
#include <Adafruit_PCF8574.h>
#include "config.h"

class HardwareInit {
public:
  // Initializes the PCF8574 I/O expander by setting up communication and configuring initial pin states.
  static void initializePCF(Adafruit_PCF8574& pcf);
  // Initializes the RTC by configuring it and ensuring it is running; may set the time if not already set.
  static void initializeRTC(RTC_DS3231& rtc);
};

#endif // HARDWARE_INIT_H