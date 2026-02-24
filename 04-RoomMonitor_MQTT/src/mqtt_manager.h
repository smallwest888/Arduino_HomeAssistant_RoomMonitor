#ifndef MQTT_MANAGER_H
#define MQTT_MANAGER_H

#include "data_model.h"

void MqttManager_Init();
void MqttManager_Loop();
bool MqttManager_EnsureConnected();
bool MqttManager_PublishData(const SensorData* data);

#endif  // MQTT_MANAGER_H
