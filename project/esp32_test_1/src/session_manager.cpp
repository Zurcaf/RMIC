#include "session_manager.h"
#include "ble_manager.h"
#include "sd_logger.h"
#include "sensor_manager.h"

static SessionState state = SessionState::IDLE;
static String sessionId = "";

void session_init() {
    sensor_init();
    sd_init();
    Serial.println("[Session] Ready");
}

void session_start() {
    if (state != SessionState::IDLE) return;
    sessionId = "session_" + String(millis());
    sd_open_session(sessionId);
    state = SessionState::RECORDING;
    Serial.printf("[Session] Started: %s\n", sessionId.c_str());
    ble_notify_status("{\"state\":\"recording\"}");
}

void session_stop() {
    if (state != SessionState::RECORDING) return;
    sd_close_session();
    state = SessionState::IDLE;
    Serial.println("[Session] Stopped");
    ble_notify_status("{\"state\":\"idle\"}");
}

void session_tick() {
    if (state != SessionState::RECORDING) return;
    SensorData d = sensor_read();
    sd_write(d);
}

SessionState session_get_state() { return state; }
String session_get_id() { return sessionId; }