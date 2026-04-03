#include "firestore_sync.h"
#include "sd_logger.h"
#include <HTTPClient.h>
#include <WiFiClientSecure.h>

#define FIREBASE_PROJECT "petbionic-71360"
#define FIREBASE_API_KEY "AIzaSyBdueBd5STH_rrrZxo59i-6CMR-ozFgyV0"
#define FIRESTORE_BASE   "https://firestore.googleapis.com/v1/projects/" \
                         FIREBASE_PROJECT "/databases/(default)/documents"

static bool post_doc(const String& path, const String& body) {
    WiFiClientSecure client;
    client.setInsecure();
    HTTPClient http;

    String url = String(FIRESTORE_BASE) + "/" + path + "?key=" FIREBASE_API_KEY;
    http.begin(client, url);
    http.addHeader("Content-Type", "application/json");
    int code = http.PATCH(body);
    bool ok = (code == 200);
    if (!ok) Serial.printf("[Firestore] Error %d: %s\n", code, http.getString().c_str());
    else Serial.printf("[Firestore] OK: %s\n", path.c_str());
    http.end();
    return ok;
}

bool firestore_sync(const String& sessionId) {
    Serial.println("[Firestore] Starting sync...");

    // Create session document
    String sessionBody =
        "{\"fields\":{"
        "\"sessionId\":{\"stringValue\":\"" + sessionId + "\"},"
        "\"device\":{\"stringValue\":\"PetBionic_01\"},"
        "\"timestamp\":{\"integerValue\":\"" + String(millis()) + "\"}"
        "}}";

    if (!post_doc("sessions/" + sessionId, sessionBody)) {
        Serial.println("[Firestore] Failed to create session doc");
        return false;
    }

    // Upload readings from SD
    sd_open_for_read(sessionId);
    String line;
    int i = 0;
    bool firstLine = true;
    while (sd_read_line(line)) {
        if (firstLine) { firstLine = false; continue; } // skip CSV header
        String body =
            "{\"fields\":{"
            "\"raw\":{\"stringValue\":\"" + line + "\"}"
            "}}";
        post_doc("sessions/" + sessionId + "/readings/" + String(i++), body);
        delay(20);
    }
    sd_close_read();

    Serial.printf("[Firestore] Sync complete — %d readings uploaded\n", i);
    return true;
}