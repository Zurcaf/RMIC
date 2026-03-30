#include <Arduino.h>
#include "session_manager.h"

bool active = false;

void session_start() {
    active = true;
    Serial.println("Session started");
}

void session_stop() {
    active = false;
    Serial.println("Session stopped");
}

bool session_active() {
    return active;
}