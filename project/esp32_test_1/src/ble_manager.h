#ifndef BLE_MANAGER_H
#define BLE_MANAGER_H

void ble_init();
void ble_send_steps(int steps);
bool ble_is_connected();

#endif