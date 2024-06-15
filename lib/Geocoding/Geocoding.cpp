#include "Geocoding.h"

int8_t geocode(char cityOrPostalCode[MAX_CITY_OR_POSTAL_CODE_LENGTH],
               float& latitude, float& longitude, char name[MAX_NAME_SIZE],
               char country[MAX_NAME_SIZE], char admin1[MAX_NAME_SIZE],
               char admin2[MAX_NAME_SIZE], char admin3[MAX_NAME_SIZE],
               char admin4[MAX_NAME_SIZE]) {
  Serial.printf("Geocoding for %s\n", cityOrPostalCode);
  latitude = 0.0;
  longitude = 0.0;

  Serial.println("Checking cache");
  Preferences preferences;
  preferences.begin("geocodeCache");
  if (strcmp(cityOrPostalCode, preferences.getString("lastCode", "").c_str()) ==
      0) {
    latitude = preferences.getFloat("latitude", 0.0);
    longitude = preferences.getFloat("longitude", 0.0);
    strcpy(name, preferences.getString("name", "").c_str());
    strcpy(country, preferences.getString("country", "").c_str());
    strcpy(admin1, preferences.getString("admin1", "").c_str());
    strcpy(admin2, preferences.getString("admin2", "").c_str());
    strcpy(admin3, preferences.getString("admin3", "").c_str());
    strcpy(admin4, preferences.getString("admin4", "").c_str());
    preferences.end();
    Serial.printf(
        "Cache hit\nLatitude: %f\nLongitude: %f\nName: "
        "%s\nCountry: %s\nAdmin1: %s\nAdmin2: %s\nAdmin3: %s\nAdmin4: %s\n",
        latitude, longitude, name, country, admin1, admin2, admin3, admin4);
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

  latitude = result["latitude"];
  longitude = result["longitude"];
  strcpy(name, result["name"]);
  if (result.containsKey("country")) {
    strcpy(country, result["country"]);
  }
  if (result.containsKey("admin1")) {
    strcpy(admin1, result["admin1"]);
  }
  if (result.containsKey("admin2")) {
    strcpy(admin2, result["admin2"]);
  }
  if (result.containsKey("admin3")) {
    strcpy(admin3, result["admin3"]);
  }
  if (result.containsKey("admin4")) {
    strcpy(admin4, result["admin4"]);
  }

  Serial.println("Done");
  client.stop();

  Serial.println("Caching geocoding result");
  preferences.putString("lastCode", cityOrPostalCode);
  preferences.putFloat("latitude", latitude);
  preferences.putFloat("longitude", longitude);
  preferences.putString("name", name);
  preferences.putString("country", country);
  preferences.putString("admin1", admin1);
  preferences.putString("admin2", admin2);
  preferences.putString("admin3", admin3);
  preferences.putString("admin4", admin4);
  preferences.end();

  Serial.printf(
      "Geocoding fetched\nLatitude: %f\nLongitude: %f\nName: "
      "%s\nCountry: %s\nAdmin1: %s\nAdmin2: %s\nAdmin3: %s\nAdmin4: %s\n",
      latitude, longitude, name, country, admin1, admin2, admin3, admin4);

  return GET_COORDINATE_SUCCESS;
}
