#ifndef SD_LOGGER_H
#define SD_LOGGER_H

#include <Arduino.h>
#include "sensor_manager.h"

void sd_init();
void sd_open_session(const String& sessionId);
void sd_close_session();
void sd_write(const SensorData& data);
void sd_open_for_read(const String& sessionId);
bool sd_read_line(String& line);
void sd_close_read();

#endif

//Insert the SD card, clean, upload, monitor — then send START from nRF Connect, wait a few seconds, then STOP. You should see:

//[SD] Initializing...
//[SD] Ready
//[Session] Started: session_XXXXX
//[SD] Logging to /session_XXXXX.csv
//[Session] Stopped
//[SD] Session file closed