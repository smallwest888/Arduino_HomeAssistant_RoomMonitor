#ifndef DATA_MODEL_H
#define DATA_MODEL_H

#include <Arduino.h>

struct SensorData {
  float temperature_c;
  float humidity_pct;
  float pressure_hpa;
  uint8_t soil1_pct;
  uint8_t soil2_pct;
};

#endif  // DATA_MODEL_H
