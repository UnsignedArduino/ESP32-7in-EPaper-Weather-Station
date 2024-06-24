# ESP32-7in-EPaper-Weather-Station

A weather station based on a Firebeetle ESP32 and a 7.5in Waveshare E-paper display. The data is fetched
from [Open-Meteo](https://open-meteo.com/).

## Build

### Hardware

The world's ugliest KiCad schematic for this project can be
found [here](https://github.com/UnsignedArduino/ESP32-7in-EPaper-Weather-Station-Hardware).

### Software

1. Have PlatformIO installed.
2. Clone the repository.
3. Open in a PlatformIO-compatible IDE because I'm too lazy to remember the commands.
4. Build filesystem image.
5. Upload filesystem image.
6. Upload.

## Contributing

Notes:

* The custom font is a slimmed down version of the [GNU Unifont](https://www.unifoundry.com/unifont/index.html) and
  converted into the `u8g2` font with [this tool](https://stncrn.github.io/u8g2-unifont-helper/).
* Icons are from [QWeather](https://icons.qweather.com/en/) and [Bootstrap](https://icons.getbootstrap.com/), converted
  with [ImageMagick](https://imagemagick.org/) to 100x100 bitmaps.
* I modified the [GxEPD2](https://github.com/ZinggJM/GxEPD2) library to
  support [reading B/W pixels](lib/GxEPD2/src/GxEPD2_BW.h) from it's framebuffer. This is to upscale the 16x16 fonts
  into a (blocky) 32x32 font on the fly.
