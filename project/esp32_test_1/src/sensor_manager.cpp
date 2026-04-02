#include "sensor_manager.h"

void sensor_init() {
    Serial.println("[Sensor] Init (stub)");
}

SensorData sensor_read() {
    SensorData d;
    d.time_ms = millis();
    d.ax = 0.1; d.ay = 0.2; d.az = 9.8;
    d.gx = 0.0; d.gy = 0.0; d.gz = 0.0;
    d.force = 100.0;
    return d;
}