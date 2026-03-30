#ifndef SENSOR_MANAGER_H
#define SENSOR_MANAGER_H

struct SensorData {

    float ax;
    float ay;
    float az;

    float force;

    unsigned long timestamp;

};

SensorData readSensors();

#endif