/*
  Room Monitor (Temperature, Humidity, Pressure) with MQTT for Home Assistant

  Hardware :
    - Arduino MKR WiFi 1010
    - Arduino MKR IoT Carrier (Env + Pressure sensors)

  Features :
    - Read Temperature, Humidity, Pressure from MKR IoT Carrier
    - Connect to WiFi
    - Publish sensor data via MQTT
    - Use Home Assistant MQTT Discovery for automatic sensor creation

  How to use :
    1. Fill in your WiFi SSID/PASSWORD and MQTT broker address/port/user/password below
    2. Upload this sketch to MKR WiFi 1010
    3. Make sure Home Assistant MQTT integration is working and listening to the same broker
    4. After boot, three sensors will appear in Home Assistant:
         - Room Temperature
         - Room Humidity
         - Room Pressure
*/

#include <WiFiNINA.h>
#include <PubSubClient.h>
#include <Arduino_MKRIoTCarrier.h>

// ---------- WiFi Settings (EDIT THIS) ----------
const char* ssid     = "胡大师的Xiaomi 15S Pro";
const char* password = "3.1415926";

// ---------- MQTT Settings (EDIT THIS) ----------
// Usually Home Assistant MQTT broker runs on the same network (e.g. 192.168.1.10)
const char* mqtt_server   = "10.235.235.119";
const uint16_t mqtt_port  = 1883;        // Default MQTT port
const char* mqtt_user     = "";          // Set if your broker requires auth, otherwise keep empty
const char* mqtt_password = "";          // Set if your broker requires auth

// Device / topic definitions for Home Assistant MQTT Discovery
// You can change "room_monitor" or room names as you like
const char* device_id   = "room_monitor";
const char* device_name = "Room Monitor";

// State topics
const char* topic_temp_state       = "home/room_monitor/temperature";
const char* topic_hum_state        = "home/room_monitor/humidity";
const char* topic_pressure_state   = "home/room_monitor/pressure";
const char* topic_soil1_state      = "home/room_monitor/soil1";
const char* topic_soil2_state      = "home/room_monitor/soil2";

// Discovery config topics (Home Assistant)
const char* topic_temp_config      = "homeassistant/sensor/room_monitor_temperature/config";
const char* topic_hum_config       = "homeassistant/sensor/room_monitor_humidity/config";
const char* topic_pressure_config  = "homeassistant/sensor/room_monitor_pressure/config";
const char* topic_soil1_config     = "homeassistant/sensor/room_monitor_soil1/config";
const char* topic_soil2_config     = "homeassistant/sensor/room_monitor_soil2/config";

// MQTT client
WiFiClient   wifiClient;
PubSubClient mqttClient(wifiClient);

// MKR IoT Carrier
MKRIoTCarrier carrier;

// Capacitive Soil Moisture sensors (from SmartFarm example)
#define SOIL1_PIN A5
#define SOIL2_PIN A6

// Timing for periodic publish
unsigned long lastPublishMs = 0;
const unsigned long publishIntervalMs = 10UL * 1000UL; // 10 seconds

// Flag to make sure discovery messages are only sent once per MQTT (re)connection
bool discoverySent = false;

// ---------- Helper: Connect to WiFi ----------
void connectWiFi() {
  Serial.print("Connecting to WiFi SSID: ");
  Serial.println(ssid);

  // If already connected, no need to reconnect
  if (WiFi.status() == WL_CONNECTED) {
    return;
  }

  WiFi.disconnect();
  delay(1000);

  WiFi.begin(ssid, password);

  uint8_t retry = 0;
  while (WiFi.status() != WL_CONNECTED && retry < 20) {
    delay(500);
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
  } else {
    Serial.println("WiFi connection failed");
  }
}

// ---------- Helper: Build a simple unique MQTT client ID ----------
String buildClientId() {
  byte mac[6];
  WiFi.macAddress(mac);

  String id = "MKRRoomMon-";
  for (int i = 0; i < 6; i++) {
    if (mac[i] < 16) id += "0";
    id += String(mac[i], HEX);
  }
  return id;
}

// ---------- Helper: Send Home Assistant MQTT Discovery messages ----------
void sendDiscoveryConfig() {
  // Device description shared by all entities
  String deviceJson = String("{\"identifiers\":[\"") + device_id +
                      "\"],\"name\":\"" + device_name +
                      "\",\"model\":\"MKR WiFi 1010 + MKR IoT Carrier\",\"manufacturer\":\"Arduino\"}";

  // Temperature sensor config
  String tempConfig = String("{") +
    "\"name\":\"Room Temperature\"," +
    "\"state_topic\":\"" + String(topic_temp_state) + "\"," +
    "\"unit_of_measurement\":\"°C\"," +
    "\"device_class\":\"temperature\"," +
    "\"state_class\":\"measurement\"," +
    "\"unique_id\":\"room_monitor_temperature\"," +
    "\"device\":" + deviceJson +
  "}";

  // Humidity sensor config
  String humConfig = String("{") +
    "\"name\":\"Room Humidity\"," +
    "\"state_topic\":\"" + String(topic_hum_state) + "\"," +
    "\"unit_of_measurement\":\"%\"," +
    "\"device_class\":\"humidity\"," +
    "\"state_class\":\"measurement\"," +
    "\"unique_id\":\"room_monitor_humidity\"," +
    "\"device\":" + deviceJson +
  "}";

  // Pressure sensor config
  // MKR IoT Carrier pressure example uses kPa; Home Assistant 常用 hPa
  // 1 kPa = 10 hPa，我们在发送状态时会做转换
  String pressureConfig = String("{") +
    "\"name\":\"Room Pressure\"," +
    "\"state_topic\":\"" + String(topic_pressure_state) + "\"," +
    "\"unit_of_measurement\":\"hPa\"," +
    "\"device_class\":\"pressure\"," +
    "\"state_class\":\"measurement\"," +
    "\"unique_id\":\"room_monitor_pressure\"," +
    "\"device\":" + deviceJson +
  "}";

  // Soil moisture 1 config (as a percentage sensor)
  String soil1Config = String("{") +
    "\"name\":\"Soil Moisture 1\"," +
    "\"state_topic\":\"" + String(topic_soil1_state) + "\"," +
    "\"unit_of_measurement\":\"%\"," +
    "\"state_class\":\"measurement\"," +
    "\"unique_id\":\"room_monitor_soil1\"," +
    "\"device\":" + deviceJson +
  "}";

  // Soil moisture 2 config
  String soil2Config = String("{") +
    "\"name\":\"Soil Moisture 2\"," +
    "\"state_topic\":\"" + String(topic_soil2_state) + "\"," +
    "\"unit_of_measurement\":\"%\"," +
    "\"state_class\":\"measurement\"," +
    "\"unique_id\":\"room_monitor_soil2\"," +
    "\"device\":" + deviceJson +
  "}";

  // Send with retain so HA can discover even if it starts later
  mqttClient.publish(topic_temp_config,     tempConfig.c_str(), true);
  mqttClient.publish(topic_hum_config,      humConfig.c_str(), true);
  mqttClient.publish(topic_pressure_config, pressureConfig.c_str(), true);
  mqttClient.publish(topic_soil1_config,    soil1Config.c_str(), true);
  mqttClient.publish(topic_soil2_config,    soil2Config.c_str(), true);

  Serial.println("Home Assistant discovery config published");
}

// ---------- Helper: Connect / reconnect to MQTT ----------
void connectMQTT() {
  if (WiFi.status() != WL_CONNECTED) {
    connectWiFi();
  }

  if (WiFi.status() != WL_CONNECTED) {
    // Still not connected, cannot proceed
    return;
  }

  mqttClient.setServer(mqtt_server, mqtt_port);

  while (!mqttClient.connected()) {
    Serial.print("Attempting MQTT connection to ");
    Serial.print(mqtt_server);
    Serial.print(":");
    Serial.print(mqtt_port);
    Serial.print(" ... ");

    String clientId = buildClientId();

    bool connected;
    if (strlen(mqtt_user) > 0) {
      connected = mqttClient.connect(clientId.c_str(), mqtt_user, mqtt_password);
    } else {
      connected = mqttClient.connect(clientId.c_str());
    }

    if (connected) {
      Serial.println("connected");
      discoverySent = false; // Will send discovery again after reconnection
    } else {
      Serial.print("failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }

  if (!discoverySent) {
    sendDiscoveryConfig();
    discoverySent = true;
  }
}

// ---------- Arduino Setup ----------
void setup() {
  Serial.begin(9600);
  while (!Serial) {
    ; // Wait for Serial monitor (useful when debugging)
  }

  // Initialize MKR IoT Carrier
  carrier.begin();

  // Connect to WiFi
  connectWiFi();

  // Configure MQTT client
  mqttClient.setServer(mqtt_server, mqtt_port);

  // Establish MQTT connection (and send discovery)
  connectMQTT();
}

// ---------- Arduino Loop ----------
void loop() {
  // Ensure WiFi and MQTT stay connected
  if (WiFi.status() != WL_CONNECTED) {
    connectWiFi();
  }

  if (!mqttClient.connected()) {
    connectMQTT();
  }

  mqttClient.loop();

  unsigned long now = millis();
  if (now - lastPublishMs >= publishIntervalMs) {
    lastPublishMs = now;

    // Read sensors from MKR IoT Carrier
    float temperature = carrier.Env.readTemperature(); // °C
    float humidity    = carrier.Env.readHumidity();    // %
    float pressure_kPa = carrier.Pressure.readPressure(); // kPa
    float pressure_hPa = pressure_kPa * 10.0f; // Convert kPa -> hPa

    // Read capacitive soil sensors (same mapping as SmartFarm example)
    int soil1_val = analogRead(SOIL1_PIN);
    int soil2_val = analogRead(SOIL2_PIN);
    int soil1_percentage = map(soil1_val, 0, 1023, 100, 0);
    int soil2_percentage = map(soil2_val, 0, 1023, 100, 0);

    // Debug print to Serial
    Serial.print("Temperature(C): ");
    Serial.println(temperature);
    Serial.print("Humidity(%): ");
    Serial.println(humidity);
    Serial.print("Pressure(hPa): ");
    Serial.println(pressure_hPa);
    Serial.print("Soil1(%): ");
    Serial.println(soil1_percentage);
    Serial.print("Soil2(%): ");
    Serial.println(soil2_percentage);

    // Publish as plain text values to MQTT
    // Use String(..., 1) to format with 1 decimal place
    String tempStr      = String(temperature, 1);
    String humStr       = String(humidity, 1);
    String pressureStr  = String(pressure_hPa, 1);
    String soil1Str     = String(soil1_percentage);
    String soil2Str     = String(soil2_percentage);

    mqttClient.publish(topic_temp_state,     tempStr.c_str(), true);     // retain last value
    mqttClient.publish(topic_hum_state,      humStr.c_str(), true);
    mqttClient.publish(topic_pressure_state, pressureStr.c_str(), true);
    mqttClient.publish(topic_soil1_state,    soil1Str.c_str(), true);
    mqttClient.publish(topic_soil2_state,    soil2Str.c_str(), true);
  }
}

