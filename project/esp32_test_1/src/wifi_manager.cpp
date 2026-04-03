#include "wifi_manager.h"

const char* ssid = "POCO X5 Pro 5G";
const char* password = "tsb192cm01091999";

void wifi_connect() {
    if (WiFi.status() == WL_CONNECTED) return;
    
    Serial.println("[WiFi] Scanning networks...");
    int n = WiFi.scanNetworks();
    Serial.printf("[WiFi] Found %d networks:\n", n);
    for (int i = 0; i < n; i++) {
        Serial.printf("  %d: %s (%d dBm) %s\n", i, 
            WiFi.SSID(i).c_str(), 
            WiFi.RSSI(i),
            WiFi.encryptionType(i) == WIFI_AUTH_OPEN ? "open" : "encrypted");
    }
    
    Serial.printf("[WiFi] Connecting to: '%s'\n", ssid);
    WiFi.begin(ssid, password);
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20) {
        delay(500);
        Serial.printf("  status: %d\n", WiFi.status());
        attempts++;
    }
    if (WiFi.status() == WL_CONNECTED) {
        Serial.printf("[WiFi] Connected! IP: %s\n", WiFi.localIP().toString().c_str());
    } else {
        Serial.printf("[WiFi] Failed! Last status: %d\n", WiFi.status());
    }
}

void wifi_check() {
    if (WiFi.status() != WL_CONNECTED) wifi_connect();
}