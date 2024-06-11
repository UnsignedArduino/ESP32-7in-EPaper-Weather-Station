#ifndef WIFI_CONNECTION_H
#define WIFI_CONNECTION_H

#include "settings.h"
#include <Arduino.h>
#include <WiFiManager.h>

void generateHTMLForSelect(const char* dest, size_t destSize, const char* label,
                           const char* injectID, const char* targetHiddenID,
                           const char* options[], const char* optionLabels[],
                           size_t optionsSize, const char* selectedValue);

int8_t resetWiFiSettings();
int8_t connectToWiFi();

#endif
