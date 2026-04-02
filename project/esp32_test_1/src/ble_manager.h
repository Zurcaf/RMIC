#ifndef BLE_MANAGER_H
#define BLE_MANAGER_H

#include <Arduino.h>

enum class BLECommand { NONE, START, STOP, STATUS };
typedef void (*BLECommandCallback)(BLECommand);

void ble_init(BLECommandCallback onCommand);
void ble_notify_status(const char* json);
bool ble_is_connected();

#endif