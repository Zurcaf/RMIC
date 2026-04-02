#ifndef SENSOR_MANAGER_H
#define SENSOR_MANAGER_H

#include <Arduino.h>

struct SensorData {
    float ax, ay, az;      // accelerometer
    float gx, gy, gz;      // gyroscope
    float force;           // load cell
    unsigned long time_ms;
};

void sensor_init();
SensorData sensor_read();

#endif