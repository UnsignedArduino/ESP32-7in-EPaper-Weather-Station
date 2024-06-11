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
  static uint32_t lastTouchMillis = 0;
  static uint32_t lastIdleFlashMillis = 0;
  static bool ledState = false;
  static bool prevTouchState = false;

  if (isTouched()) {
    lastTouchMillis = millis();
  }

  if (isTouched() && !prevTouchState) {
    Serial.println("Touch detected");
    prevTouchState = true;
    digitalWrite(LED_BUILTIN, HIGH);
    ledState = true;
  } else if (!isTouched() && prevTouchState) {
    Serial.println("Touch released");
    prevTouchState = false;
    digitalWrite(LED_BUILTIN, LOW);
    ledState = false;
  }

  if (millis() - lastTouchMillis > 1000 &&
      millis() - lastIdleFlashMillis > 500) {
    digitalWrite(LED_BUILTIN, ledState ? LOW : HIGH);
    ledState = !ledState;
    lastIdleFlashMillis = millis();
  }

  delay(10);
}
