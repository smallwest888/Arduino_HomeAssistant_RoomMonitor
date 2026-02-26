#include "mqtt_manager.h"

#include <PubSubClient.h>
#include <WiFiNINA.h>

#include "config.h"
#include "wifi_manager.h"

namespace {
WiFiClient g_wifi_client;
PubSubClient g_mqtt_client(g_wifi_client);
bool g_discovery_sent = false;
uint32_t g_last_connect_attempt_ms = 0UL;
uint32_t g_retry_delay_ms = RoomMonitorConfig::MQTT_RETRY_DELAY_MS;

String BuildClientId() {
  byte mac[RoomMonitorConfig::MAC_ADDRESS_LENGTH];
  WiFi.macAddress(mac);

  String client_id = "MKRRoomMon-";
  for (uint8_t i = 0U; i < RoomMonitorConfig::MAC_ADDRESS_LENGTH; i++) {
    if (mac[i] < RoomMonitorConfig::HEX_ZERO_PAD_THRESHOLD) {
      client_id += "0";
    }
    client_id += String(mac[i], HEX);
  }
  return client_id;
}

void SendDiscoveryConfig() {
  const String device_json = String("{\"identifiers\":[\"") + RoomMonitorConfig::DEVICE_ID +
                             "\"],\"name\":\"" + RoomMonitorConfig::DEVICE_NAME +
                             "\",\"model\":\"" + RoomMonitorConfig::DEVICE_MODEL +
                             "\",\"manufacturer\":\"" + RoomMonitorConfig::DEVICE_MANUFACTURER + "\"}";

  const String temp_config = String("{") +
                             "\"name\":\"Room Temperature\"," +
                             "\"state_topic\":\"" + String(RoomMonitorConfig::TOPIC_TEMP_STATE) + "\"," +
                             "\"unit_of_measurement\":\"°C\"," +
                             "\"device_class\":\"temperature\"," +
                             "\"state_class\":\"measurement\"," +
                             "\"unique_id\":\"room_monitor_temperature\"," +
                             "\"device\":" + device_json + "}";

  const String hum_config = String("{") +
                            "\"name\":\"Room Humidity\"," +
                            "\"state_topic\":\"" + String(RoomMonitorConfig::TOPIC_HUM_STATE) + "\"," +
                            "\"unit_of_measurement\":\"%\"," +
                            "\"device_class\":\"humidity\"," +
                            "\"state_class\":\"measurement\"," +
                            "\"unique_id\":\"room_monitor_humidity\"," +
                            "\"device\":" + device_json + "}";

  const String pressure_config = String("{") +
                                 "\"name\":\"Room Pressure\"," +
                                 "\"state_topic\":\"" + String(RoomMonitorConfig::TOPIC_PRESSURE_STATE) + "\"," +
                                 "\"unit_of_measurement\":\"hPa\"," +
                                 "\"device_class\":\"pressure\"," +
                                 "\"state_class\":\"measurement\"," +
                                 "\"unique_id\":\"room_monitor_pressure\"," +
                                 "\"device\":" + device_json + "}";

  const String soil1_config = String("{") +
                              "\"name\":\"Soil Moisture 1\"," +
                              "\"state_topic\":\"" + String(RoomMonitorConfig::TOPIC_SOIL1_STATE) + "\"," +
                              "\"unit_of_measurement\":\"%\"," +
                              "\"state_class\":\"measurement\"," +
                              "\"unique_id\":\"room_monitor_soil1\"," +
                              "\"device\":" + device_json + "}";

  const String soil2_config = String("{") +
                              "\"name\":\"Soil Moisture 2\"," +
                              "\"state_topic\":\"" + String(RoomMonitorConfig::TOPIC_SOIL2_STATE) + "\"," +
                              "\"unit_of_measurement\":\"%\"," +
                              "\"state_class\":\"measurement\"," +
                              "\"unique_id\":\"room_monitor_soil2\"," +
                              "\"device\":" + device_json + "}";

  const bool ok_temp = g_mqtt_client.publish(RoomMonitorConfig::TOPIC_TEMP_CONFIG, temp_config.c_str(), true);
  const bool ok_hum = g_mqtt_client.publish(RoomMonitorConfig::TOPIC_HUM_CONFIG, hum_config.c_str(), true);
  const bool ok_pressure = g_mqtt_client.publish(RoomMonitorConfig::TOPIC_PRESSURE_CONFIG, pressure_config.c_str(), true);
  const bool ok_soil1 = g_mqtt_client.publish(RoomMonitorConfig::TOPIC_SOIL1_CONFIG, soil1_config.c_str(), true);
  const bool ok_soil2 = g_mqtt_client.publish(RoomMonitorConfig::TOPIC_SOIL2_CONFIG, soil2_config.c_str(), true);

  const bool all_ok = ok_temp && ok_hum && ok_pressure && ok_soil1 && ok_soil2;
  Serial.print("Discovery publish ");
  Serial.println(all_ok ? "OK" : "FAILED");
  if (!all_ok) {
    Serial.print("Discovery payload lengths: ");
    Serial.print("temp=");
    Serial.print(temp_config.length());
    Serial.print(", hum=");
    Serial.print(hum_config.length());
    Serial.print(", pressure=");
    Serial.print(pressure_config.length());
    Serial.print(", soil1=");
    Serial.print(soil1_config.length());
    Serial.print(", soil2=");
    Serial.println(soil2_config.length());
  }
}
}  // namespace

// If we dont check wifi and mqtt connection, main loop will be blocked
namespace {
void PublishDiscoveryIfNeeded() {
  if (!g_discovery_sent) {
    SendDiscoveryConfig();
    g_discovery_sent = true;
  }
}

bool IsReconnectWindowOpen() {
  const uint32_t now = millis();
  if ((now - g_last_connect_attempt_ms) < g_retry_delay_ms) {
    return false;
  }
  g_last_connect_attempt_ms = now;
  return true;
}

bool TryConnectMqtt() {
  const String client_id = BuildClientId();
  if (strlen(RoomMonitorConfig::MQTT_USER) > 0U) {
    return g_mqtt_client.connect(
        client_id.c_str(),
        RoomMonitorConfig::MQTT_USER,
        RoomMonitorConfig::MQTT_PASSWORD);
  }
  return g_mqtt_client.connect(client_id.c_str());
}

void UpdateRetryDelayAfterConnect(const bool connected) {
  if (connected) {
    g_retry_delay_ms = RoomMonitorConfig::MQTT_RETRY_DELAY_MS;
    return;
  }
  if (g_retry_delay_ms < RoomMonitorConfig::MQTT_RETRY_MAX_DELAY_MS) {
    g_retry_delay_ms *= 2UL;
    if (g_retry_delay_ms > RoomMonitorConfig::MQTT_RETRY_MAX_DELAY_MS) {
      g_retry_delay_ms = RoomMonitorConfig::MQTT_RETRY_MAX_DELAY_MS;
    }
  }
}
}  // namespace

void MqttManager_Init() {
  g_mqtt_client.setBufferSize(RoomMonitorConfig::MQTT_BUFFER_SIZE_BYTES);
  g_mqtt_client.setServer(RoomMonitorConfig::MQTT_SERVER, RoomMonitorConfig::MQTT_PORT);
  Serial.print("MQTT buffer size set to ");
  Serial.println(RoomMonitorConfig::MQTT_BUFFER_SIZE_BYTES);
}

void MqttManager_Loop() {
  g_mqtt_client.loop();
}

bool MqttManager_EnsureConnected() {
  if (g_mqtt_client.connected()) {
    UpdateRetryDelayAfterConnect(true);
    PublishDiscoveryIfNeeded();
    return true;
  }

  if (!WifiManager_EnsureConnected()) {
    return false;
  }

  if (!IsReconnectWindowOpen()) {
    return false;
  }

  Serial.print("Attempting MQTT connection to ");
  Serial.print(RoomMonitorConfig::MQTT_SERVER);
  Serial.print(":");
  Serial.print(RoomMonitorConfig::MQTT_PORT);
  Serial.print(" ... ");

  const bool connected = TryConnectMqtt();

  if (connected) {
    Serial.println("connected");
    g_discovery_sent = false;
    UpdateRetryDelayAfterConnect(true);
    PublishDiscoveryIfNeeded();
  } else {
    Serial.print("failed, rc=");
    Serial.println(g_mqtt_client.state());
    UpdateRetryDelayAfterConnect(false);
  }

  return connected;
}

bool MqttManager_PublishData(const SensorData* data) {
  if (data == nullptr) {
    return false;
  }
  if (!g_mqtt_client.connected()) {
    return false;
  }

  const String temp_str = String(data->temperature_c, RoomMonitorConfig::FLOAT_DECIMALS);
  const String hum_str = String(data->humidity_pct, RoomMonitorConfig::FLOAT_DECIMALS);
  const String pressure_str = String(data->pressure_hpa, RoomMonitorConfig::FLOAT_DECIMALS);
  const String soil1_str = String(data->soil1_pct);
  const String soil2_str = String(data->soil2_pct);

  struct StateMsg {
    const char* topic;
    const char* payload;
  };
  const StateMsg messages[] = {
      {RoomMonitorConfig::TOPIC_TEMP_STATE, temp_str.c_str()},
      {RoomMonitorConfig::TOPIC_HUM_STATE, hum_str.c_str()},
      {RoomMonitorConfig::TOPIC_PRESSURE_STATE, pressure_str.c_str()},
      {RoomMonitorConfig::TOPIC_SOIL1_STATE, soil1_str.c_str()},
      {RoomMonitorConfig::TOPIC_SOIL2_STATE, soil2_str.c_str()}};

  bool all_ok = true;
  for (uint8_t i = 0U; i < (sizeof(messages) / sizeof(messages[0])); i++) {
    all_ok = g_mqtt_client.publish(messages[i].topic, messages[i].payload, true) && all_ok;
  }
  return all_ok;
}
