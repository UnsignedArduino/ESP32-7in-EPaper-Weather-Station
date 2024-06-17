#include "WiFiConnection.h"

namespace {
  void generateHTMLForSelect(const char* dest, size_t destSize,
                             const char* label, const char* injectID,
                             const char* targetHiddenID, const char* options[],
                             const char* optionLabels[], size_t optionsSize,
                             const char* selectedValue);
}

void generateHTMLForSelect(char* dest, size_t destSize, const char* label,
                           const char* injectID, const char* targetHiddenID,
                           const char* options[], const char* optionLabels[],
                           size_t optionsSize, const char* selectedValue) {
  snprintf(dest, destSize,
           "<label for=\"%s\">%s:</label><select name=\"%s\" "
           "id=\"%s\" onchange=\"document.getElementById('%s').value = "
           "this.value\">",
           injectID, label, injectID, injectID, targetHiddenID);
  for (size_t i = 0; i < optionsSize; i++) {
    snprintf(dest + strlen(dest), destSize - strlen(dest),
             "<option value=\"%s\"%s>%s</option>", options[i],
             strcmp(options[i], selectedValue) == 0 ? " selected" : "",
             optionLabels[i]);
  }
  snprintf(dest + strlen(dest), destSize - strlen(dest),
           "</select>"
           "<script type=\"module\">"
           "setTimeout(() => {"
           "document.getElementById(\"%s\").hidden = true;"
           "document.querySelector(\"[for='%s']\").hidden = true;"
           // clang-format off
           "document.getElementById(\"%s\").value = document.getElementById(\"%s\").value;"
           // clang-format on
           "});"
           "</script>",
           targetHiddenID, targetHiddenID, targetHiddenID, injectID);
}

int8_t resetWiFiSettings() {
  Serial.println("Resetting settings");
  WiFiManager wm;
  wm.resetSettings();
  return 0;
}

int8_t connectToWiFi() {
  Serial.println("Connecting to WiFi");
  WiFiManager wm;

  WiFiManagerParameter customCityOrPostalCode(
      "cityOrPostalCode", "City or postal code", cityOrPostalCodeSetting, 32);
  wm.addParameter(&customCityOrPostalCode);

  char customTempUnitHTML[1024];
  generateHTMLForSelect(customTempUnitHTML, sizeof(customTempUnitHTML),
                        "Temperature unit", "tempUnitInjectID", "tempUnitID",
                        TEMP_UNITS, TEMP_UNITS_LABELS, MAX_TEMP_UNITS,
                        tempUnitSetting);
  WiFiManagerParameter customTempUnitInject(customTempUnitHTML);
  wm.addParameter(&customTempUnitInject);
  WiFiManagerParameter customTempUnit(
      "tempUnitID", "Temperature unit: (Should be hidden)", "", 2);
  wm.addParameter(&customTempUnit);

  char customWindSpeedUnitHTML[1024];
  generateHTMLForSelect(customWindSpeedUnitHTML,
                        sizeof(customWindSpeedUnitHTML), "Wind speed unit",
                        "windSpeedUnitInjectID", "windSpeedUnitID",
                        WIND_SPEED_UNITS, WIND_SPEED_UNITS_LABELS,
                        MAX_WIND_SPEED_UNITS, windSpeedUnitSetting);
  WiFiManagerParameter customWindSpeedUnitInject(customWindSpeedUnitHTML);
  wm.addParameter(&customWindSpeedUnitInject);
  WiFiManagerParameter customWindSpeedUnit(
      "windSpeedUnitID", "Wind speed unit: (Should be hidden)", "", 4);
  wm.addParameter(&customWindSpeedUnit);

  char customPrecipitationUnitHTML[1024];
  generateHTMLForSelect(
      customPrecipitationUnitHTML, sizeof(customPrecipitationUnitHTML),
      "Precipitation unit", "precipitationUnitInjectID", "precipitationUnitID",
      PRECIPITATION_UNITS, PRECIPITATION_UNITS_LABELS, MAX_PRECIPITATION_UNITS,
      precipitationUnitSetting);
  WiFiManagerParameter customPrecipitationUnitInject(
      customPrecipitationUnitHTML);
  wm.addParameter(&customPrecipitationUnitInject);
  WiFiManagerParameter customPrecipitationUnit(
      "precipitationUnitID", "Precipitation unit: (Should be hidden)", "", 3);
  wm.addParameter(&customPrecipitationUnit);

  char customLanguageHTML[1024];
  generateHTMLForSelect(customLanguageHTML, sizeof(customLanguageHTML),
                        "Language", "languageInjectID", "languageID", LANGUAGES,
                        LANGUAGES_LABELS, MAX_LANGUAGES, languageSetting);
  WiFiManagerParameter customLanguageInject(customLanguageHTML);
  wm.addParameter(&customLanguageInject);
  WiFiManagerParameter customLanguage("languageID",
                                      "Language: (Should be hidden)", "", 3);
  wm.addParameter(&customLanguage);

  char ssid[32];
  char password[64];
  snprintf(ssid, sizeof(ssid), "Weather Station %04X",
           (uint16_t)(ESP.getEfuseMac() & 0xFFFF));
  snprintf(password, sizeof(password), "%012llX", ESP.getEfuseMac());

  wm.setAPCallback([ssid, password](WiFiManager* wm) {
    Serial.println("Launched configuration AP");
    Serial.printf("SSID: %s\n", ssid);
    Serial.printf("Password: %s\n", password);
    Serial.printf("IP: %s\n", WiFi.softAPIP().toString().c_str());
  });
  wm.setConfigPortalTimeout(120);
  wm.setConnectTimeout(30);
  wm.setConfigPortalTimeoutCallback(
      []() { Serial.println("Configuration portal timed out"); });
  bool shouldSaveConfig = false;
  wm.setSaveConfigCallback([&shouldSaveConfig]() { shouldSaveConfig = true; });

  const bool res = wm.autoConnect(ssid, password);
  if (res) {
    Serial.println("Connected to WiFi");

    if (shouldSaveConfig) {
      Serial.println("Saving settings from wifi manager");
      strncpy(cityOrPostalCodeSetting, customCityOrPostalCode.getValue(),
              MAX_CITY_OR_POSTAL_CODE_LENGTH);
      strncpy(tempUnitSetting, customTempUnit.getValue(), MAX_TEMP_UNIT_LENGTH);
      strncpy(windSpeedUnitSetting, customWindSpeedUnit.getValue(),
              MAX_WIND_SPEED_UNIT_LENGTH);
      strncpy(precipitationUnitSetting, customPrecipitationUnit.getValue(),
              MAX_PRECIPITATION_UNIT_LENGTH);
      strncpy(languageSetting, customLanguage.getValue(), MAX_LANGUAGE_LENGTH);
      saveSettings();
      printSettings();
    }

    return 0;
  } else {
    Serial.println("Failed to connect to WiFi");
    return -1;
  }
}
