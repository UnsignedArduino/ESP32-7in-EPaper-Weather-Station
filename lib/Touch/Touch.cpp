#include "Touch.h"

bool isTouched() {
  return touchRead(PIN_TOUCH) < PIN_TOUCH_THRESHOLD;
}
