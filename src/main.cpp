#include "Geocoding.h"
#include "Settings.h"
#include "Touch.h"
#include "Weather.h"
#include "WiFiConnection.h"
#include "pins.h"
#include <Adafruit_EPD.h>
#include <Adafruit_GFX.h>
#include <Arduino.h>
#include <Fonts/FreeMono9pt7b.h>
#include <Fonts/FreeMonoBold9pt7b.h>
#include <GxEPD2_BW.h>
#include <GxEPD2_display_selection_new_style.h>

void displayBegin() {
  display.init(115200, true, 2, false);
  display.setRotation(0);
  display.setFont(&FreeMono9pt7b);
  display.setTextColor(GxEPD_BLACK);
  display.setFullWindow();
  display.fillScreen(GxEPD_WHITE);
}

void displayEnd() {
  display.hibernate();
}

void setup() {
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);

  displayBegin();

  // Weird deep sleep bug workaround
  delay(500);

  Serial.printf("Efuse MAC: 0x%012llX\n", ESP.getEfuseMac());

  if (isTouched()) {
    Serial.println("Touch detected, resetting WiFi settings");
    resetWiFiSettings();
  }

  loadSettings();
  printSettings();
  connectToWiFi();

  const uint32_t timeFinishWiFiConnect = millis();

  GeocodeData geocodeData;
  getGeocode(cityOrPostalCodeSetting, geocodeData);

  WeatherData weatherData;
  int8_t result =
      getWeather(geocodeData.latitude, geocodeData.longitude, weatherData);

  const uint32_t timeFinishDataFetch = millis();

  display.fillScreen(GxEPD_WHITE);
  display.display(false);

  displayEnd();

  const uint32_t timeFinishDisplay = millis();

  Serial.println("Timings:");
  Serial.printf("  WiFi connect finished at ms %lu\n", timeFinishWiFiConnect);
  Serial.printf("  Data fetch finished at ms %lu\n", timeFinishDataFetch);
  Serial.printf("  Display finished at ms %lu\n", timeFinishDisplay);
}

void loop() {}
