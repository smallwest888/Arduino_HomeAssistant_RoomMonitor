#include "wifi_manager.h"

#include <WiFiNINA.h>

#include "config.h"

namespace {
uint32_t g_last_wifi_attempt_ms = 0UL;
}  // namespace

void WifiManager_Init() {
  WiFi.disconnect();
  g_last_wifi_attempt_ms = 0UL;
}

bool WifiManager_EnsureConnected() {
  if (WiFi.status() == WL_CONNECTED) {
    return true;
  }

  const uint32_t now = millis();
  if ((now - g_last_wifi_attempt_ms) < RoomMonitorConfig::WIFI_RETRY_DELAY_MS) {
    return false;
  }
  g_last_wifi_attempt_ms = now;

  Serial.print("Connecting to WiFi SSID: ");
  Serial.println(RoomMonitorConfig::WIFI_SSID);

  WiFi.disconnect();
  WiFi.begin(RoomMonitorConfig::WIFI_SSID, RoomMonitorConfig::WIFI_PASSWORD);
  return false;
}
