# Arduino Home Assistant Room Monitor

A smart room-monitoring project based on the **Arduino IoT Explore Kit Rev.2**, designed to integrate it with Home Assistant.

## Project Goal

This project implements an monitoring sensor that can:

- Measure temperature, humidity, pressure, and soil moisture
- Display live data on the MKR IoT Carrier circular screen
- Publish telemetry via MQTT
- Use Home Assistant MQTT Discovery for automatic device/entity creation

## Hardware and Software Stack

- **Hardware**
  - Arduino MKR WiFi 1010
  - Arduino MKR IoT Carrier
  - Soil moisture sensors (A5 / A6)
- **Libraries**
  - `Arduino_MKRIoTCarrier`
  - `WiFiNINA`
  - `PubSubClient`
- **Platform**
  - Home Assistant + Mosquitto MQTT Broker

## System Architecture

The codebase is modular, with clear separation of interface and implementation:

- `sensor_service`: sensor acquisition and data conversion
- `display_service`: circular dashboard and info panel rendering
- `wifi_manager`: Wi-Fi connection handling
- `mqtt_manager`: MQTT connect/reconnect, discovery, and publishing
- `carrier_platform`: shared hardware object initialization/access
- `config.h`: centralized parameters and constants (magic-number reduction)

The main sketch `04-RoomMonitor_MQTT.ino` acts as an orchestrator for timing and module coordination.

## Key Features

- **Center gauge mode switching (every 1s)**
  - Temperature (°C)
  - Soil moisture (%)
  - Pressure (hPa)
- **Bottom status panel**
  - Real-time H / P / S1 / S2 values
- **Non-blocking connectivity logic**
  - MQTT/Wi-Fi issues do not freeze the main display loop
- **Home Assistant auto-discovery**
  - Publishes `homeassistant/sensor/.../config` topics
- **Maintainability-focused design**
  - Core parameters centralized in `config.h`

## MQTT Topics

State topics:

- `home/room_monitor/temperature`
- `home/room_monitor/humidity`
- `home/room_monitor/pressure`
- `home/room_monitor/soil1`
- `home/room_monitor/soil2`

Discovery topics:

- `homeassistant/sensor/room_monitor_temperature/config`
- `homeassistant/sensor/room_monitor_humidity/config`
- `homeassistant/sensor/room_monitor_pressure/config`
- `homeassistant/sensor/room_monitor_soil1/config`
- `homeassistant/sensor/room_monitor_soil2/config`

## Use Cases

- Smart-home environmental monitoring
- Plant care and soil moisture supervision
- Embedded systems coursework and MQTT/HA integration demos
