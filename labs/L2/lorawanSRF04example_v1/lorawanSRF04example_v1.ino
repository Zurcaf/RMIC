/*******************************************************************************
 * Copyright (c) 2015 Thomas Telkamp and Matthijs Kooijman
 *
 * Permission is hereby granted, free of charge, to anyone
 * obtaining a copy of this document and accompanying files,
 * to do whatever they want with them without any restriction,
 * including, but not limited to, copying, modification and redistribution.
 * NO WARRANTY OF ANY KIND IS PROVIDED.
 *
 * This example sends a valid LoRaWAN packet with payload "Hello,
 * world!", using frequency and encryption settings matching those of
 * the The Things Network.
 *
 * This uses OTAA (Over-the-air activation), where where a DevEUI and
 * application key is configured, which are used in an over-the-air
 * activation procedure where a DevAddr and session keys are
 * assigned/generated for use with all further communication.
 *
 * Note: LoRaWAN per sub-band duty-cycle limitation is enforced (1% in
 * g1, 0.1% in g2), but not the TTN fair usage policy (which is probably
 * violated by this sketch when left running for longer)!

 * To use this sketch, first register your application and device with
 * the things network, to set or generate an AppEUI, DevEUI and AppKey.
 * Multiple devices can use the same AppEUI, but each device has its own
 * DevEUI and AppKey.
 *
 * Do not forget to define the radio type correctly in config.h.
 *
 *******************************************************************************/

#include <lmic.h>
#include <hal/hal.h>
#include <SPI.h>
#include "pitches.h"

#define D5 5
#define D3 4

#define LED_PIN 13
#define BUZZER_PIN 12
#define MOTION_PIN 11 // Pin connected to motion detector

int alarm_count = 0;
int proximity = LOW, tx_proximity = 0;
int buzzer_enable = false;

uint8_t rcv_buffer[12];

// This EUI must be in little-endian format, so least-significant-byte
// first. When copying an EUI from ttnctl output, this means to reverse
// the bytes. For TTN issued EUIs the last bytes should be 0xD5, 0xB3,
// 0x70.
static const u1_t PROGMEM APPEUI[8]={ 0xC1, 0x9F, 0x00, 0xD1, 0x7E, 0xE5, 0xC3, 0x70 };
void os_getArtEui (u1_t* buf) { memcpy_P(buf, APPEUI, 8);}

// This should also be in little endian format, see above.
static const u1_t PROGMEM DEVEUI[8]={ 0xAA, 0x29, 0x86, 0xB4, 0x9E, 0xCE, 0x98, 0x00 };
void os_getDevEui (u1_t* buf) { memcpy_P(buf, DEVEUI, 8);}

// This key should be in big endian format (or, since it is not really a
// number but a block of memory, endianness does not really apply). In
// practice, a key taken from ttnctl can be copied as-is.
// The key shown here is the semtech default key.
static const u1_t PROGMEM APPKEY[16] = {0x8C, 0x76, 0x7F, 0xDB, 0xBE, 0x38, 0xC8, 0x8F, 0x1C, 0x6E, 0x8A, 0x2D, 0xF5, 0x5D, 0x2B, 0x2C };

void os_getDevKey (u1_t* buf) {  memcpy_P(buf, APPKEY, 16);}

static uint8_t mydata[4];
static osjob_t sendjob, checkjob;

// Schedule TX every this many seconds (might become longer due to duty
// cycle limitations).
const unsigned TX_INTERVAL_ALARM = 5;
const unsigned TX_INTERVAL_NORMAL = 5;
const unsigned CS_INTERVAL = 2;

// Pin mapping
const lmic_pinmap lmic_pins = {
    .nss = 10,
    .rxtx = LMIC_UNUSED_PIN,
    .rst = 9,
    .dio = {2, 6, 7},
};

void onEvent (ev_t ev) {
    Serial.print(os_getTime());
    Serial.print(": ");
    switch(ev) {
        case EV_SCAN_TIMEOUT:
            Serial.println(F("EV_SCAN_TIMEOUT"));
            break;
        case EV_BEACON_FOUND:
            Serial.println(F("EV_BEACON_FOUND"));
            break;
        case EV_BEACON_MISSED:
            Serial.println(F("EV_BEACON_MISSED"));
            break;
        case EV_BEACON_TRACKED:
            Serial.println(F("EV_BEACON_TRACKED"));
            break;
        case EV_JOINING:
            Serial.println(F("EV_JOINING"));
            break;
        case EV_JOINED:
            Serial.println(F("EV_JOINED"));

            // Disable link check validation (automatically enabled
            // during join, but not supported by TTN at this time).
            LMIC_setLinkCheckMode(0);
            break;
        case EV_RFU1:
            Serial.println(F("EV_RFU1"));
            break;
        case EV_JOIN_FAILED:
            Serial.println(F("EV_JOIN_FAILED"));
            break;
        case EV_REJOIN_FAILED:
            Serial.println(F("EV_REJOIN_FAILED"));
            break;
            break;
        case EV_TXCOMPLETE:
            Serial.println(F("EV_TXCOMPLETE (includes waiting for RX windows)"));
            if (LMIC.txrxFlags & TXRX_ACK)
              Serial.println(F("Received ack"));
          if (LMIC.dataLen) {
              Serial.println(F("Received "));
              Serial.println(LMIC.dataLen);
              Serial.println(F(" bytes of payload"));
            }

            if (LMIC.dataLen != 0 || LMIC.dataBeg != 0) {
              u1_t bPort = 0;
              Serial.println(LMIC.dataLen);
              Serial.println(LMIC.dataBeg);
              if (LMIC.txrxFlags & TXRX_PORT){
                bPort = LMIC.frame[LMIC.dataBeg - 1];
              }
              for (int i = 0 ; i < LMIC.dataLen ; i++){
                rcv_buffer[i] = LMIC.frame[LMIC.dataBeg + i];
              }
              Serial.println(rcv_buffer[0]);
//               Serial.write(rcv_buffer, LMIC.dataLen);
//               Serial.flush();
               //Serial.println("END");
            }

            // Schedule next transmission
            int interval = (proximity == LOW)?TX_INTERVAL_ALARM:TX_INTERVAL_NORMAL;
            os_setTimedCallback(&sendjob, os_getTime()+sec2osticks(interval), do_send);
            break;
        case EV_LOST_TSYNC:
            Serial.println(F("EV_LOST_TSYNC"));
            break;
        case EV_RESET:
            Serial.println(F("EV_RESET"));
            break;
        case EV_RXCOMPLETE:
            // data received in ping slot
            Serial.println(F("EV_RXCOMPLETE"));
            break;
        case EV_LINK_DEAD:
            Serial.println(F("EV_LINK_DEAD"));
            break;
        case EV_LINK_ALIVE:
            Serial.println(F("EV_LINK_ALIVE"));
            break;
         default:
            Serial.println(F("Unknown event"));
            break;
    }
}

int getdistancelvl(){
  int duration, distance;
  // Clears the D3 condition
  digitalWrite(D3, LOW);
  delayMicroseconds(2);
  // Sets the D3 HIGH (ACTIVE) for 10 microseconds
  digitalWrite(D3, HIGH);
  delayMicroseconds(10);
  digitalWrite(D3, LOW);
  // Reads the D5, returns the sound wave travel time in microseconds
  duration = pulseIn(D5, HIGH);
  // Calculating the distance
  distance = duration * 0.034 / 2; // Speed of sound wave divided by 2 (go and back)
  Serial.print("Distance: ");
  Serial.print(distance);
  Serial.println(" cm");
  if(distance < 20) { 
    return HIGH;
  }

  return LOW;

}

void check_sensor(osjob_t* j){
    int prev_proximity = proximity;
    proximity = getdistancelvl();  

    if ( proximity == HIGH ) // If the sensor's output goes low, motion is detected
    {
       digitalWrite(LED_PIN, HIGH);

       buzzer_enable = rcv_buffer[0];
       if ( buzzer_enable == 1 ) 
       {
          tone(BUZZER_PIN, NOTE_A4, 1000);
       }
    
       Serial.println("Motion detected!");
       Serial.println(buzzer_enable);


       tx_proximity = 1;

       if ( prev_proximity == LOW )
       {
          alarm_count++;
          do_send(&sendjob);
       }
    }
    else
    {
       digitalWrite(LED_PIN, LOW); 
       noTone(BUZZER_PIN);
       Serial.println("No motion...");
       
          do_send(&sendjob);
    }
    os_setTimedCallback(&checkjob, os_getTime()+sec2osticks(CS_INTERVAL), check_sensor);        
}

void do_send(osjob_t* j){
    // Check if there is not a current TX/RX job running
    if (LMIC.opmode & OP_TXRXPEND) {
        Serial.println(F("OP_TXRXPEND, not sending"));
    } else {
        mydata[0] = tx_proximity;
        mydata[1] = alarm_count >> 8;
        mydata[2] = alarm_count & 0x00FF;
        mydata[3] = buzzer_enable;
        
        tx_proximity = 0;
        
        // Prepare upstream data transmission at the next possible time.
        LMIC_setTxData2(1, mydata, sizeof(mydata), 0);
        Serial.println(F("Packet queued"));
    }
    // Next TX is scheduled after TX_COMPLETE event.
}

void setup() {
  
    Serial.begin(9600);
    pinMode(D3, OUTPUT); // D3 TRIGGER
    pinMode(D5, INPUT); // D5 ECHO
    Serial.println(F("Starting"));

    pinMode(LED_PIN,OUTPUT);
    digitalWrite(LED_PIN,LOW);
    pinMode(MOTION_PIN, INPUT_PULLUP);

    pinMode(BUZZER_PIN,OUTPUT);

    #ifdef VCC_ENABLE
    // For Pinoccio Scout boards
    pinMode(VCC_ENABLE, OUTPUT);
    digitalWrite(VCC_ENABLE, HIGH);
    delay(1000);
    #endif

    // LMIC init
    os_init();
    // Reset the MAC state. Session and pending data transfers will be discarded.
    LMIC_reset();
    LMIC_setClockError(MAX_CLOCK_ERROR * 1 / 100);

    // Start job (sending automatically starts OTAA too)
    do_send(&sendjob);

    rcv_buffer[0] = 0;
    check_sensor(&checkjob);
}

void loop() {
    os_runloop_once();
}
