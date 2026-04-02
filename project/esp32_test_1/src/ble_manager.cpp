#include "ble_manager.h"
#include <Arduino.h>
#include <NimBLEDevice.h>

#define SERVICE_UUID     "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CMD_CHAR_UUID    "beb5483e-36e1-4688-b7f5-ea07361b26a8"
#define STATUS_CHAR_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a9"

static NimBLECharacteristic* pStatusChar = nullptr;
static bool deviceConnected = false;
static BLECommandCallback commandCallback = nullptr;

class ServerCallbacks : public NimBLEServerCallbacks {
    void onConnect(NimBLEServer* s) {
        deviceConnected = true;
        Serial.println("[BLE] Connected");
    }
    void onDisconnect(NimBLEServer* s) {
        deviceConnected = false;
        Serial.println("[BLE] Disconnected");
        NimBLEDevice::startAdvertising();
    }
};

class CmdCallbacks : public NimBLECharacteristicCallbacks {
    void onWrite(NimBLECharacteristic* c) {
        if (!commandCallback) return;
        std::string val = c->getValue();
        Serial.printf("[BLE] Received: %s\n", val.c_str());
        if      (val == "START")  commandCallback(BLECommand::START);
        else if (val == "STOP")   commandCallback(BLECommand::STOP);
        else if (val == "STATUS") commandCallback(BLECommand::STATUS);
    }
};

void ble_init(BLECommandCallback onCommand) {
    Serial.println("[BLE] Initializing...");
    commandCallback = onCommand;

    NimBLEDevice::init("PetBionic");

    NimBLEServer* pServer = NimBLEDevice::createServer();
    pServer->setCallbacks(new ServerCallbacks());

    NimBLEService* pService = pServer->createService(SERVICE_UUID);

    NimBLECharacteristic* pCmdChar = pService->createCharacteristic(
        CMD_CHAR_UUID,
        NIMBLE_PROPERTY::WRITE
    );
    pCmdChar->setCallbacks(new CmdCallbacks());

    pStatusChar = pService->createCharacteristic(
        STATUS_CHAR_UUID,
        NIMBLE_PROPERTY::NOTIFY
    );

    pService->start();

    NimBLEAdvertising* pAdv = NimBLEDevice::getAdvertising();
    pAdv->addServiceUUID(SERVICE_UUID);
    pAdv->start();

    Serial.println("[BLE] Advertising as PetBionic");
}

void ble_notify_status(const char* json) {
    if (!deviceConnected || !pStatusChar) return;
    pStatusChar->setValue(json);
    pStatusChar->notify();
}

bool ble_is_connected() { return deviceConnected; }