#ifndef ICONS_H
#define ICONS_H

#include "Settings.h"
#include "localizedStrings.h"
#include <Arduino.h>

const char* WMOCodeToFilename(uint8_t code, bool isDay);
const char* WMOCodeToLabel(uint8_t code);

#endif // ICONS_H
