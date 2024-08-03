#ifndef ESP32_7IN_EPAPER_WEATHER_STATION_CONFIG_H
#define ESP32_7IN_EPAPER_WEATHER_STATION_CONFIG_H

#include <Arduino.h>

const uint8_t RETRY_TIME = 15; // minutes
const char* NTP_SERVER_1 = "time.nist.gov";
const char* NTP_SERVER_2 = "pool.ntp.org";
const char* NTP_SERVER_3 = "time.cloudflare.com";

#define BLYNK_TEMPLATE_ID "TMPxxxxxxxxxx"
#define BLYNK_TEMPLATE_NAME "Weather Station"
const char* BLYNK_AUTH_TOKEN = "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx";

#define BLYNK_FIRMWARE_VERSION "0.0.1"

#endif // ESP32_7IN_EPAPER_WEATHER_STATION_CONFIG_H
