#include "icons.h"

const char* WMOCodeToFilename(uint8_t code, bool isDay) {
  // https://open-meteo.com/en/docs#:~:text=WMO%20Weather%20interpretation%20codes%20(WW)
  // Icons from QWeather
  switch (code) {
    case 0: // clear sky
      return isDay ? "/QWeather-Icons-1.6.0/100-100x100.bmp"
                   : "/QWeather-Icons-1.6.0/150-100x100.bmp";
    case 1: // mainly clear
      return isDay ? "/QWeather-Icons-1.6.0/102-100x100.bmp"
                   : "/QWeather-Icons-1.6.0/152-100x100.bmp";
    case 2: // partly clear
      return isDay ? "/QWeather-Icons-1.6.0/103-100x100.bmp"
                   : "/QWeather-Icons-1.6.0/153-100x100.bmp";
    case 3: // overcast
      return "/QWeather-Icons-1.6.0/104-100x100.bmp";
    case 45: // fog
      return "/QWeather-Icons-1.6.0/501-100x100.bmp";
    case 48: // depositing rime fog --> frost icon
      return "/QWeather-Icons-1.6.0/1016-100x100.bmp";
    case 51: // light drizzle
    case 53: // moderate drizzle
    case 55: // dense drizzle
      return "/QWeather-Icons-1.6.0/309-100x100.bmp";
    case 56: // light freezing drizzle
    case 57: // dense freezing drizzle
      return "/QWeather-Icons-1.6.0/2214-100x100.bmp";
    case 61: // slight rain
      return "/QWeather-Icons-1.6.0/305-100x100.bmp";
    case 63: // moderate rain
      return "/QWeather-Icons-1.6.0/306-100x100.bmp";
    case 65: // heavy rain
      return "/QWeather-Icons-1.6.0/307-100x100.bmp";
    case 66: // light freezing rain
    case 67: // heavy freezing rain
      return "/QWeather-Icons-1.6.0/313-100x100.bmp";
    case 71: // slight snow fall
      return "/QWeather-Icons-1.6.0/400-100x100.bmp";
    case 73: // moderate snow fall
      return "/QWeather-Icons-1.6.0/401-100x100.bmp";
    case 75: // heavy snow fall
    case 77: // snow grains (couldn't find a good icon)
      return "/QWeather-Icons-1.6.0/402-100x100.bmp";
    case 80: // slight rain showers
      return isDay ? "/QWeather-Icons-1.6.0/300-100x100.bmp"
                   : "/QWeather-Icons-1.6.0/350-100x100.bmp";
    case 81: // moderate rain showers
    case 82: // violent rain showers
      return isDay ? "/QWeather-Icons-1.6.0/301-100x100.bmp"
                   : "/QWeather-Icons-1.6.0/351-100x100.bmp";
    case 85: // slight snow showers
    case 86: // heavy snow showers
      return isDay ? "/QWeather-Icons-1.6.0/406-100x100.bmp"
                   : "/QWeather-Icons-1.6.0/456-100x100.bmp";
    case 95: // thunderstorms
      return "/QWeather-Icons-1.6.0/302-100x100.bmp";
    case 96: // slight hailing thunderstorms
    case 99: // heavy hailing thunderstorms
      return "/QWeather-Icons-1.6.0/304-100x100.bmp";
    default: // unknown
      return "/QWeather-Icons-1.6.0/999-100x100.bmp";
  }
}

const char* WMOCodeToLabel(uint8_t code) {
  // https://open-meteo.com/en/docs#:~:text=WMO%20Weather%20interpretation%20codes%20(WW)
  const uint8_t codes[] = {0,  1,  2,  3,  45, 48, 51, 53, 55, 56,
                           57, 61, 63, 65, 66, 67, 71, 73, 75, 77,
                           80, 81, 82, 85, 86, 95, 96, 99};
  const char** strings;
  if (strcmp(languageSetting, LANGUAGE_EN) == 0) {
    strings = WEATHERS_EN;
  } else if (strcmp(languageSetting, LANGUAGE_CN_TRAD) == 0) {
    strings = WEATHERS_CN_TRAD;
  } else {
    strings = WEATHERS_CN_SIMP;
  }
  for (uint32_t i = 0; i < sizeof(codes) / sizeof(codes[0]); i++) {
    if (code == codes[i]) {
      return strings[i];
    }
  }
  return strings[sizeof(codes) / sizeof(codes[0])]; // missing -1 intentional
}
