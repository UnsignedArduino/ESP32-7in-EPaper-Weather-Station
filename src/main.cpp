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
#include <U8g2_for_Adafruit_GFX.h>

U8G2_FOR_ADAFRUIT_GFX u8g2;

void displayBegin() {
  display.init(115200, true, 2, false);
  u8g2.begin(display);
  display.setRotation(0);
  display.setFont(&FreeMono9pt7b);
  display.setFullWindow();
  display.fillScreen(GxEPD_WHITE);

  u8g2.setFontMode(1);
  u8g2.setFontDirection(0);
  u8g2.setBackgroundColor(GxEPD_WHITE);
  u8g2.setForegroundColor(GxEPD_BLACK);
}

void displayEnd() {
  display.hibernate();
}

void displayWeather(GeocodeData& geoData, WeatherData& weatherData) {
  display.fillScreen(GxEPD_WHITE);

  u8g2.setFont(u8g2_font_9x18_mf);

  u8g2.setCursor(0, 10);
  u8g2.print(geoData.name);
  char* locationParts[] = {geoData.name,   geoData.admin4, geoData.admin3,
                           geoData.admin2, geoData.admin1, geoData.country};
  for (uint8_t i = 1; i < (sizeof(locationParts) / sizeof(locationParts[0]));
       i++) {
    if (strlen(locationParts[i]) > 0 &&
        strcasestr(locationParts[i], locationParts[i - 1]) == NULL) {
      u8g2.print(", ");
      u8g2.print(locationParts[i]);
    }
  }

  display.display(false);
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

  displayWeather(geocodeData, weatherData);

  displayEnd();

  const uint32_t timeFinishDisplay = millis();

  Serial.println("Timings:");
  Serial.printf("  WiFi connect finished at ms %lu\n", timeFinishWiFiConnect);
  Serial.printf("  Data fetch finished at ms %lu\n", timeFinishDataFetch);
  Serial.printf("  Display finished at ms %lu\n", timeFinishDisplay);
}

void loop() {}
