#include "mqtt_manager.h"

WiFiClient espClient;
PubSubClient client(espClient);

const char* mqtt_server = "192.168.1.16";

void mqtt_setup() {

    client.setServer(mqtt_server, 1883);

}

void reconnect() {

    while (!client.connected()) {

        Serial.println("Connecting to MQTT...");

        if (client.connect("ESP32Client")) {

            Serial.println("MQTT connected");
            client.publish("prosthesis/device01/status", "ESP32 connected");

        } else {

            Serial.print("Failed, rc=");
            Serial.println(client.state());
            delay(2000);

        }
    }
}

void mqtt_check() {

    if (!client.connected()) {
        reconnect();
    }

    client.loop();
}

void mqtt_publish(const char* topic, const char* payload) {

    client.publish(topic, payload);

}