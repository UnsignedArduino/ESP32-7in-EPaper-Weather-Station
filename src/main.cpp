#include <Arduino.h>
#include <WiFiManager.h>

int8_t connectToWiFi() {
  Serial.println("Connecting to WiFi");
  WiFiManager wm;
  // Serial.println("Resetting settings");
  // wm.resetSettings();
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
    return 0;
  } else {
    Serial.println("Failed to connect to WiFi");
    return -1;
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);

  Serial.printf("Efuse MAC: 0x%012llX\n", ESP.getEfuseMac());

  connectToWiFi();
}

void loop() {
  digitalWrite(LED_BUILTIN, HIGH);
  delay(500);
  digitalWrite(LED_BUILTIN, LOW);
  delay(500);
}
