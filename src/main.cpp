#include "Settings.h"
#include "Touch.h"
#include "WiFiConnection.h"
#include "pins.h"
#include <Arduino.h>

void setup() {
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);

  Serial.printf("Efuse MAC: 0x%012llX\n", ESP.getEfuseMac());

  if (isTouched()) {
    Serial.println("Touch detected, resetting WiFi settings");
    resetWiFiSettings();
  }

  loadSettings();
  printSettings();
  connectToWiFi();
}

void loop() {
  if (isTouched()) {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(100);
    digitalWrite(LED_BUILTIN, LOW);
    delay(100);
  } else {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(500);
    digitalWrite(LED_BUILTIN, LOW);
    delay(500);
  }
}
