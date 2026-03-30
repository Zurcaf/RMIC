#include "sensor_manager.h"
#include <Arduino.h>

SensorData readSensors() {

    SensorData data;

    data.ax = random(-10,10) / 10.0;
    data.ay = random(-10,10) / 10.0;
    data.az = random(90,100) / 10.0;

    data.force = random(0,50);

    data.timestamp = millis();

    return data;
}