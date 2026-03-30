#include <Arduino.h>
#include "ble_manager.h"

int steps = 0;

void setup() {
    Serial.begin(115200);

    ble_init();

    Serial.println("System ready...");
}

void loop() {
    steps += 10;

    if (ble_is_connected()) {
        ble_send_steps(steps);
        Serial.println("Sent steps: " + String(steps));
    }

    delay(2000);
}