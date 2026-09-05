#include <Arduino.h>
#include <WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>







#include "password.h"



WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);





void setup() {
    Serial.begin(9600);
}


uint64_t whenUpdated = 0, whenSet = 0, set = 0;
uint8_t buf, timeBuf[8];

time_t getTime() {
    time_t timeNow;
    uint64_t delta = millis() - whenSet;

    timeNow = set + (delta - delta % 1000) / 1000;

    return timeNow;
}

time_t getNtpTime() {
    WiFi.begin(ssid, password);

    while ( WiFi.status() != WL_CONNECTED ) {
        delay ( 500 );
        Serial.println("Connecting...");
    }

    timeClient.update();
    whenUpdated = millis();

    WiFi.disconnect();

    return timeClient.getEpochTime();
}

void clearBuf() {
    while (Serial.available() > 0) {
        Serial.read();
    }
}

void loop() {


    if (Serial.available() > 0) {
        buf = Serial.read();

        if (buf == 0x00) {
            Serial.println(getTime());
        }
        else if (buf == 0x01) {
            set = getNtpTime();
            whenSet = whenUpdated;
        }
        else buf = 0xFF;
    }
}