#ifndef SENSOR_DATA_H
#define SENSOR_DATA_H

#include "config.h"

struct SensorData {
  float tempInt, humInt;
  float tempExt, humExt;
  int soilMoisture[NUM_SOIL_SENSORS];
  bool valid;
};

#endif // SENSOR_DATA_H