#include "sensor_service.h"

#include "carrier_platform.h"
#include "config.h"

namespace {
uint8_t SoilRawToPercent(const int raw_value) {
  const long mapped = map(
      raw_value,
      RoomMonitorConfig::SOIL_ADC_MIN,
      RoomMonitorConfig::SOIL_ADC_MAX,
      RoomMonitorConfig::SOIL_PERCENT_MAX,
      RoomMonitorConfig::SOIL_PERCENT_MIN);

  if (mapped < RoomMonitorConfig::SOIL_PERCENT_MIN) {
    return static_cast<uint8_t>(RoomMonitorConfig::SOIL_PERCENT_MIN);
  }
  if (mapped > RoomMonitorConfig::SOIL_PERCENT_MAX) {
    return static_cast<uint8_t>(RoomMonitorConfig::SOIL_PERCENT_MAX);
  }
  return static_cast<uint8_t>(mapped);
}
}  // namespace

void SensorService_Init() {
  CarrierPlatform_Init();
}

bool SensorService_Read(SensorData* out_data) {
  if (out_data == nullptr) {
    return false;
  }

  MKRIoTCarrier* carrier = CarrierPlatform_Get();
  if (carrier == nullptr) {
    return false;
  }

  const float pressure_kpa = carrier->Pressure.readPressure();

  out_data->temperature_c = carrier->Env.readTemperature();
  out_data->humidity_pct = carrier->Env.readHumidity();
  out_data->pressure_hpa = pressure_kpa * RoomMonitorConfig::KPA_TO_HPA_FACTOR;
  out_data->soil1_pct = SoilRawToPercent(analogRead(RoomMonitorConfig::SOIL1_PIN));
  out_data->soil2_pct = SoilRawToPercent(analogRead(RoomMonitorConfig::SOIL2_PIN));
  return true;
}
