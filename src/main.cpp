#include "Geocoding.h"
#include "Settings.h"
#include "Touch.h"
#include "Weather.h"
#include "WiFiConnection.h"
#include "pins.h"
#include <Arduino.h>

void setup() {
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);

  Serial.printf("Efuse MAC: 0x%012llX\n", ESP.getEfuseMac());

  if (isTouched()) {
    Serial.println("Touch detected, resetting WiFi settings");
    resetWiFiSettings();
  }

  loadSettings();
  printSettings();
  connectToWiFi();

  float latitude, longitude;
  char name[MAX_NAME_SIZE], country[MAX_NAME_SIZE], admin1[MAX_NAME_SIZE],
      admin2[MAX_NAME_SIZE], admin3[MAX_NAME_SIZE], admin4[MAX_NAME_SIZE];
  getGeocode(cityOrPostalCodeSetting, latitude, longitude, name, country,
             admin1, admin2, admin3, admin4);

  WeatherData data;
  int8_t result = getWeather(latitude, longitude, data);
}

void loop() {}
