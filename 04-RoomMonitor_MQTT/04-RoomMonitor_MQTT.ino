/*
  Room Monitor main orchestration unit.
  Interface and implementation details are split into src/*.h and src/*.cpp.
*/

#include "src/config.h"
#include "src/data_model.h"
#include "src/display_service.h"
#include "src/mqtt_manager.h"
#include "src/sensor_service.h"
#include "src/wifi_manager.h"

namespace {
uint32_t g_last_publish_ms = 0UL;
}

void setup() {
  Serial.begin(RoomMonitorConfig::SERIAL_BAUD_RATE);
  while (!Serial) {
    ;  // Keep behavior explicit for board debug sessions.
  }

  SensorService_Init();
  DisplayService_Init();
  DisplayService_ShowBootText();
  delay(RoomMonitorConfig::DISPLAY_BOOT_HOLD_MS);
  WifiManager_Init();
  MqttManager_Init();
}

void loop() {
  const uint32_t now = millis();
  if ((now - g_last_publish_ms) >= RoomMonitorConfig::PUBLISH_INTERVAL_MS) {
    g_last_publish_ms = now;

    SensorData data = {};
    if (SensorService_Read(&data)) {
      DisplayService_ShowData(&data);
      Serial.print("Temperature(C): ");
      Serial.println(data.temperature_c);
      Serial.print("Humidity(%): ");
      Serial.println(data.humidity_pct);
      Serial.print("Pressure(hPa): ");
      Serial.println(data.pressure_hpa);
      Serial.print("Soil1(%): ");
      Serial.println(data.soil1_pct);
      Serial.print("Soil2(%): ");
      Serial.println(data.soil2_pct);
      if (MqttManager_EnsureConnected()) {
        (void)MqttManager_PublishData(&data);
      }
    }
  }

  MqttManager_Loop();
}

