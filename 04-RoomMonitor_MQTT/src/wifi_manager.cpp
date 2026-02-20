#include "wifi_manager.h"

#include <WiFiNINA.h>

#include "config.h"

void WifiManager_Init() {
  WiFi.disconnect();
}

bool WifiManager_EnsureConnected() {
  if (WiFi.status() == WL_CONNECTED) {
    return true;
  }

  Serial.print("Connecting to WiFi SSID: ");
  Serial.println(RoomMonitorConfig::WIFI_SSID);

  WiFi.disconnect();
  delay(2UL * RoomMonitorConfig::WIFI_RETRY_DELAY_MS);
  WiFi.begin(RoomMonitorConfig::WIFI_SSID, RoomMonitorConfig::WIFI_PASSWORD);

  uint8_t retry = 0U;
  while ((WiFi.status() != WL_CONNECTED) && (retry < RoomMonitorConfig::WIFI_MAX_RETRIES)) {
    delay(RoomMonitorConfig::WIFI_RETRY_DELAY_MS);
    Serial.print(".");
    retry++;
  }

  Serial.println();
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("WiFi connected");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
    Serial.print("Signal Strength (dBm): ");
    Serial.println(WiFi.RSSI());
    return true;
  }

  Serial.println("WiFi connection failed");
  return false;
}
