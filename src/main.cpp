#include "Display.h"
#include "Geocoding.h"
#include "Settings.h"
#include "Weather.h"
#include "WiFiConnection.h"
#include <Arduino.h>
#include <LittleFS.h>

// TODO: Fix traditional Chinese font missing characters
// TODO: Add deep sleep
// TODO: Switch from touch button to physical button
// TODO: Add better error handling
// TODO: Add icons to some text
// TODO: Add battery level

char* formatTemp(char* buf, size_t bufSize, float temp) {
  if (strcmp(tempUnitSetting, TEMP_UNIT_CELSIUS) == 0) {
    snprintf(buf, bufSize, "%.0f°C", round(temp));
  } else {
    snprintf(buf, bufSize, "%.0f°F", round(temp));
  }
  return buf;
}

void displayWeather(GeocodeData& geoData, WeatherData& weatherData) {
  display.fillScreen(GxEPD_WHITE);

  u8g2.setCursor(30, 46);
  u8g2.print(geoData.name);
  char* locationParts[] = {geoData.name,   geoData.admin4, geoData.admin3,
                           geoData.admin2, geoData.admin1, geoData.country};
  for (uint32_t i = 1; i < (sizeof(locationParts) / sizeof(locationParts[0]));
       i++) {
    if (strlen(locationParts[i]) > 0 &&
        strcasestr(locationParts[i], locationParts[i - 1]) == nullptr) {
      u8g2.print(", ");
      u8g2.print(locationParts[i]);
    }
  }

  displayBitmap(
    WMOCodeToFilename(weatherData.currWeatherCode, weatherData.currIsDay), 30,
    80);

  u8g2.setCursor(150, 80 + 16);
  const char* currWeatherLabel = WMOCodeToLabel(weatherData.currWeatherCode);
  u8g2.print(currWeatherLabel);
  displayScaleArea(150, 80, u8g2.getUTF8Width(currWeatherLabel), 16, 2);

  char currTempBuf[24];
  u8g2.setCursor(150, 80 + 34 + 16);
  if (strcmp(tempUnitSetting, TEMP_UNIT_CELSIUS) == 0) {
    snprintf(currTempBuf, sizeof(currTempBuf), "%.0f°C (%.0f°C - %.0f°C)",
             round(weatherData.currTemp), round(weatherData.currLowTemp),
             round(weatherData.currHighTemp));
  } else {
    snprintf(currTempBuf, sizeof(currTempBuf), "%.0f°F (%.0f°F - %.0f°F)",
             round(weatherData.currTemp), round(weatherData.currLowTemp),
             round(weatherData.currHighTemp));
  }
  u8g2.print(currTempBuf);
  displayScaleArea(150, 80 + 34, u8g2.getUTF8Width(currTempBuf), 16, 2);

  char currHumidBuf[8];
  snprintf(currHumidBuf, sizeof(currHumidBuf), "%.0f%%",
           round(weatherData.currHumidity));
  u8g2.setCursor(150, 80 + 34 * 2 + 16);
  u8g2.print(currHumidBuf);
  displayScaleArea(150, 80 + 34 * 2, u8g2.getUTF8Width(currHumidBuf), 16, 2);

  const char** weekdayNames;
  const char** monthNames;
  const char** dayNames;
  if (strcmp(languageSetting, LANGUAGE_EN) == 0) {
    weekdayNames = WEEKDAY_NAMES_EN;
    monthNames = MONTH_NAMES_EN;
    dayNames = DAY_NAMES_EN;
  } else if (strcmp(languageSetting, LANGUAGE_CN_TRAD) == 0) {
    weekdayNames = WEEKDAY_NAMES_CN_TRAD;
    monthNames = MONTH_NAMES_CN_TRAD;
    dayNames = DAY_NAMES_CN_TRAD;
  } else {
    weekdayNames = WEEKDAY_NAMES_CN_SIMP;
    monthNames = MONTH_NAMES_CN_SIMP;
    dayNames = DAY_NAMES_CN_SIMP;
  }
  const uint16_t endY = 350;
  for (uint8_t i = 1; i < MAX_FORECAST_DAYS - 1; i++) {
    const uint16_t x = 160 * (i - 1);
    const uint16_t centerX = x + 80;

    const time_t time = weatherData.forecastUnixTimes[i];

    const char* dayName = weekdayNames[weekday(time)];
    u8g2.setCursor(cursorXFromCenter(dayName, centerX, 2),
                   endY - 34 * 4 + 16 - 10);
    u8g2.print(dayName);
    displayScaleArea(cursorXFromCenter(dayName, centerX, 2), endY - 34 * 4 - 10,
                     u8g2.getUTF8Width(dayName), 16, 2);

    char dateBuf[16];
    snprintf(dateBuf, sizeof(dateBuf), "%s%s", monthNames[month(time)],
             dayNames[day(time)]);
    u8g2.setCursor(cursorXFromCenter(dateBuf, centerX, 2),
                   endY - 34 * 3 + 16 - 10);
    u8g2.print(dateBuf);
    displayScaleArea(cursorXFromCenter(dateBuf, centerX, 2), endY - 34 * 3 - 10,
                     u8g2.getUTF8Width(dateBuf), 16, 2);

    char tempsBuf[8];
    formatTemp(tempsBuf, sizeof(tempsBuf), weatherData.forecastLowTemps[i]);
    u8g2.setCursor(cursorXFromCenter(tempsBuf, centerX, 2),
                   endY - 34 * 2 + 16 - 10);
    u8g2.print(tempsBuf);
    displayScaleArea(cursorXFromCenter(tempsBuf, centerX, 2),
                     endY - 34 * 2 - 10, u8g2.getUTF8Width(tempsBuf), 16, 2);
    formatTemp(tempsBuf, sizeof(tempsBuf), weatherData.forecastHighTemps[i]);
    u8g2.setCursor(cursorXFromCenter(tempsBuf, centerX, 2),
                   endY - 34 * 1 + 16 - 10);
    u8g2.print(tempsBuf);
    displayScaleArea(cursorXFromCenter(tempsBuf, centerX, 2),
                     endY - 34 * 1 - 10, u8g2.getUTF8Width(tempsBuf), 16, 2);

    displayBitmap(WMOCodeToFilename(weatherData.forecastWeatherCodes[i], true),
                  x + 30, endY);
  }

  display.display(false);
}

void printWakeupReason() {
  esp_sleep_wakeup_cause_t reason = esp_sleep_get_wakeup_cause();

  switch (reason) {
    case ESP_SLEEP_WAKEUP_EXT0:
      Serial.println("Wakeup caused by external signal using RTC_IO");
      break;
    case ESP_SLEEP_WAKEUP_EXT1:
      Serial.println("Wakeup caused by external signal using RTC_CNTL");
      break;
    case ESP_SLEEP_WAKEUP_TIMER:
      Serial.println("Wakeup caused by timer");
      break;
    case ESP_SLEEP_WAKEUP_TOUCHPAD:
      Serial.println("Wakeup caused by touch");
      break;
    case ESP_SLEEP_WAKEUP_ULP:
      Serial.println("Wakeup caused by ULP program");
      break;
    default:
      Serial.printf("Wakeup was not caused by deep sleep: %d\n", reason);
      break;
  }
}

void loadFontForCurrentLang() {
  if (strcmp(languageSetting, LANGUAGE_EN) == 0) {
    Serial.println("Using GNU Unifont (for EN)");
    u8g2.setFont(u8g2_font_unifont_tf);
  } else if (strcmp(languageSetting, LANGUAGE_CN_TRAD) == 0) {
    Serial.println("Using WenQuanYi Micro Hei (for CN_TRAD)");
    u8g2.setFont(u8g2_font_wqy16_t_gb2312);
  } else {
    Serial.println("Using WenQuanYi Micro Hei (for CN_SIMP)");
    u8g2.setFont(u8g2_font_wqy16_t_gb2312);
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);

  // Weird deep sleep bug workaround
  delay(500);

  Serial.printf("Efuse MAC: 0x%012llX\n", ESP.getEfuseMac());

  loadSettings();
  printSettings();

  displayBegin();
  loadFontForCurrentLang();

  display.fillScreen(GxEPD_WHITE);
  u8g2.setCursor(30, 46);
  u8g2.print("Refreshing...");
  display.display(false);

  if (!LittleFS.begin()) {
    Serial.println("Failed to mount file system");
  } else {
    Serial.printf("File system mounted successfully - used %d of %d kb\n",
                  LittleFS.usedBytes() / 1024, LittleFS.totalBytes() / 1024);
  }

  //  resetWiFiSettings();

  const int8_t connectRes =
    connectToWiFi([](char* ssid, char* password, char* ip) {
      display.fillScreen(GxEPD_WHITE);
      u8g2.setCursor(30, 46);
      u8g2.print("Configuration AP launched.");
      u8g2.setCursor(30, 66);
      u8g2.print(
        "Join the WiFi network in order to configure the weather station.");
      u8g2.setCursor(30, 106);

      u8g2.print("SSID: ");
      u8g2.print(ssid);
      u8g2.setCursor(30, 126);
      u8g2.print("Password: ");
      u8g2.print(password);
      u8g2.setCursor(30, 146);
      u8g2.print("IP: ");
      u8g2.print(ip);
      u8g2.print(
        " (may pop up automatically or you may be asked to \"sign in\")");

      u8g2.setCursor(30, 186);
      u8g2.print("Once on the page with titled \"WiFiManager\", hit the "
                 "\"Configure WiFi\" button.");
      u8g2.setCursor(30, 206);
      u8g2.print("Select a WiFi network and type in the password. Change the "
                 "other options to configure to ");
      u8g2.setCursor(30, 226);
      u8g2.print("your liking. Then hit the \"Save\" button. You will be "
                 "disconnected from the ");
      u8g2.setCursor(30, 246);
      u8g2.print("configuration WiFi network.");

      display.display(false);
    });
  bool showWeather = true;
  switch (connectRes) {
    default:
    case WIFI_CONNECTION_SUCCESS:
      break;
    case WIFI_CONNECTION_SUCCESS_CONFIG: {
      display.fillScreen(GxEPD_WHITE);
      // In case the user changed it
      loadFontForCurrentLang();
      u8g2.setCursor(30, 46);
      u8g2.print("Successfully configured!");
      u8g2.setCursor(30, 66);
      u8g2.print("Refreshing...");
      display.display(false);
      break;
    }
    case WIFI_CONNECTION_ERROR: {
      display.fillScreen(GxEPD_WHITE);
      u8g2.setCursor(30, 46);
      u8g2.print("Failed to connect to WiFi!");
      u8g2.setCursor(30, 66);
      u8g2.print("Hold the function button for 3 seconds to restart the WiFi "
                 "configuration process.");
      display.display(false);
      showWeather = false;
      break;
    }
    case WIFI_CONNECTION_ERROR_TIMEOUT: {
      display.fillScreen(GxEPD_WHITE);
      u8g2.setCursor(30, 46);
      u8g2.print("Configuration timed out!");
      u8g2.setCursor(30, 66);
      u8g2.print("Hold the function button for 3 seconds to restart the WiFi "
                 "configuration process.");
      display.display(false);
      showWeather = false;
      break;
    }
  }

  const uint32_t timeFinishWiFiConnect = millis();
  uint32_t timeFinishDataFetch = millis();

  if (showWeather) {
    GeocodeData geocodeData{};
    getGeocode(cityOrPostalCodeSetting, geocodeData);

    WeatherData weatherData{};
    int8_t result =
      getWeather(geocodeData.latitude, geocodeData.longitude, weatherData);

    disconnectFromWiFi();

    timeFinishDataFetch = millis();

    displayWeather(geocodeData, weatherData);
  } else {
    disconnectFromWiFi();
  }

  displayEnd();

  const uint32_t timeFinishDisplay = millis();

  Serial.println("Timings:");
  Serial.printf("  WiFi connect finished at ms %u\n", timeFinishWiFiConnect);
  Serial.printf("  Data fetch finished at ms %u\n", timeFinishDataFetch);
  Serial.printf("  Display finished at ms %u\n", timeFinishDisplay);
}

void loop() {}
