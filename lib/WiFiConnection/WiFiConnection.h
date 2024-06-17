#ifndef WIFI_CONNECTION_H
#define WIFI_CONNECTION_H

#include "settings.h"
#include <Arduino.h>
#include <WiFiManager.h>

const int8_t WIFI_CONNECTION_ERROR_TIMEOUT = -2;
const int8_t WIFI_CONNECTION_ERROR = -1;
const int8_t WIFI_CONNECTION_SUCCESS = 0;
const int8_t WIFI_CONNECTION_SUCCESS_CONFIG = 1;

int8_t resetWiFiSettings();
int8_t connectToWiFi(void (*onConfigAPLaunch)(char*, char*, char*));
int8_t disconnectFromWiFi();

#endif
