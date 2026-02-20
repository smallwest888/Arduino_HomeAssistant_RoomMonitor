#include "carrier_platform.h"

namespace {
MKRIoTCarrier g_carrier;
bool g_is_initialized = false;
}  // namespace

void CarrierPlatform_Init() {
  if (!g_is_initialized) {
    g_carrier.begin();
    g_is_initialized = true;
  }
}

MKRIoTCarrier* CarrierPlatform_Get() {
  return &g_carrier;
}
