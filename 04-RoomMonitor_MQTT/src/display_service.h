#ifndef DISPLAY_SERVICE_H
#define DISPLAY_SERVICE_H

#include "data_model.h"

void DisplayService_Init();
void DisplayService_ShowBootText();
void DisplayService_ShowData(const SensorData* data);

#endif  // DISPLAY_SERVICE_H
