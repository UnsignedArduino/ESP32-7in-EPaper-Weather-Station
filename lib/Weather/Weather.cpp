#include "Weather.h"

void printWeather(WeatherData& data) {
  Serial.println("Current weather:");
  Serial.printf("  Time: %s\n", data.currISOTime);
  Serial.printf("  Temperature: %.1f\n", data.currTemp);
  Serial.printf("  Humidity: %.1f\n", data.currHumidity);
  Serial.printf("  Is day: %s\n", data.currIsDay ? "yes" : "no");
  Serial.printf("  Weather code: %d\n", data.currWeatherCode);

  Serial.println("Forecast:");
  for (uint8_t i = 0; i < MAX_FORECAST_DAYS; i++) {
    Serial.printf("  (%d) Time: %s\n", i, data.forecastISOTimes[i]);
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
  client.printf(
      "GET /v1/forecast?"
      "latitude=%f&longitude=%f&"
      "current=temperature_2m,relative_humidity_2m,is_day,weather_code&"
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

  client.print("timezone=auto HTTP/1.1\r\nHost: api.open-meteo.com\r\n\r\n");

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

  JsonObject current = doc["current"];
  strcpy(data.currISOTime, current["time"]);
  data.currTemp = current["temperature_2m"];
  data.currHumidity = current["relative_humidity_2m"];
  data.currIsDay = current["is_day"];
  data.currWeatherCode = current["weather_code"];

  JsonObject daily = doc["daily"];
  JsonArray dailyTimes = daily["time"];
  JsonArray dailyWeatherCodes = daily["weather_code"];
  JsonArray dailyHighTemps = daily["temperature_2m_max"];
  JsonArray dailyLowTemps = daily["temperature_2m_min"];
  for (uint8_t i = 0; i < MAX_FORECAST_DAYS; i++) {
    strcpy(data.forecastISOTimes[i], dailyTimes[i]);
    data.forecastWeatherCodes[i] = dailyWeatherCodes[i];
    data.forecastHighTemps[i] = dailyHighTemps[i];
    data.forecastLowTemps[i] = dailyLowTemps[i];
  }

  printWeather(data);

  Serial.println("Done");
  client.stop();

  return GET_WEATHER_SUCCESS;
}
