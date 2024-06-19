#ifndef WEATHER_H
#define WEATHER_H

#include "Settings.h"
#include "localizedStrings.h"
#include <Arduino.h>
#include <ArduinoJson.h>
#include <TimeLib.h>
#include <WiFi.h>

const int8_t GET_WEATHER_SUCCESS = 0;
const int8_t GET_WEATHER_CONNECTION_FAIL = -1;
const int8_t GET_WEATHER_CONNECTION_TIMEOUT = -2;
const int8_t GET_WEATHER_PARSE_FAIL = -3;

const uint8_t MAX_FORECAST_DAYS = 7;
const uint8_t MAX_TIMESTAMP_SIZE = 20;

// clang-format off
struct WeatherData {
  time_t currUnixTime;
  uint8_t currWeatherCode;
  float currTemp;
  float currHighTemp;
  float currLowTemp;
  float currHumidity;
  bool currIsDay;

  time_t forecastUnixTimes[MAX_FORECAST_DAYS];
  uint8_t forecastWeatherCodes[MAX_FORECAST_DAYS];
  float forecastHighTemps[MAX_FORECAST_DAYS];
  float forecastLowTemps[MAX_FORECAST_DAYS];
};
// clang-format on
void printWeather(WeatherData& data);

int8_t getWeather(float latitude, float longitude, WeatherData& data);

const char* WMOCodeToFilename(uint8_t code, bool isDay);
const char* WMOCodeToLabel(uint8_t code);

#endif