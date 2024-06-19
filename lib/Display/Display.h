#ifndef ESP32_7IN_EPAPER_WEATHER_STATION_DISPLAY_H
#define ESP32_7IN_EPAPER_WEATHER_STATION_DISPLAY_H

#include <Adafruit_GFX.h>
#include <Arduino.h>
#include <GxEPD2_BW.h>
// Use this file to find the used display line
// #include <GxEPD2_display_selection_new_style.h>
#include <LittleFS.h>
#include <U8g2_for_Adafruit_GFX.h>

#define GxEPD2_DISPLAY_CLASS GxEPD2_BW
#define GxEPD2_DRIVER_CLASS                                                    \
  GxEPD2_750_T7 // GDEW075T7   800x480, EK79655 (GD7965), (WFT0583CZ61)
#define MAX_DISPLAY_BUFFER_SIZE 65536ul // e.g.
#define MAX_HEIGHT(EPD)                                                        \
  (EPD::HEIGHT <= MAX_DISPLAY_BUFFER_SIZE / (EPD::WIDTH / 8)                   \
     ? EPD::HEIGHT                                                             \
     : MAX_DISPLAY_BUFFER_SIZE / (EPD::WIDTH / 8))
#define EPD_CS SS

extern GxEPD2_DISPLAY_CLASS<GxEPD2_DRIVER_CLASS,
                            MAX_HEIGHT(GxEPD2_DRIVER_CLASS)>
  display;
extern U8G2_FOR_ADAFRUIT_GFX u8g2;
extern GFXcanvas1 canvas;

void displayBegin();
void displayScaleArea(int16_t x, int16_t y, int16_t w, int16_t h,
                      uint8_t scale);
void displayBitmap(const char* filename, int16_t x, int16_t y);
void blitCanvasToDisplay();
void displayEnd();

int16_t cursorXFromCenter(const char* text, uint16_t centerX,
                          uint8_t scaleText = 1);

#endif // ESP32_7IN_EPAPER_WEATHER_STATION_DISPLAY_H
