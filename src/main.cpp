#include <Adafruit_EPD.h>
#include <Adafruit_GFX.h>
#include <Arduino.h>
#include <Fonts/FreeMono9pt7b.h>
#include <Fonts/FreeMonoBold9pt7b.h>
#include <GxEPD2_BW.h>
#include <GxEPD2_display_selection_new_style.h>
#include <U8g2_for_Adafruit_GFX.h>

U8G2_FOR_ADAFRUIT_GFX u8g2;

void setup() {
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);

  display.init(115200, true, 2, false);
  u8g2.begin(display);
  display.setRotation(0);
  display.setFont(&FreeMono9pt7b);
  display.setFullWindow();
  display.fillScreen(GxEPD_WHITE);

  u8g2.setFontMode(1);
  u8g2.setFontDirection(0);
  u8g2.setBackgroundColor(GxEPD_WHITE);
  u8g2.setForegroundColor(GxEPD_BLACK);

  u8g2.setFont(u8g2_font_9x18_mf);
  u8g2.setCursor(0, 10);
  u8g2.println("Hello, world!");

  u8g2.setCursor(0, 200);
  u8g2.setFont(u8g2_font_wqy16_t_gb2312);
  u8g2.println("你好，世界！");

  display.display(false);
  display.hibernate();
}

void loop() {}
