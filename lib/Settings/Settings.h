#ifndef SETTINGS_H
#define SETTINGS_H

#include <Arduino.h>
#include <Preferences.h>

const uint8_t MAX_CITY_OR_POSTAL_CODE_LENGTH = 32;
extern char cityOrPostalCodeSetting[MAX_CITY_OR_POSTAL_CODE_LENGTH];

const uint8_t MAX_TEMP_UNIT_LENGTH = 2;
const uint8_t MAX_TEMP_UNITS = 2;
extern const char* TEMP_UNIT_FAHRENHEIT;
extern const char* TEMP_UNIT_CELSIUS;
extern const char* TEMP_UNITS[MAX_TEMP_UNITS];
extern const char* TEMP_UNITS_LABELS[MAX_TEMP_UNITS];
extern char tempUnitSetting[MAX_TEMP_UNIT_LENGTH];

const uint8_t MAX_WIND_SPEED_UNIT_LENGTH = 4;
const uint8_t MAX_WIND_SPEED_UNITS = 4;
extern const char* WIND_SPEED_UNIT_KMH;
extern const char* WIND_SPEED_UNIT_MS;
extern const char* WIND_SPEED_UNIT_MPH;
extern const char* WIND_SPEED_UNIT_KN;
extern const char* WIND_SPEED_UNITS[MAX_WIND_SPEED_UNITS];
extern const char* WIND_SPEED_UNITS_LABELS[MAX_WIND_SPEED_UNITS];
extern char windSpeedUnitSetting[MAX_WIND_SPEED_UNIT_LENGTH];

const uint8_t MAX_PRECIPITATION_UNIT_LENGTH = 3;
const uint8_t MAX_PRECIPITATION_UNITS = 2;
extern const char* PRECIPITATION_UNIT_MM;
extern const char* PRECIPITATION_UNIT_IN;
extern const char* PRECIPITATION_UNITS[MAX_PRECIPITATION_UNITS];
extern const char* PRECIPITATION_UNITS_LABELS[MAX_PRECIPITATION_UNITS];
extern char precipitationUnitSetting[MAX_PRECIPITATION_UNIT_LENGTH];

const uint8_t MAX_LANGUAGE_LENGTH = 8;
const uint8_t MAX_LANGUAGES = 3;
extern const char* LANGUAGE_EN;
extern const char* LANGUAGE_CN_SIMP;
extern const char* LANGUAGE_CN_TRAD;
extern const char* LANGUAGES[MAX_LANGUAGES];
extern const char* LANGUAGES_LABELS[MAX_LANGUAGES];
extern char languageSetting[MAX_LANGUAGE_LENGTH];

bool saveSettings();
bool loadSettings();

void printSettings();

#endif