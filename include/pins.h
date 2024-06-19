/*

Wiring (using Firebeetle ESP32 dev board)

- E-Paper display (Waveshare 7.5 inch, 800x480 pixels, black/white)
  - PWR: 3.3v
  - Busy: IO 4
  - RST: IO 16
  - DC: IO 17
  - CS: IO 5
  - CLK: SCK (IO 18)
  - DIN: MOSI (IO 23)
  - GND: GND
  - VCC: 3.3v

  - Driver HAT switches
    - Display config: "B" (0.47R)
    - SPI: 0 (4 line SPI)

- Function button (a simple push button)
  - One side to GND
  - Other side to D4 (IO 27)

*/

#ifndef PINS_H
#define PINS_H

#include <Arduino.h>

const gpio_num_t FUNCTION_BTN_PIN = GPIO_NUM_27;

#endif
