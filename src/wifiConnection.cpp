#include "wifiConnection.h"

// @todo Fix inputs that should be hidden not hiding
// @todo Add saving via preferences
int8_t connectToWiFi() {
  Serial.println("Connecting to WiFi");
  WiFiManager wm;

  Serial.println("Resetting settings");
  wm.resetSettings();

  WiFiManagerParameter customCityOrPostalCode("cityOrPostalCode",
                                              "City or postal code", "", 32);
  wm.addParameter(&customCityOrPostalCode);

  const char* customTempUnitHTML = R"(
    <label for="tempUnitInjectID">Temperature unit:</label>
    <select name="tempUnitInjectID" id="tempUnitInjectID" onchange="document.getElementById('tempUnitID').value = this.value">
      <option value="f">Fahrenheit (f)</option>
      <option value="c">Celsius (c)</option>
    </select>
    <script>
      setTimeout(() => {
        document.getElementById("tempUnitID").hidden = true;
        document.querySelector("[for='tempUnitID']").hidden = true;
        document.getElementById("tempUnitID").value = document.getElementById("tempUnitInjectID").value;
      });
    </script>
  )";
  WiFiManagerParameter customTempUnitInject(customTempUnitHTML);
  wm.addParameter(&customTempUnitInject);
  WiFiManagerParameter customTempUnit(
      "tempUnitID", "Temperature unit: (Should be hidden)", "", 2);
  wm.addParameter(&customTempUnit);

  const char* customWindSpeedUnitHTML = R"(
    <label for="windSpeedUnitInjectID">Wind speed unit:</label>
    <select name="windSpeedUnitInjectID" id="windSpeedUnitInjectID" onchange="document.getElementById('windSpeedUnitID').value = this.value">
      <option value="Kmh">Kilometers per hour (Km/h)</option>
      <option value="ms">Meters per second (m/s)</option>
      <option value="mph">Miles per hour (mph)</option>
      <option value="kn">Knots (kn)</option>
    </select>
    <script>
      setTimeout(() => {
        document.getElementById("windSpeedUnitID").hidden = true;
        document.querySelector("[for='windSpeedUnitID']").hidden = true;
        document.getElementById("windSpeedUnitID").value = document.getElementById("windSpeedUnitInjectID").value;
      });
    </script>
  )";
  WiFiManagerParameter customWindSpeedUnitInject(customWindSpeedUnitHTML);
  wm.addParameter(&customWindSpeedUnitInject);
  WiFiManagerParameter customWindSpeedUnit(
      "windSpeedUnitID", "Wind speed unit: (Should be hidden)", "", 4);
  wm.addParameter(&customWindSpeedUnit);

  const char* customPrecipitationUnitHTML = R"(
    <label for="precipitationUnitInjectID">Temperature unit:</label>
    <select name="precipitationUnitInjectID" id="precipitationUnitInjectID" onchange="document.getElementById('precipitationUnitID').value = this.value">
      <option value="mm">Millimeter (mm)</option>
      <option value="in">Inch (in)</option>
    </select>
    <script>
      setTimeout(() => {
        document.getElementById("precipitationUnitID").hidden = true;
        document.querySelector("[for='precipitationUnitID']").hidden = true;
        document.getElementById("precipitationUnitID").value = document.getElementById("precipitationUnitInjectID").value;
      });
    </script>
  )";
  WiFiManagerParameter customPrecipitationUnitInject(
      customPrecipitationUnitHTML);
  wm.addParameter(&customPrecipitationUnitInject);
  WiFiManagerParameter customPrecipitationUnit(
      "precipitationUnitID", "Precipitation unit: (Should be hidden)", "", 2);
  wm.addParameter(&customPrecipitationUnit);

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

  const bool res = wm.autoConnect(ssid, password);
  if (res) {
    Serial.println("Connected to WiFi");

    Serial.printf("City or postal code: %s\n",
                  customCityOrPostalCode.getValue());
    Serial.printf("Temperature unit: %s\n", customTempUnit.getValue());
    Serial.printf("Wind speed unit: %s\n", customWindSpeedUnit.getValue());
    Serial.printf("Precipitation unit: %s\n",
                  customPrecipitationUnit.getValue());
    return 0;
  } else {
    Serial.println("Failed to connect to WiFi");
    return -1;
  }
}
