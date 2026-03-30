#ifndef MQTT_MANAGER_H
#define MQTT_MANAGER_H

#include <WiFi.h>
#include <PubSubClient.h>

void mqtt_setup();
void mqtt_check();
void mqtt_publish(const char* topic, const char* payload);

#endif