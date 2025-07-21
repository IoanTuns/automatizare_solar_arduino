#ifndef SYSTEM_DISPLAY_H
#define SYSTEM_DISPLAY_H

#include <Arduino.h>
#include <WiFiS3.h>
#include <RTClib.h>
#include "config.h"
#include "SensorData.h"  // Include the shared header

class SystemDisplay {
public:
  static void displayStatus(const SensorData& sensors, RTC_DS3231& rtc);
  
private:
  static unsigned long lastDisplay;
};

#endif // SYSTEM_DISPLAY_H