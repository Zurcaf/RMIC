#include "wifi_manager.h"

const char* ssid = "POCO X5 Pro 5G";
const char* password = "tsb192cm01091999";

void wifi_connect() {

    Serial.println("Connecting to WiFi...");

    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    Serial.println("\nWiFi connected!");
    Serial.print("IP: ");
    Serial.println(WiFi.localIP());
}

void wifi_check() {

    if (WiFi.status() != WL_CONNECTED) {

        Serial.println("WiFi lost. Reconnecting...");
        wifi_connect();

    }
}