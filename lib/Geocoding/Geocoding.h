
#ifndef GEOCODING_H
#define GEOCODING_H

#include "Settings.h"
#include <Arduino.h>
#include <ArduinoJson.h>
#include <Preferences.h>
#include <WiFi.h>

const int8_t GET_COORDINATE_SUCCESS = 0;
const int8_t GET_COORDINATE_SUCCESS_CACHE = 1;
const int8_t GET_COORDINATE_CONNECTION_FAIL = -1;
const int8_t GET_COORDINATE_CONNECTION_TIMEOUT = -2;
const int8_t GET_COORDINATE_PARSE_FAIL = -3;

const uint8_t MAX_NAME_SIZE = 32;
// clang-format off
struct GeocodeData {
  float latitude;
  float longitude;
  char name[MAX_NAME_SIZE];
  char country[MAX_NAME_SIZE];
  char admin1[MAX_NAME_SIZE];
  char admin2[MAX_NAME_SIZE];
  char admin3[MAX_NAME_SIZE];
  char admin4[MAX_NAME_SIZE];
};
// clang-format on
void printGeocode(GeocodeData& data);

int8_t getGeocode(char cityOrPostalCode[MAX_CITY_OR_POSTAL_CODE_LENGTH],
                  GeocodeData& data);

#endif
