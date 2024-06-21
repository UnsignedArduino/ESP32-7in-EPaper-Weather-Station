#include "Display.h"
#include "Geocoding.h"
#include "Settings.h"
#include "Weather.h"
#include "WiFiConnection.h"
#include "config.h"
#include "pins.h"
#include "time.h"
#include "unifont_custom.h"
#include <Arduino.h>
#include <Button.h>
#include <LittleFS.h>
#include <WiFi.h>
#include <driver/rtc_io.h>

// TODO: Add better error handling
// TODO: Add battery level and warnings
// TODO: Add icons to some text

Button functionBtn(FUNCTION_BTN_PIN);

RTC_DATA_ATTR bool lastUpdateSuccess = false;

bool accurateTime = false;

bool updateTime(int32_t utcOffset, time_t estimate) {
  accurateTime = false;
  Serial.printf("Configuring time with UTC offset %+d\n", utcOffset / 3600);
  configTime(utcOffset, 0, NTP_SERVER_1, NTP_SERVER_2, NTP_SERVER_3);
  struct tm timeInfo {};
  for (uint8_t i = 0; i < 3; i++) {
    Serial.printf("Attempt %d of 3 to sync time\n", i + 1);
    if (!getLocalTime(&timeInfo)) {
      Serial.println("Failed to sync time");
      delay(1000);
    } else {
      accurateTime = true;
      Serial.println("Successfully synced time");
      break;
    }
  }
  if (accurateTime) {
    Serial.println("Time synced successfully");
    time_t now;
    time(&now);
    setTime(now + utcOffset);
    Serial.print("Time is ");
    Serial.println(&timeInfo, "%A, %B %d %Y %H:%M:%S");
  } else {
    Serial.println("Failed to sync time, using estimate from Open-Meteo");
    setTime(estimate + utcOffset);
    Serial.printf("Estimate time is %s, %s %s %d %02d:%02d:%02d\n",
                  dayShortStr(weekday()), monthShortStr(month()),
                  dayShortStr(day()), year(), hour(), minute(), second());
  }
  return true;
}

char* formatTemp(char* buf, size_t bufSize, float temp) {
  if (strcmp(tempUnitSetting, TEMP_UNIT_CELSIUS) == 0) {
    snprintf(buf, bufSize, "%.0f°C", round(temp));
  } else {
    snprintf(buf, bufSize, "%.0f°F", round(temp));
  }
  return buf;
}

void displayWeather(GeocodeData& geoData, WeatherData& weatherData) {
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

  display.fillScreen(GxEPD_WHITE);

  char todayDateBuf[32];
  snprintf(todayDateBuf, sizeof(todayDateBuf), "%s, %s%s",
           weekdayNames[weekday()], monthNames[month()], dayNames[day()]);
  u8g2.setCursor(30, 46);
  u8g2.print(todayDateBuf);
  displayScaleArea(30, 30, u8g2.getUTF8Width(todayDateBuf), 16, 2);

  int16_t currLocX;
  char* locationParts[] = {geoData.name,   geoData.admin4, geoData.admin3,
                           geoData.admin2, geoData.admin1, geoData.country};
  uint16_t longestLength =
    u8g2.getUTF8Width(locationParts[0]) * 2; // scale of first part
  for (char* locationPart : locationParts) {
    const uint16_t length = u8g2.getUTF8Width(locationPart);
    if (length > longestLength) {
      longestLength = length;
    }
  }
  currLocX = 800 - 30 - longestLength;
  u8g2.setCursor(currLocX, 46);
  u8g2.print(geoData.name);
  displayScaleArea(currLocX, 30, u8g2.getUTF8Width(geoData.name), 16, 2);
  int16_t currLocY = 30 + 34 + 16;
  for (uint32_t i = 1; i < (sizeof(locationParts) / sizeof(locationParts[0]));
       i++) {
    if (strlen(locationParts[i]) > 0 &&
        strcasestr(locationParts[i], locationParts[i - 1]) == nullptr) {
      u8g2.setCursor(currLocX, currLocY);
      currLocY += 16;
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

  const uint16_t endY = 350 - 16;
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

  u8g2.setCursor(30, endY + 100 + 32);
  if (accurateTime) {
    u8g2.printf("Last updated %s%s at %02d:%02d", monthNames[month()],
                dayNames[day()], hour(), minute());
  } else {
    u8g2.printf("Last updated %s%s at ??:??", monthNames[month()],
                dayNames[day()]);
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

void fontTest() {
  display.fillScreen(GxEPD_WHITE);
  u8g2.setCursor(0, 0);
  u8g2.println("Hello, world!");
  const char* testString = " !\"#$%&'()*+,-./:;<=>?@[\\]^_`{|}~0123456789\n"
                           "ABCDEFGHIJKLMNOPQRSTUVWXYZ\n"
                           "abcdefghijklmnopqrstuvwxyz\n\n"
                           "一七三中九二云五份八六冰冻十四多大天小日"
                           "星晴月朗期未毛知粒细部雨雪雲雷雹雾霜霧\n"
                           "°";
  u8g2.println(testString);
  display.display(false);
}

void setup() {
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);

  // Weird deep sleep bug workaround
  delay(500);

  Serial.printf("Efuse MAC: 0x%012llX\n", ESP.getEfuseMac());

  functionBtn.begin();

  loadSettings();
  printSettings();

  displayBegin();
  Serial.println("Loading custom GNU Unifont for all languages");
  u8g2.setFont(unifont_custom);
  //  fontTest();

  if (!LittleFS.begin()) {
    Serial.println("Failed to mount file system");
  } else {
    Serial.printf("File system mounted successfully - used %d of %d kb\n",
                  LittleFS.usedBytes() / 1024, LittleFS.totalBytes() / 1024);
  }

  bool showBootup = true;

  printWakeupReason();
  if (esp_sleep_get_wakeup_cause() == ESP_SLEEP_WAKEUP_TIMER &&
      lastUpdateSuccess) {
    showBootup = false;
  }

  if (showBootup) {
    display.fillScreen(GxEPD_WHITE);
    u8g2.setCursor(30, 46);
    u8g2.print("Refreshing...");
    u8g2.setCursor(30, 450 + 16);
    u8g2.print("Weather data by Open-Meteo.com");
    display.display(false);
  }

#pragma clang diagnostic push
#pragma ide diagnostic ignored "Simplify"
  if (functionBtn.read() == Button::PRESSED) {
    Serial.println("Detected button press, hold for 3 seconds to reset");
    display.fillScreen(GxEPD_WHITE);
    u8g2.setCursor(30, 46);
    u8g2.print(
      "Hold the function button for 3 seconds to reset WiFi settings. ");
    u8g2.setCursor(30, 66);
    u8g2.print("Otherwise, release the button to continue as normal.");
    display.display(false);
    const uint32_t holdUntil = millis() + 3000;
    while (millis() < holdUntil) {
      if (functionBtn.read() == Button::PRESSED) {
        break;
      }
    }
    if (functionBtn.read() == Button::PRESSED) {
      Serial.println("Button held long enough, resetting WiFi settings");
      resetWiFiSettings();
      display.fillScreen(GxEPD_WHITE);
      u8g2.setCursor(30, 46);
      u8g2.print("WiFi settings reset. ");
      u8g2.setCursor(30, 66);
      u8g2.print("Starting configuration AP...");
      display.display(false);
    } else {
      Serial.println("Button released prematurely, continuing");
      display.fillScreen(GxEPD_WHITE);
      u8g2.setCursor(30, 46);
      u8g2.print("WiFi settings not reset. ");
      u8g2.setCursor(30, 66);
      u8g2.print("Refreshing...");
      u8g2.setCursor(30, 450 + 16);
      u8g2.print("Weather data by Open-Meteo.com");
      display.display(false);
    }
  }
#pragma clang diagnostic pop

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
      u8g2.setCursor(30, 46);
      u8g2.print("Successfully configured!");
      u8g2.setCursor(30, 66);
      u8g2.print("Refreshing...");
      u8g2.setCursor(30, 450 + 16);
      u8g2.print("Weather data by Open-Meteo.com");
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
    getWeather(geocodeData.latitude, geocodeData.longitude, weatherData);

    updateTime(weatherData.utcOffset, weatherData.currUnixTime);

    disconnectFromWiFi();

    timeFinishDataFetch = millis();

    displayWeather(geocodeData, weatherData);
    lastUpdateSuccess = true;
  } else {
    disconnectFromWiFi();
    lastUpdateSuccess = false;
  }

  displayEnd();

  const uint32_t timeFinishDisplay = millis();

  Serial.println("Timings:");
  Serial.printf("  WiFi connect finished at ms %u\n", timeFinishWiFiConnect);
  Serial.printf("  Data fetch finished at ms %u\n", timeFinishDataFetch);
  Serial.printf("  Display finished at ms %u\n", timeFinishDisplay);

  // Sometimes the LED_BUILTIN is pulled up, no clue why
  gpio_hold_en(GPIO_NUM_2);
  gpio_deep_sleep_hold_en();

  Serial.printf("Going to sleep for %d minutes\n", UPDATE_TIME);
  esp_sleep_enable_ext0_wakeup(FUNCTION_BTN_PIN, 0);
  esp_sleep_enable_timer_wakeup(UPDATE_TIME * 60 * 1000000ULL);
  esp_deep_sleep_start();
}

void loop() {}
