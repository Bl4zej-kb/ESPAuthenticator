#include <Arduino.h>
#include <WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include "mbedtls/md.h"
#include <Base32-Decode.h>


#include "password.h"



WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

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

int32_t getOTP(uint64_t time, unsigned char* secret, size_t secretLen) {
    uint64_t counter = time / 30;
    uint8_t msg[8];

    for (int8_t i = 7; i >= 0; i--) {
        msg[i] = counter & 0xFF;
        counter >>= 8;
    }

    uint8_t hash[20];

    mbedtls_md_context_t ctx;
    mbedtls_md_init(&ctx);

    mbedtls_md_setup(
        &ctx,
        mbedtls_md_info_from_type(MBEDTLS_MD_SHA1),
        1
    );

    mbedtls_md_hmac_starts(
        &ctx,
        secret,
        secretLen
    );

    mbedtls_md_hmac_update(&ctx, msg, 8);
    mbedtls_md_hmac_finish(&ctx, hash);

    mbedtls_md_free(&ctx);

    uint8_t offset = hash[19] & 0x0F;

    uint32_t binCode =
        ((uint32_t)(hash[offset] & 0x7F) << 24) |
        ((uint32_t)hash[offset + 1] << 16) |
        ((uint32_t)hash[offset + 2] << 8) |
        (uint32_t)hash[offset + 3];

    return binCode % 1000000;
}
char secretC[] = "AAA";
unsigned char secretD[64];
int secretLen;


void setup() {
    Serial.begin(9600);
    setToNtpTime();

    secretLen = base32decode(secretC, secretD, sizeof(secretD));
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
        else if (buf == 0x02) {
           Serial.println(getOTP(getTime(), secretD, secretLen));
        }
        else buf = 0xFF;
    }

    delay(10);
}