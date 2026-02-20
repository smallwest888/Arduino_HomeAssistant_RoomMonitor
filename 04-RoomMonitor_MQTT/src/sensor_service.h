#ifndef SENSOR_SERVICE_H
#define SENSOR_SERVICE_H

#include "data_model.h"

void SensorService_Init();
bool SensorService_Read(SensorData* out_data);

#endif  // SENSOR_SERVICE_H
