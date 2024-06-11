#include "settings.h"
#include "wifiConnection.h"
#include <Arduino.h>

void setup() {
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);

  Serial.printf("Efuse MAC: 0x%012llX\n", ESP.getEfuseMac());

  loadSettings();
  printSettings();
  // resetWiFiSettings();
  connectToWiFi();
}

void loop() {
  digitalWrite(LED_BUILTIN, HIGH);
  delay(500);
  digitalWrite(LED_BUILTIN, LOW);
  delay(500);
}
