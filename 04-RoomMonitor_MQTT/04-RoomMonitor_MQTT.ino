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
uint32_t g_last_display_ms = 0UL;
SensorData g_last_data = {};
bool g_has_data = false;
constexpr uint32_t SERIAL_WAIT_TIMEOUT_MS = 2000UL;
}

void setup() {
  Serial.begin(RoomMonitorConfig::SERIAL_BAUD_RATE);
  const uint32_t serial_wait_start = millis();
  while (!Serial && ((millis() - serial_wait_start) < SERIAL_WAIT_TIMEOUT_MS)) {
    ;  // Avoid blocking forever when no serial monitor is attached.
  }

  SensorService_Init();
  DisplayService_Init();
  DisplayService_ShowBootText();
  delay(RoomMonitorConfig::DISPLAY_BOOT_HOLD_MS);
  WifiManager_Init();
  MqttManager_Init();

  // Try to switch to dashboard immediately after boot screen.
  SensorData initial_data = {};
  if (SensorService_Read(&initial_data)) {
    g_last_data = initial_data;
  }
  g_has_data = true;
  g_last_display_ms = millis();
  DisplayService_ShowData(&g_last_data);
}

void loop() {
  const uint32_t now = millis();
  if ((now - g_last_display_ms) >= RoomMonitorConfig::DISPLAY_REFRESH_MS) {
    g_last_display_ms = now;

    SensorData data = {};
    if (SensorService_Read(&data)) {
      g_last_data = data;
      g_has_data = true;
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
    } else {
      Serial.println("Sensor read failed, keep last values");
    }

    if (g_has_data) {
      DisplayService_ShowData(&g_last_data);
    }
  }

  if (g_has_data && ((now - g_last_publish_ms) >= RoomMonitorConfig::PUBLISH_INTERVAL_MS)) {
    g_last_publish_ms = now;
    (void)MqttManager_PublishData(&g_last_data);
  }

  (void)MqttManager_EnsureConnected();
  MqttManager_Loop();
}

