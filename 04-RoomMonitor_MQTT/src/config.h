#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

namespace RoomMonitorConfig {

// WiFi settings
constexpr const char* WIFI_SSID = "IoTLab";
constexpr const char* WIFI_PASSWORD = "91500267";

// MQTT settings
constexpr const char* MQTT_SERVER = "192.168.0.137";
constexpr uint16_t MQTT_PORT = 1883;
constexpr const char* MQTT_USER = "t1-1";
constexpr const char* MQTT_PASSWORD = "Softwareprojectsmarthomedemolab";

// Device metadata
constexpr const char* DEVICE_ID = "room_monitor";
constexpr const char* DEVICE_NAME = "Room Monitor";
constexpr const char* DEVICE_MODEL = "MKR WiFi 1010 + MKR IoT Carrier";
constexpr const char* DEVICE_MANUFACTURER = "Arduino";

// MQTT state topics
constexpr const char* TOPIC_TEMP_STATE = "home/room_monitor/temperature";
constexpr const char* TOPIC_HUM_STATE = "home/room_monitor/humidity";
constexpr const char* TOPIC_PRESSURE_STATE = "home/room_monitor/pressure";
constexpr const char* TOPIC_SOIL1_STATE = "home/room_monitor/soil1";
constexpr const char* TOPIC_SOIL2_STATE = "home/room_monitor/soil2";

// Home Assistant discovery topics
constexpr const char* TOPIC_TEMP_CONFIG = "homeassistant/sensor/room_monitor_temperature/config";
constexpr const char* TOPIC_HUM_CONFIG = "homeassistant/sensor/room_monitor_humidity/config";
constexpr const char* TOPIC_PRESSURE_CONFIG = "homeassistant/sensor/room_monitor_pressure/config";
constexpr const char* TOPIC_SOIL1_CONFIG = "homeassistant/sensor/room_monitor_soil1/config";
constexpr const char* TOPIC_SOIL2_CONFIG = "homeassistant/sensor/room_monitor_soil2/config";

// Timing and retry parameters
constexpr uint32_t PUBLISH_INTERVAL_MS = 10UL * 1000UL;
constexpr uint32_t DISPLAY_REFRESH_MS = 1000UL;
constexpr uint32_t WIFI_RETRY_DELAY_MS = 500UL;
constexpr uint8_t WIFI_MAX_RETRIES = 20U;
constexpr uint32_t MQTT_RETRY_DELAY_MS = 5000UL;

// Serial and numeric formatting
constexpr uint32_t SERIAL_BAUD_RATE = 9600UL;
constexpr uint8_t FLOAT_DECIMALS = 1U;
constexpr uint8_t MAC_ADDRESS_LENGTH = 6U;

// Sensor pins and mapping
constexpr pin_size_t SOIL1_PIN = A5;
constexpr pin_size_t SOIL2_PIN = A6;
constexpr int SOIL_ADC_MIN = 0;
constexpr int SOIL_ADC_MAX = 1023;
constexpr int SOIL_PERCENT_MIN = 0;
constexpr int SOIL_PERCENT_MAX = 100;
constexpr float KPA_TO_HPA_FACTOR = 10.0F;

// Display colors and layout
constexpr uint16_t DISPLAY_COLOR_WHITE = 0xFFFFU;
constexpr uint16_t DISPLAY_COLOR_RED = 0xF800U;
constexpr uint16_t DISPLAY_COLOR_BLUE = 0x001FU;
constexpr uint16_t DISPLAY_COLOR_BLACK = 0x0000U;
constexpr uint16_t DISPLAY_COLOR_CYAN = 0x07FFU;
constexpr uint16_t DISPLAY_COLOR_GREEN = 0x07E0U;
constexpr uint16_t DISPLAY_COLOR_ORANGE = 0xFC00U;
constexpr uint16_t DISPLAY_COLOR_GRAY = 0x7BEFU;
constexpr uint8_t DISPLAY_TEXT_SIZE_SMALL = 2U;
constexpr uint8_t DISPLAY_TEXT_SIZE_LARGE = 4U;
constexpr int16_t DISPLAY_BOOT_X = 60;
constexpr int16_t DISPLAY_BOOT_Y = 80;
constexpr int16_t DISPLAY_DATA_X = 10;
constexpr int16_t DISPLAY_DATA_Y = 20;
constexpr uint32_t DISPLAY_BOOT_HOLD_MS = 1500UL;
constexpr int16_t DISPLAY_OUTER_MARGIN = 12;
constexpr int16_t DISPLAY_GAUGE_RING_OFFSET = 24;
constexpr int16_t DISPLAY_GAUGE_TICK_INNER_OFFSET = 10;
constexpr int16_t DISPLAY_GAUGE_TICK_OUTER_OFFSET = 2;
constexpr int16_t DISPLAY_GAUGE_NEEDLE_OFFSET = 30;
constexpr int16_t DISPLAY_INFO_PANEL_X = 12;
constexpr int16_t DISPLAY_INFO_PANEL_Y = 190;
constexpr int16_t DISPLAY_INFO_PANEL_W = 216;
constexpr int16_t DISPLAY_INFO_PANEL_H = 42;
constexpr int16_t DISPLAY_INFO_ROW_Y = 198;
constexpr float DISPLAY_TEMP_MIN_C = -10.0F;
constexpr float DISPLAY_TEMP_MAX_C = 50.0F;
constexpr float DISPLAY_GAUGE_START_DEG = 135.0F;
constexpr float DISPLAY_GAUGE_END_DEG = 405.0F;
constexpr float DISPLAY_DEG_TO_RAD = 0.01745329252F;

}  // namespace RoomMonitorConfig

#endif  // CONFIG_H
