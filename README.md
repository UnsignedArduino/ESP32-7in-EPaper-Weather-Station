# ESP32-7in-EPaper-Weather-Station

A weather station based on a Firebeetle ESP32 and a 7.5in Waveshare E-paper display. The data is fetched
from [Open-Meteo](https://open-meteo.com/).

## Build

### Hardware

A KiCad schematic and Fusion 360 design for it's case for this project can be
found [here](https://github.com/UnsignedArduino/ESP32-7in-EPaper-Weather-Station-Hardware).

### Software

1. Have PlatformIO installed.
2. Clone the repository.
3. Open in a PlatformIO-compatible IDE because I'm too lazy to remember the commands.
4. Create a Blynk template with a battery percentage (integer from 0 to 100) data stream and widget on virtual pin V0
   and a success/fail rate (string) data stream and widget on virtual pin V1 and a `failed_refresh` event.
5. In [`include`](include), rename [`config.sample.h`](include/config.sample.h) to `config.h` and fill in values.
6. Build filesystem image.
7. Upload filesystem image.
8. Go to `.platformio\packages\framework-arduinoespressif32\libraries\Update\src\Updater.cpp` and edit the definition
   for `bool UpdateClass::setMD5(const char* expected_md5)`:
   ```c++
   bool UpdateClass::setMD5(const char *expected_md5) {
     if (strlen(expected_md5) != 32) {
       return false;
     }
     _target_md5 = expected_md5;
     _target_md5.toLowerCase();  // <-- ADD THIS LINE
     return true;
   }
   ```
   Although this issue is fixed in
   the [latest version of the ESP32 Arduino core](https://github.com/espressif/arduino-esp32/blob/master/libraries/Update/src/Updater.cpp#L454),
   [PlatformIO is stuck with ESP32 Arduino core v2](https://github.com/platformio/platform-espressif32/issues/1225).
9. Upload.

## Contributing

Notes:

* The custom font is a slimmed down version of the [GNU Unifont](https://www.unifoundry.com/unifont/index.html) and
  converted into the `u8g2` font with [this tool](https://stncrn.github.io/u8g2-unifont-helper/).
* Icons are from [QWeather](https://icons.qweather.com/en/) and [Bootstrap](https://icons.getbootstrap.com/), converted
  with [ImageMagick](https://imagemagick.org/) to 100x100 bitmaps.
* I modified the [GxEPD2](https://github.com/ZinggJM/GxEPD2) library to
  support [reading B/W pixels](lib/GxEPD2/src/GxEPD2_BW.h) from it's framebuffer. This is to upscale the 16x16 fonts
  into a (blocky) 32x32 font on the fly.
