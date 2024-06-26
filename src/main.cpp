// For debugging
// #define NO_REFRESH_TEXT
// #define FORCE_LOW_BATTERY_ERROR
// #define FORCE_WIFI_CONNECTION_ERROR
// #define FORCE_GEOCODE_CONNECTION_FAIL
// #define FORCE_WEATHER_CONNECTION_FAIL
// #define BLINK_ON_BATTERY_READ

#include "Display.h"
#include "Geocoding.h"
#include "Settings.h"
#include "Weather.h"
#include "WiFiConnection.h"
#include "config.h"
#include "icons.h"
#include "pins.h"
#include "time.h"
#include "unifont_custom.h"
#include <Arduino.h>
#include <Button.h>
#include <LittleFS.h>
#include <driver/rtc_io.h>
#include <hulp_arduino.h>

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

uint8_t batteryPercent = 0;
RTC_DATA_ATTR bool measuringBattThisTime = true;
RTC_SLOW_ATTR ulp_var_t ulpBatteryMilliVolt; // uint16_t

void updateBatteryState() {
#ifdef BLINK_ON_BATTERY_READ
  digitalWrite(LED_BUILTIN, HIGH);
  delay(100);
  digitalWrite(LED_BUILTIN, LOW);
  delay(100);
#endif
  // If the battery voltage is 0, then we must go to deep sleep to read the
  // battery voltage this boot. Then the ULP can get an accurate reading. After
  // the ULP wakes the main processor up, we use that.
  if (measuringBattThisTime) {
    measuringBattThisTime = false;
    Serial.println("Loading ULP program to read battery voltage");
    hulp_configure_analog_pin(BATTERY_PIN, ADC_ATTEN_DB_11, ADC_WIDTH_BIT_12);
    const ulp_insn_t program[] = {
      M_DELAY_MS_20_1000(500),
      I_STAGE_RST(),
      I_MOVI(R0, 0),
      I_MOVI(R3, 0),
      I_ADC_POWER_ON(),
      I_ANALOG_READ(R0, BATTERY_PIN),
      I_PUT(R0, R3, ulpBatteryMilliVolt),
      I_ADC_POWER_OFF(),
      I_HALT(),
    };
    hulp_peripherals_on();
    hulp_ulp_load(program, sizeof(program), 0, 0);
    hulp_ulp_run_once(0);
    Serial.println(
      "Going to deep sleep for 1 second to let ULP program run once");
    esp_sleep_enable_timer_wakeup(1000000ULL); // 1 second
    esp_deep_sleep_start();
  }

  Serial.println("Reading battery voltage set by ULP");
  ulpBatteryMilliVolt.val =
    map(ulpBatteryMilliVolt.val, 0, 4096, 0, 3300) * 2; // Voltage divider
  Serial.printf("Battery voltage: %d mV\n", ulpBatteryMilliVolt.val);

  // Although 3 AA batteries are good till 3v, we need to stop at 3.8v because
  // of the LDO regulator max drop out of 0.5v. Most batteries start higher
  // than 1.5v, so we can use 5v as the max.
  batteryPercent =
    constrain(map(ulpBatteryMilliVolt.val, 3900, 5000, 0, 100), 0, 100);
  Serial.printf("Battery percentage: %d%%\n", batteryPercent);

  measuringBattThisTime = true;
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
  u8g2.setCursor(30, 46 - 2);
  u8g2.print(todayDateBuf);
  displayScaleArea(30, 30, u8g2.getUTF8Width(todayDateBuf), 16, 2);

  char lastUpdatedBuf[48];
  if (accurateTime) {
    snprintf(lastUpdatedBuf, 48, "Last updated at %02d:%02d", hour(), minute());
  } else {
    snprintf(lastUpdatedBuf, 48, "Last updated  at ??:??");
  }

  char batteryBuf[32];
  snprintf(batteryBuf, sizeof(batteryBuf), "Battery at %d%%", batteryPercent);

  int16_t currLocX;
  const char* parts[] = {geoData.name,   geoData.admin4, geoData.admin3,
                         geoData.admin2, geoData.admin1, geoData.country,
                         "\n",           lastUpdatedBuf, batteryBuf};
  uint16_t longestLength =
    (u8g2.getUTF8Width(parts[0])) * 2; // scale of first part
  for (const char* locationPart : parts) {
    const uint16_t length = u8g2.getUTF8Width(locationPart);
    if (length > longestLength) {
      longestLength = length;
    }
  }
  currLocX = 800 - 30 - longestLength;
  u8g2.setCursor(currLocX, 46 - 2);
  u8g2.print(parts[0]);
  displayScaleArea(currLocX, 30, u8g2.getUTF8Width(geoData.name), 16, 2);
  int16_t currLocY = 30 + 34 + 16 - 2;
  for (uint32_t i = 1; i < (sizeof(parts) / sizeof(parts[0])); i++) {
    if (strlen(parts[i]) > 0 && strcasestr(parts[i], parts[i - 1]) == nullptr) {
      u8g2.setCursor(currLocX, currLocY);
      currLocY += 16;
      u8g2.print(parts[i]);
    }
  }

  // Today's date ends at y level 44
  // The bitmap is 100x100
  // The forecast starts at y level
  // (202 + 44) / 2 - (100 / 2) =

  const uint16_t y = 73;

  displayBitmap(
    WMOCodeToFilename(weatherData.currWeatherCode, weatherData.currIsDay), 30,
    y);

  u8g2.setCursor(150, y + 16);
  const char* currWeatherLabel = WMOCodeToLabel(weatherData.currWeatherCode);
  u8g2.print(currWeatherLabel);
  displayScaleArea(150, y, u8g2.getUTF8Width(currWeatherLabel), 16, 2);

  char currTempBuf[24];
  u8g2.setCursor(150, y + 34 + 16);
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
  displayScaleArea(150, y + 34, u8g2.getUTF8Width(currTempBuf), 16, 2);

  char currHumidBuf[8];
  snprintf(currHumidBuf, sizeof(currHumidBuf), "%.0f%%",
           round(weatherData.currHumidity));
  u8g2.setCursor(150, y + 34 * 2 + 16);
  u8g2.print(currHumidBuf);
  displayScaleArea(150, y + 34 * 2, u8g2.getUTF8Width(currHumidBuf), 16, 2);

  const uint16_t endY = 350 - 16 - 2;
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

void fontTest() {
  display.fillScreen(GxEPD_WHITE);
  u8g2.setCursor(0, 0);
  u8g2.println("Hello, world!");
  u8g2.println();
  u8g2.println("Font test: ");
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

  delay(500);

  Serial.println("ESP32 Weather Station");
  Serial.printf("Efuse MAC: 0x%012llX\n", ESP.getEfuseMac());

  updateBatteryState();
  delay(50);
  displayEnablePower(); // Give the E-paper chip time to initialize
  delay(50);

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

#ifdef FORCE_LOW_BATTERY_ERROR
  {
#else
  if (batteryPercent < 5) {
#endif
    Serial.println("Battery is too low! Going to permanent deep sleep.");
    display.fillScreen(GxEPD_WHITE);
    displayBitmap("/bootstrap-icons-1.11.3/battery-100x100.bmp", 30, 30);
    u8g2.setCursor(160, 46 - 2);
    u8g2.print("Battery is low!");
    u8g2.setCursor(160, 66 - 2);
    u8g2.print("Please replace the batteries.");
    display.display(false);
    displayEnd();
    esp_deep_sleep_start();
#ifdef FORCE_LOW_BATTERY_ERROR
    return;
#endif
  }

  bool showBootup = true;

#ifdef NO_REFRESH_TEXT
  showBootup = false;
#endif

  printWakeupReason();
  if (esp_sleep_get_wakeup_cause() == ESP_SLEEP_WAKEUP_TIMER &&
      lastUpdateSuccess) {
    showBootup = false;
  }

  if (showBootup) {
    display.fillScreen(GxEPD_WHITE);
    displayBitmap("/bootstrap-icons-1.11.3/arrow-repeat-100x100.bmp", 30, 30);
    u8g2.setCursor(160, 46 - 2);
    u8g2.print("Refreshing...");
    u8g2.setCursor(30, 450 - 2);
    u8g2.print("Weather data by Open-Meteo.com");
    display.display(false);
  }

#pragma clang diagnostic push
#pragma ide diagnostic ignored "Simplify"
  if (functionBtn.read() == Button::PRESSED) {
    Serial.println("Detected button press, hold for 3 seconds to reset");
    display.fillScreen(GxEPD_WHITE);
    displayBitmap("/bootstrap-icons-1.11.3/trash-100x100.bmp", 30, 30);
    u8g2.setCursor(160, 46 - 2);
    u8g2.print(
      "Hold the function button for 3 seconds to reset WiFi settings. ");
    u8g2.setCursor(160, 66 - 2);
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
      displayBitmap("/bootstrap-icons-1.11.3/check-100x100.bmp", 30, 30);
      u8g2.setCursor(160, 46 - 2);
      u8g2.print("WiFi settings reset. ");
      u8g2.setCursor(160, 66 - 2);
      u8g2.print("Starting configuration AP...");
      display.display(false);
    } else {
      Serial.println("Button released prematurely, continuing");
      display.fillScreen(GxEPD_WHITE);
      displayBitmap("/bootstrap-icons-1.11.3/arrow-repeat-100x100.bmp", 30, 30);
      u8g2.setCursor(160, 46 - 2);
      u8g2.print("WiFi settings not reset. ");
      u8g2.setCursor(160, 66 - 2);
      u8g2.print("Refreshing...");
      u8g2.setCursor(30, 450 - 2);
      u8g2.print("Weather data by Open-Meteo.com");
      display.display(false);
    }
  }
#pragma clang diagnostic pop

#ifdef FORCE_WIFI_CONNECTION_ERROR
  const int8_t connectRes = WIFI_CONNECTION_ERROR;
#else
  const int8_t connectRes =
    connectToWiFi([](char* ssid, char* password, char* ip) {
      display.fillScreen(GxEPD_WHITE);
      displayBitmap("/bootstrap-icons-1.11.3/wifi-100x100.bmp", 30, 30);

      u8g2.setCursor(160, 46 - 2);
      u8g2.print("Configuration AP launched.");
      u8g2.setCursor(160, 66 - 2);
      u8g2.print(
        "Join the WiFi network in order to configure the weather station.");
      u8g2.setCursor(160, 106 - 2);

      u8g2.print("SSID: ");
      u8g2.print(ssid);
      u8g2.setCursor(160, 126 - 2);
      u8g2.print("Password: ");
      u8g2.print(password);
      u8g2.setCursor(160, 146 - 2);
      u8g2.print("IP: ");
      u8g2.print(ip);
      u8g2.print(
        " (may pop up automatically or you may be asked to \"sign in\")");

      u8g2.setCursor(160, 186 - 2);
      u8g2.print("Once on the page with titled \"WiFiManager\", hit the "
                 "\"Configure WiFi\" button.");
      u8g2.setCursor(160, 206 - 2);
      u8g2.print("Select a WiFi network and type in the password. Change the "
                 "other options to ");
      u8g2.setCursor(160, 226 - 2);
      u8g2.print(
        "configure to your liking. Then hit the \"Save\" button. You will be ");
      u8g2.setCursor(160, 246 - 2);
      u8g2.print("disconnected from the configuration WiFi network.");

      display.display(false);
    });
#endif
  bool showWeather = true;
  switch (connectRes) {
    default:
    case WIFI_CONNECTION_SUCCESS:
      break;
    case WIFI_CONNECTION_SUCCESS_CONFIG: {
      display.fillScreen(GxEPD_WHITE);
      displayBitmap("/bootstrap-icons-1.11.3/check-100x100.bmp", 30, 30);
      u8g2.setCursor(160, 46 - 2);
      u8g2.print("Successfully configured!");
      u8g2.setCursor(160, 66 - 2);
      u8g2.print("Refreshing...");
      u8g2.setCursor(30, 450 - 2);
      u8g2.print("Weather data by Open-Meteo.com");
      display.display(false);
      break;
    }
    case WIFI_CONNECTION_ERROR: {
      display.fillScreen(GxEPD_WHITE);
      displayBitmap("/bootstrap-icons-1.11.3/wifi-off-100x100.bmp", 30, 30);
      u8g2.setCursor(160, 46 - 2);
      u8g2.print("Failed to connect to WiFi!");
      u8g2.setCursor(160, 66 - 2);
      u8g2.printf("Retrying in %d minutes - Hold the function button for 3 "
                  "seconds to restart ",
                  RETRY_TIME);
      u8g2.setCursor(160, 86 - 2);
      u8g2.print("the WiFi configuration process now.");
      display.display(false);
      showWeather = false;
      break;
    }
    case WIFI_CONNECTION_ERROR_TIMEOUT: {
      display.fillScreen(GxEPD_WHITE);
      displayBitmap("/bootstrap-icons-1.11.3/wifi-off-100x100.bmp", 30, 30);
      u8g2.setCursor(160, 46 - 2);
      u8g2.print("WiFi timed out!");
      u8g2.setCursor(160, 66 - 2);
      u8g2.printf("Retrying in %d minutes - Hold the function button for 3 "
                  "seconds to restart ",
                  RETRY_TIME);
      u8g2.setCursor(160, 86 - 2);
      u8g2.print("the WiFi configuration process now.");
      display.display(false);
      showWeather = false;
      break;
    }
  }

  const uint32_t timeFinishWiFiConnect = millis();

  GeocodeData geocodeData{};
  if (showWeather) {
#ifdef FORCE_GEOCODE_CONNECTION_FAIL
    const int8_t geocodeResult = GET_COORDINATE_CONNECTION_FAIL;
#else
    const int8_t geocodeResult =
      getGeocode(cityOrPostalCodeSetting, geocodeData);
#endif

    if (!(geocodeResult == GET_COORDINATE_SUCCESS ||
          geocodeResult == GET_COORDINATE_SUCCESS_CACHE)) {
      showWeather = false;
      display.fillScreen(GxEPD_WHITE);
      displayBitmap("/bootstrap-icons-1.11.3/exclamation-100x100.bmp", 30, 30);
      u8g2.setCursor(160, 46 - 2);
      switch (geocodeResult) { // NOLINT(*-multiway-paths-covered)
        case GET_COORDINATE_CONNECTION_FAIL: {
          u8g2.print("Failed to connect to the geocoding API!");
          break;
        }
        case GET_COORDINATE_CONNECTION_TIMEOUT: {
          u8g2.print("Geocoding API timed out!");
          break;
        }
        case GET_COORDINATE_PARSE_FAIL: {
          u8g2.print("Failed to parse geocoding API response!");
          break;
        }
      }
      u8g2.setCursor(160, 66 - 2);
      u8g2.printf(
        "Retrying in %d minutes - Press the function button to try again now.",
        RETRY_TIME);
      display.display(false);
    }
  }

  WeatherData weatherData{};
  if (showWeather) {
#ifdef FORCE_WEATHER_CONNECTION_FAIL
    const int8_t weatherResult = GET_WEATHER_CONNECTION_FAIL;
#else
    const int8_t weatherResult =
      getWeather(geocodeData.latitude, geocodeData.longitude, weatherData);
#endif

    if (weatherResult != GET_WEATHER_SUCCESS) {
      showWeather = false;
      display.fillScreen(GxEPD_WHITE);
      displayBitmap("/bootstrap-icons-1.11.3/exclamation-100x100.bmp", 30, 30);
      u8g2.setCursor(160, 46 - 2);
      switch (weatherResult) { // NOLINT(*-multiway-paths-covered)
        case GET_WEATHER_CONNECTION_FAIL: {
          u8g2.print("Failed to connect to the weather API!");
          break;
        }
        case GET_WEATHER_CONNECTION_TIMEOUT: {
          u8g2.print("Weather API timed out!");
          break;
        }
        case GET_WEATHER_PARSE_FAIL: {
          u8g2.print("Failed to parse weather API response!");
          break;
        }
      }
      u8g2.setCursor(160, 66 - 2);
      u8g2.printf(
        "Retrying in %d minutes - Press the function button to try again now.",
        RETRY_TIME);
      display.display(false);
    }

    updateTime(weatherData.utcOffset, weatherData.currUnixTime);
  }

  disconnectFromWiFi();

  const uint32_t timeFinishDataFetch = millis();

  if (showWeather) {
    displayWeather(geocodeData, weatherData);
    lastUpdateSuccess = true;
  }

  displayEnd();
  displayDisablePower();

  const uint32_t timeFinishDisplay = millis();

  Serial.println("Timings:");
  Serial.printf("  WiFi connect finished at ms %u\n", timeFinishWiFiConnect);
  Serial.printf("  Data fetch finished at ms %u\n", timeFinishDataFetch);
  Serial.printf("  Display finished at ms %u\n", timeFinishDisplay);

  // Sometimes the LED_BUILTIN is pulled up, no clue why
  gpio_hold_en(GPIO_NUM_2);
  gpio_deep_sleep_hold_en();

  esp_sleep_enable_ext0_wakeup(FUNCTION_BTN_PIN, 0);
  Serial.printf("Going to sleep for %d minutes\n",
                (lastUpdateSuccess ? UPDATE_TIME : RETRY_TIME));
  esp_sleep_enable_timer_wakeup((lastUpdateSuccess ? UPDATE_TIME : RETRY_TIME) *
                                60 * 1000000ULL);
  esp_deep_sleep_start();
}

void loop() {}
