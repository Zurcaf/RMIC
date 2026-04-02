#include "sd_logger.h"
#include <SD.h>
#include <SPI.h>

#define SD_CS_PIN 8

static File logFile;
static File readFile;
static String currentPath;

static const int BUFFER_SIZE = 2048;
static char writeBuffer[BUFFER_SIZE];
static int bufferPos = 0;


void sd_init() {
    Serial.println("[SD] Initializing SPI...");
    SPI.begin(21, 7, 6, 8); // SCK=D6(21), MISO=D5(7), MOSI=D4(6), CS=D8(8)
    
    if (!SD.begin(8)) {
        Serial.println("[SD] Mount failed!");
        return;
    }
    
    Serial.printf("[SD] Card type: %d\n", SD.cardType());
    Serial.printf("[SD] Card size: %llu MB\n", SD.cardSize() / (1024 * 1024));
    Serial.println("[SD] Ready");
}

void sd_open_session(const String& sessionId) {
    currentPath = "/" + sessionId + ".csv";
    
    Serial.printf("[SD] Card type at open: %d\n", SD.cardType());
    Serial.printf("[SD] Trying to open: %s\n", currentPath.c_str());
    
    logFile = SD.open(currentPath, FILE_WRITE);
    if (!logFile) {
        Serial.println("[SD] Failed to open file!");
        // Try root directory listing
        File root = SD.open("/");
        if (root) {
            Serial.println("[SD] Root opened OK, listing files:");
            File f = root.openNextFile();
            while (f) {
                Serial.printf("  %s\n", f.name());
                f = root.openNextFile();
            }
            root.close();
        } else {
            Serial.println("[SD] Cannot open root either - card not mounted");
        }
        return;
    }
    logFile.println("time_ms,type,v1,v2,v3,v4,v5,v6");
    bufferPos = 0;
    Serial.printf("[SD] Logging to %s\n", currentPath.c_str());
}

static void flush_buffer() {
    if (bufferPos > 0 && logFile) {
        logFile.write((uint8_t*)writeBuffer, bufferPos);
        logFile.flush();
        bufferPos = 0;
    }
}

void sd_write(const SensorData& data) {
    int n = snprintf(writeBuffer + bufferPos,
                     BUFFER_SIZE - bufferPos,
                     "%lu,IMU,%.4f,%.4f,%.4f,%.4f,%.4f,%.4f\n",
                     data.time_ms,
                     data.ax, data.ay, data.az,
                     data.gx, data.gy, data.gz);
    bufferPos += n;

    n = snprintf(writeBuffer + bufferPos,
                 BUFFER_SIZE - bufferPos,
                 "%lu,HX,%.4f,0,0,0,0,0\n",
                 data.time_ms, data.force);
    bufferPos += n;

    if (bufferPos >= BUFFER_SIZE - 128) flush_buffer();
}

void sd_close_session() {
    flush_buffer();
    if (logFile) {
        logFile.close();
        Serial.println("[SD] Session file closed");
    }
}

void sd_open_for_read(const String& sessionId) {
    String path = "/" + sessionId + ".csv";
    readFile = SD.open(path, FILE_READ);
    if (!readFile) {
        Serial.printf("[SD] Can't open for read: %s\n", path.c_str());
        return;
    }
    Serial.printf("[SD] Reading %s\n", path.c_str());
}

bool sd_read_line(String& line) {
    if (!readFile || !readFile.available()) return false;
    line = readFile.readStringUntil('\n');
    line.trim();
    return line.length() > 0;
}

void sd_close_read() {
    if (readFile) readFile.close();
}