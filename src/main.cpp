// For debugging
// #define NO_REFRESH_TEXT
// #define FORCE_LOW_BATTERY_ERROR
// #define FORCE_WIFI_CONNECTION_ERROR
// #define FORCE_GEOCODE_CONNECTION_FAIL
// #define FORCE_WEATHER_CONNECTION_FAIL
// #define BLINK_ON_BATTERY_READ
// #define FORCE_FAKE_WIFI_CONFIGURATION_SCREEN

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
#include <BlynkSimpleEsp32.h>
#include <Button.h>
#include <LittleFS.h>
#include <driver/rtc_io.h>
#include <qrcode.h>

Button functionBtn(FUNCTION_BTN_PIN);

RTC_DATA_ATTR bool lastUpdateSuccess = false;
RTC_DATA_ATTR uint32_t successUpdates = 0;
RTC_DATA_ATTR uint32_t failedUpdates = 0;

uint8_t batteryPercent = 0;

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

RTC_DATA_ATTR float batteryVoltage = 0.0;

void updateBatteryState() {
#ifdef BLINK_ON_BATTERY_READ
  digitalWrite(LED_BUILTIN, HIGH);
  delay(100);
  digitalWrite(LED_BUILTIN, LOW);
  delay(100);
#endif
  // https://forum.arduino.cc/t/battery-percentage-correction/485341/30
  // IIR / Single pole filter
  // DSP is hard
  const float x = 0.9;

  if (batteryVoltage < 1) {
    Serial.println("First battery reading, using raw value");
    delay(500);
    batteryVoltage = (float)analogReadMilliVolts(BATTERY_PIN) * 2;
  }

  Serial.println("Updating battery voltage with single pole filter");
  Serial.printf("Battery pack voltage before filter input: %f mV\n",
                batteryVoltage);

  delay(500);
  uint16_t pack = analogReadMilliVolts(BATTERY_PIN) * 2;
  batteryVoltage = batteryVoltage * x + (float)pack * (1 - x);
  delay(10);

  Serial.printf("Battery pack voltage after filter input: %f mV\n",
                batteryVoltage);

  batteryPercent = constrain(map(batteryVoltage, 3800, 4800, 0, 100), 0, 100);
  Serial.printf("Battery percentage: %d%%\n", batteryPercent);
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
    Serial.println("Using english for weather display");
    weekdayNames = WEEKDAY_NAMES_EN;
    monthNames = MONTH_NAMES_EN;
    dayNames = DAY_NAMES_EN;
  } else if (strcmp(languageSetting, LANGUAGE_CN_TRAD) == 0) {
    Serial.println("Using traditional Chinese for weather display");
    weekdayNames = WEEKDAY_NAMES_CN_TRAD;
    monthNames = MONTH_NAMES_CN_TRAD;
    dayNames = DAY_NAMES_CN_TRAD;
  } else {
    Serial.println("Using simplified Chinese for weather display");
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
    snprintf(lastUpdatedBuf, 48, "Last updated at ??:??");
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
  updateBatteryState();
  delay(50);
  displayEnablePower(); // Give the chip time to initialize
  delay(50);

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

#ifdef FORCE_LOW_BATTERY_ERROR
  {
#else
  if (batteryPercent < 5) {
#endif
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
  #ifdef FORCE_FAKE_WIFI_CONFIGURATION_SCREEN
  Serial.println("Forcing fake WiFi configuration screen for testing");

  char ssid[32];
  char password[64];
  snprintf(ssid, sizeof(ssid), "Weather Station %04X",
           (uint16_t)(ESP.getEfuseMac() & 0xFFFF));
  snprintf(password, sizeof(password), "%012llX", ESP.getEfuseMac());

  const char* ip = "192.168.1.4";
  #else
  const int8_t connectRes = connectToWiFi([](char* ssid, char* password,
                                             char* ip) {
  #endif
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
  u8g2.print(" (may pop up automatically or you may be asked to \"sign in\")");

  u8g2.setCursor(160, 186 - 2);
  u8g2.print("Or scan the QR code below to connect to the configuration AP: ");

  QRCode qrcode;
  // QR code format is
  // WIFI:S:<SSID>;T:<WEP|WPA|nopass>;P:<PASSWORD>;H:<true|false|blank>;;
  // https://en.wikipedia.org/wiki/QR_code#Joining_a_Wi%E2%80%91Fi_network
  char qrCodeString[128];
  const uint8_t version = 4;
  uint8_t qrCodeData[qrcode_getBufferSize(version)];
  snprintf(qrCodeString, sizeof(qrCodeString), "WIFI:S:%s;T:WPA;P:%s;H:false;;",
           ssid, password);
  Serial.printf("QR code data: %s (%d chars)\n", qrCodeString,
                strlen(qrCodeString));
  qrcode_initText(&qrcode, qrCodeData, version, ECC_LOW, qrCodeString);
  Serial.printf("Generated QR code for WiFi network with size %dx%d\n",
                qrcode.size, qrcode.size);
  const uint8_t BLOCK_SIZE = 4;
  for (uint8_t y = 0; y < qrcode.size; y++) {
    for (uint8_t x = 0; x < qrcode.size; x++) {
      if (qrcode_getModule(&qrcode, x, y)) {
        display.fillRect(160 + x * BLOCK_SIZE, 204 + y * BLOCK_SIZE, BLOCK_SIZE,
                         BLOCK_SIZE, GxEPD_BLACK);
      }
    }
  }
  uint16_t nextY = 206 - 2 + qrcode.size * BLOCK_SIZE + 20 * 2 + 2;

  u8g2.setCursor(160, nextY - 2);
  u8g2.print("Once on the page with titled \"WiFiManager\", hit the "
             "\"Configure WiFi\" button.");
  u8g2.setCursor(160, nextY + 20 - 2);
  u8g2.print("Select a WiFi network and type in the password. Change the "
             "other options to ");
  u8g2.setCursor(160, nextY + 20 * 2 - 2);
  u8g2.print(
    "configure to your liking. Then hit the \"Save\" button. You will be ");
  u8g2.setCursor(160, nextY + 20 * 3 - 2);
  u8g2.print("disconnected from the configuration WiFi network.");

  display.display(false);
  #ifdef FORCE_FAKE_WIFI_CONFIGURATION_SCREEN

  displayEnd();
  displayDisablePower();

  const int8_t connectRes = WIFI_CONNECTION_SUCCESS_CONFIG;

  return;
  #else
  });
  #endif
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

  if (showWeather) {
    Serial.println("Successful update!");
    successUpdates++;
  } else {
    Serial.println("Failed update!");
    failedUpdates++;
  }
  Serial.printf("Successful updates: %d\nFailed updates: %d\n", successUpdates,
                failedUpdates);

  Serial.println("Starting Blynk");
  Blynk.config(BLYNK_AUTH_TOKEN);
  if (Blynk.connect()) {
    Serial.println("Connected to Blynk, sending telemetry!");

    Serial.printf("V0 = %d (battery percent)\n", batteryPercent);
    Blynk.virtualWrite(V0, batteryPercent);

    char tempBuf[32];
    snprintf(tempBuf, sizeof(tempBuf), "%d / %d", successUpdates,
             failedUpdates);
    Serial.printf("V1 = %s (success / failed updates)\n", tempBuf);
    Blynk.virtualWrite(V1, tempBuf);

    Blynk.run();
    Serial.println("Disconnecting from Blynk");
    Blynk.disconnect();
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

  uint32_t targetSleepTime =
    lastUpdateSuccess ? updatePeriodSetting : RETRY_TIME; // minutes
  if (sleepTimeEnabledSetting) {
    if (!lastUpdateSuccess) {
      Serial.println("Although sleep time is enabled, last update was not a "
                     "success, so not sleeping for extended time");
    } else {
      // I'm too lazy to figure out the logic for this
      // Keep adding to the sleep time until it's outside the sleep time range
      while (hour(now() + targetSleepTime * 60) >= sleepTimeStartHourSetting ||
             hour(now() + targetSleepTime * 60) < sleepTimeEndHourSetting) {
        targetSleepTime += 1;
      }
      Serial.printf("Sleep time is enabled, sleeping for %d minutes\n",
                    targetSleepTime);
    }
  }

  Serial.printf("Going to sleep for %d minutes\n", targetSleepTime);
  esp_sleep_enable_timer_wakeup(targetSleepTime * 60 * 1000000ULL);
  esp_deep_sleep_start();
}

void loop() {}
