#ifndef CARRIER_PLATFORM_H
#define CARRIER_PLATFORM_H

#include <Arduino_MKRIoTCarrier.h>

void CarrierPlatform_Init();
MKRIoTCarrier* CarrierPlatform_Get();

#endif  // CARRIER_PLATFORM_H
