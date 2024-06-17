#include "Geocoding.h"
#include "Settings.h"
#include "Touch.h"
#include "Weather.h"
#include "WiFiConnection.h"
#include "config.h"
#include "localizedStrings.h"
#include "pins.h"
#include <Adafruit_EPD.h>
#include <Adafruit_GFX.h>
#include <Arduino.h>
#include <Fonts/FreeMono9pt7b.h>
#include <Fonts/FreeMonoBold9pt7b.h>
#include <GxEPD2_BW.h>
#include <GxEPD2_display_selection_new_style.h>
#include <LittleFS.h>
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

uint16_t read16(fs::File& f) {
  // BMP data is stored little-endian, same as Arduino.
  uint16_t result;
  ((uint8_t*)&result)[0] = f.read(); // LSB
  ((uint8_t*)&result)[1] = f.read(); // MSB
  return result;
}

uint32_t read32(fs::File& f) {
  // BMP data is stored little-endian, same as Arduino.
  uint32_t result;
  ((uint8_t*)&result)[0] = f.read(); // LSB
  ((uint8_t*)&result)[1] = f.read();
  ((uint8_t*)&result)[2] = f.read();
  ((uint8_t*)&result)[3] = f.read(); // MSB
  return result;
}

// https://github.com/ZinggJM/GxEPD2/blob/master/examples/GxEPD2_Spiffs_Example/GxEPD2_Spiffs_Example.ino#L245
void displayBitmap(const char* filename, int16_t x, int16_t y) {
  const bool invert = true;

  bool with_color = false;
  static const uint16_t input_buffer_pixels = 800; // may affect performance

  static const uint16_t max_row_width =
      1872; // for up to 7.8" display 1872x1404
  static const uint16_t max_palette_pixels = 256; // for depth <= 8

  uint8_t input_buffer[3 * input_buffer_pixels]; // up to depth 24
  uint8_t output_row_mono_buffer[max_row_width /
                                 8]; // buffer for at least one row of b/w bits
  uint8_t output_row_color_buffer[max_row_width / 8]; // buffer for at least one
                                                      // row of color bits
  uint8_t mono_palette_buffer[max_palette_pixels /
                              8]; // palette buffer for depth <= 8 b/w
  uint8_t color_palette_buffer[max_palette_pixels /
                               8]; // palette buffer for depth <= 8 c/w

  fs::File file;
  bool valid = false; // valid format to be handled
  bool flip = true;   // bitmap is stored bottom-to-top
  uint32_t startTime = millis();
  if ((x >= display.epd2.WIDTH) || (y >= display.epd2.HEIGHT))
    return;
  // Serial.print("Loading image '");
  // Serial.print(filename);
  // Serial.println("'");
  file = LittleFS.open(filename, "r");
  if (!file) {
    Serial.println("File not found");
    return;
  } else {
    // Serial.println("Opened file successfully");
  }
  // Parse BMP header
  uint16_t signature = read16(file);
  // Serial.print("Magic number: 0x");
  // Serial.println(signature, HEX);
  if (signature == 0x4D42) // BMP signature
  {
    uint32_t fileSize = read32(file);
    uint32_t creatorBytes = read32(file);
    (void)creatorBytes;                  // unused
    uint32_t imageOffset = read32(file); // Start of image data
    uint32_t headerSize = read32(file);
    uint32_t width = read32(file);
    int32_t height = (int32_t)read32(file);
    uint16_t planes = read16(file);
    uint16_t depth = read16(file); // bits per pixel
    uint32_t format = read32(file);
    if ((planes == 1) &&
        ((format == 0) || (format == 3))) // uncompressed is handled, 565 also
    {
      // Serial.print("File size: ");
      // Serial.println(fileSize);
      // Serial.print("Image Offset: ");
      // Serial.println(imageOffset);
      // Serial.print("Header size: ");
      // Serial.println(headerSize);
      // Serial.print("Bit Depth: ");
      // Serial.println(depth);
      // Serial.print("Image size: ");
      // Serial.print(width);
      // Serial.print('x');
      // Serial.println(height);
      // BMP rows are padded (if needed) to 4-byte boundary
      uint32_t rowSize = (width * depth / 8 + 3) & ~3;
      if (depth < 8)
        rowSize = ((width * depth + 8 - depth) / 8 + 3) & ~3;
      if (height < 0) {
        height = -height;
        flip = false;
      }
      uint16_t w = width;
      uint16_t h = height;
      if ((x + w - 1) >= display.epd2.WIDTH)
        w = display.epd2.WIDTH - x;
      if ((y + h - 1) >= display.epd2.HEIGHT)
        h = display.epd2.HEIGHT - y;
      if (w <= max_row_width) // handle with direct drawing
      {
        valid = true;
        uint8_t bitmask = 0xFF;
        uint8_t bitshift = 8 - depth;
        uint16_t red, green, blue;
        bool whitish = false;
        bool colored = false;
        if (depth == 1)
          with_color = false;
        if (depth <= 8) {
          if (depth < 8)
            bitmask >>= depth;
          // file.seek(54); //palette is always @ 54
          file.seek(imageOffset -
                    (4 << depth)); // 54 for regular, diff for colorsimportant
          for (uint16_t pn = 0; pn < (1 << depth); pn++) {
            blue = file.read();
            green = file.read();
            red = file.read();
            if (invert) {
              blue = 255 - blue;
              green = 255 - green;
              red = 255 - red;
            }
            file.read();
            whitish = with_color
                          ? ((red > 0x80) && (green > 0x80) && (blue > 0x80))
                          : ((red + green + blue) > 3 * 0x80); // whitish
            colored = (red > 0xF0) || ((green > 0xF0) &&
                                       (blue > 0xF0)); // reddish or yellowish?
            if (0 == pn % 8)
              mono_palette_buffer[pn / 8] = 0;
            mono_palette_buffer[pn / 8] |= whitish << pn % 8;
            if (0 == pn % 8)
              color_palette_buffer[pn / 8] = 0;
            color_palette_buffer[pn / 8] |= colored << pn % 8;
          }
        }
        uint32_t rowPosition =
            flip ? imageOffset + (height - h) * rowSize : imageOffset;
        for (uint16_t row = 0; row < h;
             row++, rowPosition += rowSize) // for each line
        {
          uint32_t in_remain = rowSize;
          uint32_t in_idx = 0;
          uint32_t in_bytes = 0;
          uint8_t in_byte = 0;           // for depth <= 8
          uint8_t in_bits = 0;           // for depth <= 8
          uint8_t out_byte = 0xFF;       // white (for w%8!=0 border)
          uint8_t out_color_byte = 0xFF; // white (for w%8!=0 border)
          uint32_t out_idx = 0;
          file.seek(rowPosition);
          for (uint16_t col = 0; col < w; col++) // for each pixel
          {
            // Time to read more pixel data?
            if (in_idx >= in_bytes) // ok, exact match for 24bit also (size IS
                                    // multiple of 3)
            {
              in_bytes =
                  file.read(input_buffer, in_remain > sizeof(input_buffer)
                                              ? sizeof(input_buffer)
                                              : in_remain);
              in_remain -= in_bytes;
              in_idx = 0;
            }
            switch (depth) {
              case 32:
                blue = input_buffer[in_idx++];
                green = input_buffer[in_idx++];
                red = input_buffer[in_idx++];
                if (invert) {
                  blue = 255 - blue;
                  green = 255 - green;
                  red = 255 - red;
                }
                in_idx++; // skip alpha
                whitish =
                    with_color
                        ? ((red > 0x80) && (green > 0x80) && (blue > 0x80))
                        : ((red + green + blue) > 3 * 0x80); // whitish
                colored =
                    (red > 0xF0) ||
                    ((green > 0xF0) && (blue > 0xF0)); // reddish or yellowish?
                break;
              case 24:
                blue = input_buffer[in_idx++];
                green = input_buffer[in_idx++];
                red = input_buffer[in_idx++];
                if (invert) {
                  blue = 255 - blue;
                  green = 255 - green;
                  red = 255 - red;
                }
                whitish =
                    with_color
                        ? ((red > 0x80) && (green > 0x80) && (blue > 0x80))
                        : ((red + green + blue) > 3 * 0x80); // whitish
                colored =
                    (red > 0xF0) ||
                    ((green > 0xF0) && (blue > 0xF0)); // reddish or yellowish?
                break;
              case 16: {
                uint8_t lsb = input_buffer[in_idx++];
                uint8_t msb = input_buffer[in_idx++];
                if (format == 0) // 555
                {
                  blue = (lsb & 0x1F) << 3;
                  green = ((msb & 0x03) << 6) | ((lsb & 0xE0) >> 2);
                  red = (msb & 0x7C) << 1;
                } else // 565
                {
                  blue = (lsb & 0x1F) << 3;
                  green = ((msb & 0x07) << 5) | ((lsb & 0xE0) >> 3);
                  red = (msb & 0xF8);
                }
                if (invert) {
                  blue = 255 - blue;
                  green = 255 - green;
                  red = 255 - red;
                }
                whitish =
                    with_color
                        ? ((red > 0x80) && (green > 0x80) && (blue > 0x80))
                        : ((red + green + blue) > 3 * 0x80); // whitish
                colored =
                    (red > 0xF0) ||
                    ((green > 0xF0) && (blue > 0xF0)); // reddish or yellowish?
              } break;
              case 1:
              case 2:
              case 4:
              case 8: {
                if (0 == in_bits) {
                  in_byte = input_buffer[in_idx++];
                  in_bits = 8;
                }
                uint16_t pn = (in_byte >> bitshift) & bitmask;
                whitish = mono_palette_buffer[pn / 8] & (0x1 << pn % 8);
                colored = color_palette_buffer[pn / 8] & (0x1 << pn % 8);
                in_byte <<= depth;
                in_bits -= depth;
              } break;
            }
            if (whitish) {
              // keep white
            } else if (colored && with_color) {
              out_color_byte &= ~(0x80 >> col % 8); // colored
            } else {
              out_byte &= ~(0x80 >> col % 8); // black
            }
            if ((7 == col % 8) ||
                (col == w - 1)) // write that last byte! (for w%8!=0 border)
            {
              output_row_color_buffer[out_idx] = out_color_byte;
              output_row_mono_buffer[out_idx++] = out_byte;
              out_byte = 0xFF;       // white (for w%8!=0 border)
              out_color_byte = 0xFF; // white (for w%8!=0 border)
            }
          } // end pixel
          uint16_t yrow = y + (flip ? h - row - 1 : row);
          // display.writeImage(output_row_mono_buffer, output_row_color_buffer,
          // x,
          //                    yrow, w, 1);
          display.drawBitmap(x, yrow, output_row_mono_buffer, w, 1,
                             GxEPD_BLACK);
        } // end line
        // Serial.print("loaded in ");
        // Serial.print(millis() - startTime);
        // Serial.println(" ms");
      }
    } else {
      Serial.printf("Error display bitmap %s\n", filename);
      Serial.println("Invalid bitmap format and plane count");
      Serial.print("planes: ");
      Serial.println(planes);
      Serial.print("format: ");
      Serial.println(format);
    }
  } else {
    Serial.printf("Error display bitmap %s\n", filename);
    Serial.println("Not a bitmap");
  }
  file.close();
  if (!valid) {
    Serial.printf("Error display bitmap %s\n", filename);
    Serial.println("bitmap format not handled.");
  }
}

uint16_t cursorXFromCenter(const char* text, uint16_t centerX) {
  return centerX - u8g2.getUTF8Width(text) / 2;
}

const char* WMOCodeToFilename(uint8_t code, bool isDay) {
  // https://open-meteo.com/en/docs#:~:text=WMO%20Weather%20interpretation%20codes%20(WW)
  // Icons from QWeather
  switch (code) {
    case 0: // clear sky
      return isDay ? "/100-100x100.bmp" : "/150-100x100.bmp";
    case 1: // mainly clear
      return isDay ? "/102-100x100.bmp" : "/152-100x100.bmp";
    case 2: // partly clear
      return isDay ? "/103-100x100.bmp" : "/153-100x100.bmp";
    case 3: // overcast
      return "/104-100x100.bmp";
    case 45: // fog
      return "/501-100x100.bmp";
    case 48: // depositing rime fog --> frost icon
      return "/1016-100x100.bmp";
    case 51: // light drizzle
    case 53: // moderate drizzle
    case 55: // dense drizzle
      return "/309-100x100.bmp";
    case 56: // light freezing drizzle
    case 57: // dense freezing drizzle
      return "/2214-100x100.bmp";
    case 61: // slight rain
      return "/305-100x100.bmp";
    case 63: // moderate rain
      return "/306-100x100.bmp";
    case 65: // heavy rain
      return "/307-100x100.bmp";
    case 66: // light freezing rain
    case 67: // heavy freezing rain
      return "/313-100x100.bmp";
    case 71: // slight snow fall
      return "/400-100x100.bmp";
    case 73: // moderate snow fall
      return "/401-100x100.bmp";
    case 75: // heavy snow fall
    case 77: // snow grains (couldn't find a good icon)
      return "/402-100x100.bmp";
    case 80: // slight rain showers
      return isDay ? "/300-100x100.bmp" : "/350-100x100.bmp";
    case 81: // moderate rain showers
    case 82: // violent rain showers
      return isDay ? "/301-100x100.bmp" : "/351-100x100.bmp";
    case 85: // slight snow showers
    case 86: // heavy snow showers
      return isDay ? "/406-100x100.bmp" : "/456-100x100.bmp";
    case 95: // thunderstorms
      return "/302-100x100.bmp";
    case 96: // slight hailing thunderstorms
    case 99: // heavy hailing thunderstorms
      return "/304-100x100.bmp";
    default: // unknown
      return "/999-100x100.bmp";
  }
}

const char* WMOCodeToLabel(uint8_t code) {
  // https://open-meteo.com/en/docs#:~:text=WMO%20Weather%20interpretation%20codes%20(WW)
  const uint8_t codes[] = {0,  1,  2,  3,  45, 48, 51, 53, 55, 56,
                           57, 61, 63, 65, 66, 67, 71, 73, 75, 77,
                           80, 81, 82, 85, 86, 95, 96, 99};
  const char** strings;
  if (strcmp(languageSetting, LANGUAGE_EN) == 0) {
    strings = WEATHERS_EN;
  } else {
    strings = WEATHERS_CN;
  }
  for (uint8_t i = 0; i < sizeof(codes) / sizeof(codes[0]); i++) {
    if (code == codes[i]) {
      return strings[i];
    }
  }
  return strings[sizeof(codes) / sizeof(codes[0]) - 1];
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
  display.fillScreen(GxEPD_WHITE);

  u8g2.setCursor(30, 46);
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

  displayBitmap(
      WMOCodeToFilename(weatherData.currWeatherCode, weatherData.currIsDay), 30,
      100);

  u8g2.setCursor(150, 100);
  u8g2.print(WMOCodeToLabel(weatherData.currWeatherCode));
  u8g2.setCursor(150, 120);
  u8g2.print(round(weatherData.currTemp), 0);
  u8g2.print(strcmp(tempUnitSetting, TEMP_UNIT_CELSIUS) == 0 ? "°C" : "°F");
  u8g2.print(" (");
  u8g2.print(round(weatherData.currLowTemp), 0);
  u8g2.print(strcmp(tempUnitSetting, TEMP_UNIT_CELSIUS) == 0 ? "°C" : "°F");
  u8g2.print(" - ");
  u8g2.print(round(weatherData.currHighTemp), 0);
  u8g2.print(strcmp(tempUnitSetting, TEMP_UNIT_CELSIUS) == 0 ? "°C" : "°F");
  u8g2.print(")");
  u8g2.setCursor(150, 140);
  u8g2.print(round(weatherData.currHumidity), 0);
  u8g2.print("%");

  const char** weekdayNames;
  const char** monthNames;
  const char** dayNames;
  if (strcmp(languageSetting, LANGUAGE_EN) == 0) {
    weekdayNames = WEEKDAY_NAMES_EN;
    monthNames = MONTH_NAMES_EN;
    dayNames = DAY_NAMES_EN;
  } else {
    weekdayNames = WEEKDAY_NAMES_CN;
    monthNames = MONTH_NAMES_CN;
    dayNames = DAY_NAMES_CN;
  }
  const uint16_t y = 270;
  for (uint8_t i = 1; i < MAX_FORECAST_DAYS - 1; i++) {
    const uint16_t x = 160 * (i - 1);
    const uint16_t centerX = x + 80;

    const uint32_t time = weatherData.forecastUnixTimes[i];

    const char* dayName = weekdayNames[weekday(time)];
    u8g2.setCursor(cursorXFromCenter(dayName, centerX), y);
    u8g2.print(dayName);

    char dateBuf[16];
    snprintf(dateBuf, sizeof(dateBuf), "%s %s", monthNames[month(time)],
             dayNames[day(time)]);
    u8g2.setCursor(cursorXFromCenter(dateBuf, centerX), y + 20);
    u8g2.print(dateBuf);

    char tempsBuf[16];
    formatTemp(tempsBuf, sizeof(tempsBuf), weatherData.forecastLowTemps[i]);
    u8g2.setCursor(cursorXFromCenter(tempsBuf, centerX), y + 40);
    u8g2.print(tempsBuf);
    formatTemp(tempsBuf, sizeof(tempsBuf), weatherData.forecastHighTemps[i]);
    u8g2.setCursor(cursorXFromCenter(tempsBuf, centerX), y + 60);
    u8g2.print(tempsBuf);

    displayBitmap(WMOCodeToFilename(weatherData.forecastWeatherCodes[i], true),
                  x + 30, y + 80);
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

RTC_DATA_ATTR bool lastUpdateSuccess = false;

void setup() {
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);

  // Weird deep sleep bug workaround
  delay(500);

  Serial.printf("Efuse MAC: 0x%012llX\n", ESP.getEfuseMac());

  bool showRefresh = true;

  printWakeupReason();

  // TODO: Add touch pad wake up
  esp_sleep_enable_timer_wakeup(UPDATE_TIME * 60 * 1000000ULL);

  if (esp_sleep_get_wakeup_cause() == ESP_SLEEP_WAKEUP_TIMER &&
      lastUpdateSuccess) {
    showRefresh = false;
    Serial.println("Not showing refresh because of timer wakeup and last "
                   "update was a success");
  }

  displayBegin();
  if (strcmp(languageSetting, LANGUAGE_EN) == 0) {
    u8g2.setFont(u8g2_font_unifont_tf);
  } else {
    u8g2.setFont(u8g2_font_wqy16_t_gb2312);
  }

  if (showRefresh) {
    display.fillScreen(GxEPD_WHITE);
    u8g2.setCursor(30, 46);
    // TODO: Localize with localizedStrings.h, add icon
    u8g2.print("Refreshing...");
    display.display(false);
  }

  if (!LittleFS.begin()) {
    Serial.println("Failed to mount file system");
  } else {
    Serial.println("File system mounted successfully");
    Serial.printf("Used %d of %d kb\n", LittleFS.usedBytes() / 1024,
                  LittleFS.totalBytes() / 1024);
  }

  if (isTouched()) {
    Serial.println("Touch detected, hold for 3 second to reset WiFi settings");
    digitalWrite(LED_BUILTIN, HIGH);
    const uint32_t start = millis();
    while (millis() - start < 3000) {
      if (!isTouched()) {
        break;
      }
      delay(10);
    }
    digitalWrite(LED_BUILTIN, LOW);
    if (isTouched()) {
      Serial.println("Held for 3 seconds, resetting WiFi settings");
      resetWiFiSettings();
    } else {
      Serial.println("Continuing without resetting");
    }
  }

  loadSettings();
  printSettings();
  // TODO: If we go to WiFi provisioning / configuration, show wifi icons, QR
  // code, instructions etc. Then show refresh icon again afterwards.
  connectToWiFi();

  const uint32_t timeFinishWiFiConnect = millis();

  GeocodeData geocodeData;
  getGeocode(cityOrPostalCodeSetting, geocodeData);

  WeatherData weatherData;
  int8_t result =
      getWeather(geocodeData.latitude, geocodeData.longitude, weatherData);

  disconnectFromWiFi();

  const uint32_t timeFinishDataFetch = millis();

  displayWeather(geocodeData, weatherData);

  displayEnd();

  const uint32_t timeFinishDisplay = millis();

  Serial.println("Timings:");
  Serial.printf("  WiFi connect finished at ms %lu\n", timeFinishWiFiConnect);
  Serial.printf("  Data fetch finished at ms %lu\n", timeFinishDataFetch);
  Serial.printf("  Display finished at ms %lu\n", timeFinishDisplay);

  lastUpdateSuccess = true;

  Serial.printf("Updating again in %d minutes\n", UPDATE_TIME);
  esp_deep_sleep_start();
}

void loop() {}
