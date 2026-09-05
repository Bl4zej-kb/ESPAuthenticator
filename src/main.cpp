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


uint64_t whenSet = 0, set = 0;
uint8_t buf, timeBuf[8];

uint64_t getTime() {
    time_t timeNow;
    uint64_t delta = esp_timer_get_time() / 1000 - whenSet;

    timeNow = set + (delta - delta % 1000) / 1000;

    return timeNow;
}

void setToNtpTime() {
    WiFi.begin(ssid, password);

    while ( WiFi.status() != WL_CONNECTED ) {
        delay ( 500 );
        Serial.println("Connecting...");
    }

    timeClient.update();
    set = timeClient.getEpochTime();
    whenSet = esp_timer_get_time() / 1000;

    WiFi.disconnect();

    Serial.println("Got time from NTP server");
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
            setToNtpTime();
        }
        else buf = 0xFF;
    }
}