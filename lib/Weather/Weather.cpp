#include "Weather.h"

void printUnixTime(uint32_t unixTime) {
  char timestamp[MAX_TIMESTAMP_SIZE];
  snprintf(timestamp, MAX_TIMESTAMP_SIZE, "%02d/%02d/%04d %02d:%02d:%02d",
           month(unixTime), day(unixTime), year(unixTime), hour(unixTime),
           minute(unixTime), second(unixTime));
  Serial.print(timestamp);
}

void printWeather(WeatherData& data) {
  Serial.println("Current weather:");
  Serial.printf("  Time: ");
  printUnixTime(data.currUnixTime);
  Serial.printf(" (UTC%+ld)\n", data.utcOffset / 3600);
  Serial.printf("  Weather code: %d\n", data.currWeatherCode);
  Serial.printf("  Temperature: %.1f\n", data.currTemp);
  Serial.printf("  High temp: %.1f\n", data.currHighTemp);
  Serial.printf("  Low temp: %.1f\n", data.currLowTemp);
  Serial.printf("  Humidity: %.1f\n", data.currHumidity);
  Serial.printf("  Is day: %s\n", data.currIsDay ? "yes" : "no");

  Serial.println("Forecast:");
  for (uint8_t i = 0; i < MAX_FORECAST_DAYS; i++) {
    Serial.printf("  (%d) Time: ", i);
    printUnixTime(data.forecastUnixTimes[i]);
    Serial.println();
    Serial.printf("  (%d) Weather code: %d\n", i, data.forecastWeatherCodes[i]);
    Serial.printf("  (%d) High temp: %.1f\n", i, data.forecastHighTemps[i]);
    Serial.printf("  (%d) Low temp: %.1f\n", i, data.forecastLowTemps[i]);
  }
}

int8_t getWeather(float latitude, float longitude, WeatherData& data) {
  Serial.printf("Getting weather for %f, %f\n", latitude, longitude);

  Serial.println("Connecting to weather API");
  WiFiClient client;
  if (!client.connect("api.open-meteo.com", 80)) {
    Serial.println("Failed to connect to weather API");
    return GET_WEATHER_CONNECTION_FAIL;
  }

  Serial.println("Connected, sending request");
  // https://api.open-meteo.com/v1/forecast?latitude=XXXX&longitude=XXXX&
  // current=temperature_2m,relative_humidity_2m,is_day&
  // daily=weather_code,temperature_2m_max,temperature_2m_min&
  // temperature_unit=fahrenheit&wind_speed_unit=mph&precipitation_unit=inch&timezone=auto&
  // past_days=1
  client.printf("GET /v1/forecast?"
                "latitude=%f&longitude=%f&"
                "current=temperature_2m,relative_humidity_2m,is_day&"
                "daily=weather_code,temperature_2m_max,temperature_2m_min&",
                latitude, longitude);

  if (strcmp(tempUnitSetting, TEMP_UNIT_FAHRENHEIT) == 0) {
    client.print("temperature_unit=fahrenheit&");
  } // celsius is default

  if (strcmp(windSpeedUnitSetting, WIND_SPEED_UNIT_MS) == 0) {
    client.print("wind_speed_unit=ms&");
  } else if (strcmp(windSpeedUnitSetting, WIND_SPEED_UNIT_MPH) == 0) {
    client.print("wind_speed_unit=mph&");
  } else if (strcmp(windSpeedUnitSetting, WIND_SPEED_UNIT_KN) == 0) {
    client.print("wind_speed_unit=kn&");
  } // kmh is default

  if (strcmp(precipitationUnitSetting, PRECIPITATION_UNIT_IN) == 0) {
    client.print("precipitation_unit=inch&");
  } // mm is default

  client.print("timeformat=unixtime&timezone=auto HTTP/1.1\r\nHost: "
               "api.open-meteo.com\r\n\r\n");

  Serial.println("Waiting for response");
  const uint32_t timeout = millis() + 5000;
  while (!client.available() && millis() < timeout) {
    delay(100);
  }
  if (!client.available()) {
    Serial.println("No response, timeout");
    return GET_WEATHER_CONNECTION_TIMEOUT;
  }

  client.find("\r\n\r\n");
  // There is a random number inserted before our JSON document for some reason
  while (client.peek() != '{') {
    client.read();
  }

  Serial.println("Reading and parsing response");

  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, client);
  if (error) {
    Serial.print("Failed to parse response: ");
    Serial.println(error.c_str());
    return GET_WEATHER_PARSE_FAIL;
  }

  data.utcOffset = doc["utc_offset_seconds"];

  JsonObject current = doc["current"];
  data.currUnixTime = current["time"];
  data.currWeatherCode = doc["daily"]["weather_code"][0];
  data.currTemp = current["temperature_2m"];
  data.currHighTemp = doc["daily"]["temperature_2m_max"][0];
  data.currLowTemp = doc["daily"]["temperature_2m_min"][0];
  data.currHumidity = current["relative_humidity_2m"];
  data.currIsDay = current["is_day"];

  JsonObject daily = doc["daily"];
  JsonArray dailyTimes = daily["time"];
  JsonArray dailyWeatherCodes = daily["weather_code"];
  JsonArray dailyHighTemps = daily["temperature_2m_max"];
  JsonArray dailyLowTemps = daily["temperature_2m_min"];
  for (uint8_t i = 0; i < MAX_FORECAST_DAYS; i++) {
    data.forecastUnixTimes[i] = dailyTimes[i];
    data.forecastWeatherCodes[i] = dailyWeatherCodes[i];
    data.forecastHighTemps[i] = dailyHighTemps[i];
    data.forecastLowTemps[i] = dailyLowTemps[i];
  }

  printWeather(data);

  Serial.println("Done");
  client.stop();

  return GET_WEATHER_SUCCESS;
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
  } else if (strcmp(languageSetting, LANGUAGE_CN_TRAD) == 0) {
    strings = WEATHERS_CN_TRAD;
  } else {
    strings = WEATHERS_CN_SIMP;
  }
  for (uint32_t i = 0; i < sizeof(codes) / sizeof(codes[0]); i++) {
    if (code == codes[i]) {
      return strings[i];
    }
  }
  return strings[sizeof(codes) / sizeof(codes[0])]; // missing -1 intentional
}
