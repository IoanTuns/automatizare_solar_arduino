#ifndef SENSOR_DATA_H
#define SENSOR_DATA_H

#include "config.h"

struct SensorData {
  float tempInt, humInt;
  float tempExt, humExt;
  int soilMoisture[NUM_SOIL_SENSORS];
  float flowRate[NUM_WATER_FLOW_METERS];
  int rainSensorValue;
  bool valid;
  // String soilStatus[NUM_SOIL_SENSORS]; // Add this line to include soilStatus
};

#endif // SENSOR_DATA_H