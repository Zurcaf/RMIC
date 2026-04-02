#include "ble_manager.h"
#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

#define SERVICE_UUID     "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CMD_CHAR_UUID    "beb5483e-36e1-4688-b7f5-ea07361b26a8"
#define STATUS_CHAR_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a9"

static BLECharacteristic* pStatusChar = nullptr;
static bool deviceConnected = false;
static BLECommandCallback commandCallback = nullptr;

class ServerCallbacks : public BLEServerCallbacks {
    void onConnect(BLEServer* s) {
        deviceConnected = true;
        Serial.println("[BLE] Connected");
    }
    void onDisconnect(BLEServer* s) {
        deviceConnected = false;
        Serial.println("[BLE] Disconnected");
        s->startAdvertising();
    }
};

class CmdCallbacks : public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic* c) {
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

    BLEDevice::init("PetBionic");

    BLEServer* pServer = BLEDevice::createServer();
    pServer->setCallbacks(new ServerCallbacks());

    BLEService* pService = pServer->createService(SERVICE_UUID);

    BLECharacteristic* pCmdChar = pService->createCharacteristic(
        CMD_CHAR_UUID,
        BLECharacteristic::PROPERTY_WRITE
    );
    pCmdChar->setCallbacks(new CmdCallbacks());

    pStatusChar = pService->createCharacteristic(
        STATUS_CHAR_UUID,
        BLECharacteristic::PROPERTY_NOTIFY
    );
    pStatusChar->addDescriptor(new BLE2902());

    pService->start();

    BLEAdvertising* pAdv = BLEDevice::getAdvertising();
    pAdv->addServiceUUID(SERVICE_UUID);
    pAdv->setScanResponse(true);
    pAdv->start();

    Serial.println("[BLE] Advertising as PetBionic");
}

void ble_notify_status(const char* json) {
    if (!deviceConnected || !pStatusChar) return;
    pStatusChar->setValue(json);
    pStatusChar->notify();
}

bool ble_is_connected() { return deviceConnected; }