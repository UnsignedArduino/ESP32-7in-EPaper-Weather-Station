#ifndef WEATHER_H
#define WEATHER_H

#include "Settings.h"
#include <Arduino.h>
#include <ArduinoJson.h>
#include <WiFi.h>

const int8_t GET_WEATHER_SUCCESS = 0;
const int8_t GET_WEATHER_CONNECTION_FAIL = -1;
const int8_t GET_WEATHER_CONNECTION_TIMEOUT = -2;
const int8_t GET_WEATHER_PARSE_FAIL = -3;

const uint8_t MAX_FORECAST_DAYS = 7;
const uint8_t MAX_TIMESTAMP_SIZE = 20;

// clang-format off
struct WeatherData {
  char currISOTime[MAX_TIMESTAMP_SIZE];
  float currTemp;
  float currHumidity;
  bool currIsDay;
  uint8_t currWeatherCode;

  char forecastISOTimes[MAX_FORECAST_DAYS][MAX_TIMESTAMP_SIZE];
  uint8_t forecastWeatherCodes[MAX_FORECAST_DAYS];
  float forecastHighTemps[MAX_FORECAST_DAYS];
  float forecastLowTemps[MAX_FORECAST_DAYS];
};
// clang-format on
void printWeather(WeatherData& data);

int8_t getWeather(float latitude, float longitude, WeatherData& data);

#endif