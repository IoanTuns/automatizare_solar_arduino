#ifndef HARDWARE_INIT_H
#define HARDWARE_INIT_H

#include <Arduino.h>
#include <Wire.h>
#include <RTClib.h>
#include <Adafruit_PCF8574.h>
#include "config.h"

class HardwareInit {
public:
  static void initializePins();
  // Initializes the RTC by configuring it and ensuring it is running; may set the time if not already set.
  static DateTime initializeRTC(RTC_DS3231& rtc, bool setTime = false);
  static bool initializeSD(int chipSelect);
};

#endif // HARDWARE_INIT_H