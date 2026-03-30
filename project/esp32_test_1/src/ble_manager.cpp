#include <Arduino.h>
#include "ble_manager.h"
#include <NimBLEDevice.h>

NimBLECharacteristic *pCharacteristic;
bool deviceConnected = false;

// Callbacks
class MyServerCallbacks: public NimBLEServerCallbacks {
    void onConnect(NimBLEServer* pServer) {
        deviceConnected = true;
        Serial.println("Device connected");
    };

    void onDisconnect(NimBLEServer* pServer) {
        deviceConnected = false;
        Serial.println("Device disconnected");
        NimBLEDevice::startAdvertising(); // restart advertising
    }
};

void ble_init() {
    NimBLEDevice::init("PetBionic");

    NimBLEServer *pServer = NimBLEDevice::createServer();
    pServer->setCallbacks(new MyServerCallbacks());

    NimBLEService *pService = pServer->createService("1234");

    pCharacteristic = pService->createCharacteristic(
        "5678",
        NIMBLE_PROPERTY::NOTIFY
    );

    // NO descriptor
    // NO pService->start()

    NimBLEAdvertising *pAdvertising = NimBLEDevice::getAdvertising();
    pAdvertising->start();

    Serial.println("BLE initialized, waiting for connection...");
}

void ble_send_steps(int steps) {
    if (!deviceConnected) return;

    String json = "{\"steps\":" + String(steps) + "}";

    pCharacteristic->setValue(json.c_str());
    pCharacteristic->notify();
}

bool ble_is_connected() {
    return deviceConnected;
}