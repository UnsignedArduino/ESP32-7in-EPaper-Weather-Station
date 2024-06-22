/*

Wiring (using Firebeetle ESP32 dev board)

- E-Paper display (Waveshare 7.5 inch, 800x480 pixels, black/white)
  - PWR: D3 (IO 26) - allows us to switch the display on/off
  - Busy: IO 4
  - RST: IO 16
  - DC: IO 17
  - CS: IO 5
  - CLK: SCK (IO 18)
  - DIN: MOSI (IO 23)
  - GND: GND
  - VCC: D2 (IO 25) - allows us to switch the display on/off

  - Driver HAT switches
    - Display config: "B" (0.47R)
    - SPI: 0 (4 line SPI)

- Function button (a simple push button)
  - One side to GND
  - Other side to D4 (IO 27)

- Battery (3 AA batteries in series)
  - Positive to VCC
  - Negative to GND

- Battery reader through resistor divider (1M and 1M)
  - Voltage divider end to VCC
  - Middle to A0
  - Other voltage divider end to GND
*/

#ifndef PINS_H
#define PINS_H

#include <Arduino.h>

const gpio_num_t FUNCTION_BTN_PIN = GPIO_NUM_27;
const gpio_num_t BATTERY_PIN = GPIO_NUM_36;

const gpio_num_t EPD_PWR_PIN = GPIO_NUM_26;
const gpio_num_t EPD_VCC_PIN = GPIO_NUM_25;

#endif
