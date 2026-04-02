#ifndef SESSION_MANAGER_H
#define SESSION_MANAGER_H

#include <Arduino.h>

enum class SessionState { IDLE, RECORDING, SYNCING, DONE };

void session_init();
void session_start();
void session_stop();
void session_tick();
SessionState session_get_state();
String session_get_id();

#endif