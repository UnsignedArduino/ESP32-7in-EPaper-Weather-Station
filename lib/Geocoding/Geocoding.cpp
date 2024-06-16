#include "Geocoding.h"

void printGeocode(GeocodeData& data) {
  Serial.printf("Geocode:\n");
  Serial.printf("  Latitude: %f\n", data.latitude);
  Serial.printf("  Longitude: %f\n", data.longitude);
  Serial.printf("  Name: %s\n", data.name);
  Serial.printf("  Country: %s\n", data.country);
  Serial.printf("  Admin1: %s\n", data.admin1);
  Serial.printf("  Admin2: %s\n", data.admin2);
  Serial.printf("  Admin3: %s\n", data.admin3);
  Serial.printf("  Admin4: %s\n", data.admin4);
}

int8_t getGeocode(char cityOrPostalCode[MAX_CITY_OR_POSTAL_CODE_LENGTH],
                  GeocodeData& data) {
  Serial.printf("Geocoding for %s\n", cityOrPostalCode);
  data.latitude = 0.0;
  data.longitude = 0.0;

  Serial.println("Checking cache");
  Preferences preferences;
  preferences.begin("geocodeCache");
  if (strcmp(cityOrPostalCode, preferences.getString("lastCode", "").c_str()) ==
      0) {
    data.latitude = preferences.getFloat("latitude", 0.0);
    data.longitude = preferences.getFloat("longitude", 0.0);
    strcpy(data.name, preferences.getString("name", "").c_str());
    strcpy(data.country, preferences.getString("country", "").c_str());
    strcpy(data.admin1, preferences.getString("admin1", "").c_str());
    strcpy(data.admin2, preferences.getString("admin2", "").c_str());
    strcpy(data.admin3, preferences.getString("admin3", "").c_str());
    strcpy(data.admin4, preferences.getString("admin4", "").c_str());
    preferences.end();
    Serial.println("Cache hit");
    printGeocode(data);
    return GET_COORDINATE_SUCCESS_CACHE;
  }

  Serial.println("Cache miss, fetching from server");

  Serial.println("Connecting to geocoding API");
  WiFiClient client;
  if (!client.connect("geocoding-api.open-meteo.com", 80)) {
    Serial.println("Failed to connect to geocoding API");
    return GET_COORDINATE_CONNECTION_FAIL;
  }

  Serial.println("Connected, sending request");
  client.printf("GET /v1/search?name=01801&count=1&language=en&format=json "
                "HTTP/1.1\r\n");
  client.printf("Host: geocoding-api.open-meteo.com\r\n\r\n");

  Serial.println("Waiting for response");
  const uint32_t timeout = millis() + 5000;
  while (!client.available() && millis() < timeout) {
    delay(100);
  }
  if (!client.available()) {
    Serial.println("No response, timeout");
    return GET_COORDINATE_CONNECTION_TIMEOUT;
  }

  client.find("\r\n\r\n");

  Serial.println("Reading and parsing response");

  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, client);
  if (error) {
    Serial.print("Failed to parse response: ");
    Serial.println(error.c_str());
    return GET_COORDINATE_PARSE_FAIL;
  }

  JsonObject result = doc["results"][0];

  data.latitude = result["latitude"];
  data.longitude = result["longitude"];
  strcpy(data.name, result["name"]);
  if (result.containsKey("country")) {
    strcpy(data.country, result["country"]);
  }
  if (result.containsKey("admin1")) {
    strcpy(data.admin1, result["admin1"]);
  }
  if (result.containsKey("admin2")) {
    strcpy(data.admin2, result["admin2"]);
  }
  if (result.containsKey("admin3")) {
    strcpy(data.admin3, result["admin3"]);
  }
  if (result.containsKey("admin4")) {
    strcpy(data.admin4, result["admin4"]);
  }

  Serial.println("Done");
  client.stop();

  Serial.println("Caching geocoding result");
  preferences.putString("lastCode", cityOrPostalCode);
  preferences.putFloat("latitude", data.latitude);
  preferences.putFloat("longitude", data.longitude);
  preferences.putString("name", data.name);
  preferences.putString("country", data.country);
  preferences.putString("admin1", data.admin1);
  preferences.putString("admin2", data.admin2);
  preferences.putString("admin3", data.admin3);
  preferences.putString("admin4", data.admin4);
  preferences.end();

  Serial.println("Geocoding complete");
  printGeocode(data);

  return GET_COORDINATE_SUCCESS;
}
