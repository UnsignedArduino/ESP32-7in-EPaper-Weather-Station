#include "Settings.h"

char cityOrPostalCodeSetting[MAX_CITY_OR_POSTAL_CODE_LENGTH] = "";

const char* TEMP_UNIT_FAHRENHEIT = "f";
const char* TEMP_UNIT_CELSIUS = "c";
const char* TEMP_UNITS[MAX_TEMP_UNITS] = {TEMP_UNIT_FAHRENHEIT,
                                          TEMP_UNIT_CELSIUS};
const char* TEMP_UNITS_LABELS[MAX_TEMP_UNITS] = {"Fahrenheit (f)",
                                                 "Celsius (c)"};
char tempUnitSetting[MAX_TEMP_UNIT_LENGTH] = "f";

const char* WIND_SPEED_UNIT_KMH = "Kmh";
const char* WIND_SPEED_UNIT_MS = "ms";
const char* WIND_SPEED_UNIT_MPH = "mph";
const char* WIND_SPEED_UNIT_KN = "kn";
const char* WIND_SPEED_UNITS[MAX_WIND_SPEED_UNITS] = {
  WIND_SPEED_UNIT_KMH, WIND_SPEED_UNIT_MS, WIND_SPEED_UNIT_MPH,
  WIND_SPEED_UNIT_KN};
const char* WIND_SPEED_UNITS_LABELS[MAX_WIND_SPEED_UNITS] = {
  "Kilometers per hour (Kmh)", "Meters per second (ms)", "Miles per hour (mph)",
  "Knots (kn)"};
char windSpeedUnitSetting[MAX_WIND_SPEED_UNIT_LENGTH] = "Kmh";

const char* PRECIPITATION_UNIT_MM = "mm";
const char* PRECIPITATION_UNIT_IN = "in";
const char* PRECIPITATION_UNITS[MAX_PRECIPITATION_UNITS] = {
  PRECIPITATION_UNIT_MM, PRECIPITATION_UNIT_IN};
const char* PRECIPITATION_UNITS_LABELS[MAX_PRECIPITATION_UNITS] = {
  "Millimeter (mm)", "Inch (in)"};
char precipitationUnitSetting[MAX_PRECIPITATION_UNIT_LENGTH] = "mm";

const char* LANGUAGE_EN = "en";
const char* LANGUAGE_CN_SIMP = "cn_simp";
const char* LANGUAGE_CN_TRAD = "cn_trad";
const char* LANGUAGES[MAX_LANGUAGES] = {LANGUAGE_EN, LANGUAGE_CN_SIMP,
                                        LANGUAGE_CN_TRAD};
const char* LANGUAGES_LABELS[MAX_LANGUAGES] = {"English (en)",
                                               "Simplified Chinese (cn_simp)",
                                               "Traditional Chinese (cn_trad)"};
char languageSetting[MAX_LANGUAGE_LENGTH] = "en";

uint16_t updatePeriodSetting = 60;

bool sleepTimeEnabledSetting = true;
uint8_t sleepTimeStartHourSetting = 22;
uint8_t sleepTimeEndHourSetting = 4;

bool saveSettings() {
  Serial.println("Saving settings to preferences");
  Preferences preferences;
  preferences.begin("weatherStation");
  preferences.putString("cityOrZipCode", cityOrPostalCodeSetting);
  preferences.putString("tempUnit", tempUnitSetting);
  //  preferences.putString("windSpeedUnit", windSpeedUnitSetting);
  //  preferences.putString("precipUnit", precipitationUnitSetting);
  preferences.putString("lang", languageSetting);
  preferences.putUShort("updatePeriod", updatePeriodSetting);
  preferences.putBool("STEnabled", sleepTimeEnabledSetting);
  preferences.putUChar("STStartHour", sleepTimeStartHourSetting);
  preferences.putUChar("STEndHour", sleepTimeEndHourSetting);
  preferences.end();
  Serial.println("Settings saved");
  return true;
}

bool loadSettings() {
  Serial.println("Loading settings from preferences");
  Preferences preferences;
  preferences.begin("weatherStation");
  preferences.getString("cityOrZipCode", cityOrPostalCodeSetting,
                        MAX_CITY_OR_POSTAL_CODE_LENGTH);
  preferences.getString("tempUnit", tempUnitSetting, MAX_TEMP_UNIT_LENGTH);
  //  preferences.getString("windSpeedUnit", windSpeedUnitSetting,
  //                        MAX_WIND_SPEED_UNIT_LENGTH);
  //  preferences.getString("precipUnit", precipitationUnitSetting,
  //                        MAX_PRECIPITATION_UNIT_LENGTH);
  preferences.getString("lang", languageSetting, MAX_LANGUAGE_LENGTH);
  updatePeriodSetting = preferences.getUShort("updatePeriod", 60);
  sleepTimeEnabledSetting = preferences.getBool("STEnabled", true);
  sleepTimeStartHourSetting = preferences.getUChar("STStartHour", 22);
  sleepTimeEndHourSetting = preferences.getUChar("STEndHour", 4);
  preferences.end();
  Serial.println("Settings loaded");
  return true;
}

void printSettings() {
  Serial.println("Weather station settings:");
  Serial.printf("  City or postal code: %s\n", cityOrPostalCodeSetting);
  Serial.printf("  Temperature unit: %s\n", tempUnitSetting);
  //  Serial.printf("  Wind speed unit: %s\n", windSpeedUnitSetting);
  //  Serial.printf("  Precipitation unit: %s\n", precipitationUnitSetting);
  Serial.printf("  Language: %s\n", languageSetting);
  Serial.printf("  Sleep time enabled: %s\n",
                sleepTimeEnabledSetting ? "true" : "false");
  Serial.printf("  Sleep time start hour: %d\n", sleepTimeStartHourSetting);
  Serial.printf("  Sleep time end hour: %d\n", sleepTimeEndHourSetting);
}
