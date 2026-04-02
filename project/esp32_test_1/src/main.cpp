#include <Arduino.h>
#include "ble_manager.h"
#include "session_manager.h"
#include <SD.h>
#include <SPI.h>

void on_ble_command(BLECommand cmd) {
    switch (cmd) {
        case BLECommand::START:  session_start(); break;
        case BLECommand::STOP:   session_stop();  break;
        case BLECommand::STATUS: {
            String s = "{\"state\":\"" + String((int)session_get_state()) + "\"}";
            ble_notify_status(s.c_str());
            break;
        }
        default: break;
    }
}

void setup() {
    Serial.begin(115200);
    unsigned long start = millis();
    while (!Serial && millis() - start < 5000);

    Serial.println("SETUP START");
    session_init();
    ble_init(on_ble_command);
    Serial.println("SETUP DONE");
}

void loop() {
    session_tick();
    Serial.println("alive");
    delay(1000);
}