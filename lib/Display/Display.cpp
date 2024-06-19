#include "Display.h"

GxEPD2_DISPLAY_CLASS<GxEPD2_DRIVER_CLASS, MAX_HEIGHT(GxEPD2_DRIVER_CLASS)>
  display(
    GxEPD2_DRIVER_CLASS(/*CS=5*/ EPD_CS, /*DC=*/17, /*RST=*/16,
                        /*BUSY=*/4)); // my suggested wiring and proto board
U8G2_FOR_ADAFRUIT_GFX u8g2;
GFXcanvas1 canvas(800, 480);

void displayBegin() {
  display.init(115200, true, 2, false);
  display.setRotation(0);
  display.setFullWindow();
  display.fillScreen(GxEPD_WHITE);

  canvas.setRotation(0);
  canvas.fillScreen(GxEPD_WHITE);

  u8g2.begin(canvas);
  u8g2.setFontMode(1);
  u8g2.setFontDirection(0);
  u8g2.setBackgroundColor(GxEPD_WHITE);
  u8g2.setForegroundColor(GxEPD_BLACK);
}

void displayScaleArea(int16_t x, int16_t y, int16_t w, int16_t h,
                      uint8_t scale) {
  for (int16_t relY = h; relY >= 0; relY--) {
    for (int16_t relX = w; relX >= 0; relX--) {
      const int16_t oldX = x + relX;
      const int16_t oldY = y + relY;
      const int16_t newX = x + relX * scale;
      const int16_t newY = y + relY * scale;
      const bool color = canvas.getPixel(oldX, oldY);
      canvas.fillRect(newX, newY, scale, scale, color);
    }
  }
}

uint16_t read16(fs::File& f) {
  // BMP data is stored little-endian, same as Arduino.
  uint16_t result;
  ((uint8_t*)&result)[0] = f.read(); // LSB
  ((uint8_t*)&result)[1] = f.read(); // MSB
  return result;
}

uint32_t read32(fs::File& f) {
  // BMP data is stored little-endian, same as Arduino.
  uint32_t result;
  ((uint8_t*)&result)[0] = f.read(); // LSB
  ((uint8_t*)&result)[1] = f.read();
  ((uint8_t*)&result)[2] = f.read();
  ((uint8_t*)&result)[3] = f.read(); // MSB
  return result;
}

// https://github.com/ZinggJM/GxEPD2/blob/master/examples/GxEPD2_Spiffs_Example/GxEPD2_Spiffs_Example.ino#L245
void displayBitmap(const char* filename, int16_t x, int16_t y) {
  const bool invert = true;

  bool with_color = false;
  static const uint16_t input_buffer_pixels = 800; // may affect performance

  static const uint16_t max_row_width =
    1872; // for up to 7.8" display 1872x1404
  static const uint16_t max_palette_pixels = 256; // for depth <= 8

  uint8_t input_buffer[3 * input_buffer_pixels]; // up to depth 24
  uint8_t output_row_mono_buffer[max_row_width /
                                 8]; // buffer for at least one row of b/w bits
  uint8_t output_row_color_buffer[max_row_width / 8]; // buffer for at least one
                                                      // row of color bits
  uint8_t mono_palette_buffer[max_palette_pixels /
                              8]; // palette buffer for depth <= 8 b/w
  uint8_t color_palette_buffer[max_palette_pixels /
                               8]; // palette buffer for depth <= 8 c/w

  fs::File file;
  bool valid = false; // valid format to be handled
  bool flip = true;   // bitmap is stored bottom-to-top
  uint32_t startTime = millis();
  if ((x >= display.epd2.WIDTH) || (y >= display.epd2.HEIGHT))
    return;
  // Serial.print("Loading image '");
  // Serial.print(filename);
  // Serial.println("'");
  file = LittleFS.open(filename, "r");
  if (!file) {
    Serial.println("File not found");
    return;
  } else {
    // Serial.println("Opened file successfully");
  }
  // Parse BMP header
  uint16_t signature = read16(file);
  // Serial.print("Magic number: 0x");
  // Serial.println(signature, HEX);
  if (signature == 0x4D42) // BMP signature
  {
    uint32_t fileSize = read32(file);
    uint32_t creatorBytes = read32(file);
    (void)creatorBytes;                  // unused
    uint32_t imageOffset = read32(file); // Start of image data
    uint32_t headerSize = read32(file);
    uint32_t width = read32(file);
    int32_t height = (int32_t)read32(file);
    uint16_t planes = read16(file);
    uint16_t depth = read16(file); // bits per pixel
    uint32_t format = read32(file);
    if ((planes == 1) &&
        ((format == 0) || (format == 3))) // uncompressed is handled, 565 also
    {
      // Serial.print("File size: ");
      // Serial.println(fileSize);
      // Serial.print("Image Offset: ");
      // Serial.println(imageOffset);
      // Serial.print("Header size: ");
      // Serial.println(headerSize);
      // Serial.print("Bit Depth: ");
      // Serial.println(depth);
      // Serial.print("Image size: ");
      // Serial.print(width);
      // Serial.print('x');
      // Serial.println(height);
      // BMP rows are padded (if needed) to 4-byte boundary
      uint32_t rowSize = (width * depth / 8 + 3) & ~3;
      if (depth < 8)
        rowSize = ((width * depth + 8 - depth) / 8 + 3) & ~3;
      if (height < 0) {
        height = -height;
        flip = false;
      }
      uint16_t w = width;
      uint16_t h = height;
      if ((x + w - 1) >= display.epd2.WIDTH)
        w = display.epd2.WIDTH - x;
      if ((y + h - 1) >= display.epd2.HEIGHT)
        h = display.epd2.HEIGHT - y;
      if (w <= max_row_width) // handle with direct drawing
      {
        valid = true;
        uint8_t bitmask = 0xFF;
        uint8_t bitshift = 8 - depth;
        uint16_t red, green, blue;
        bool whitish = false;
        bool colored = false;
        if (depth == 1)
          with_color = false;
        if (depth <= 8) {
          if (depth < 8)
            bitmask >>= depth;
          // file.seek(54); //palette is always @ 54
          file.seek(imageOffset -
                    (4 << depth)); // 54 for regular, diff for colorsimportant
          for (uint16_t pn = 0; pn < (1 << depth); pn++) {
            blue = file.read();
            green = file.read();
            red = file.read();
            if (invert) {
              blue = 255 - blue;
              green = 255 - green;
              red = 255 - red;
            }
            file.read();
            whitish = with_color
                        ? ((red > 0x80) && (green > 0x80) && (blue > 0x80))
                        : ((red + green + blue) > 3 * 0x80); // whitish
            colored = (red > 0xF0) || ((green > 0xF0) &&
                                       (blue > 0xF0)); // reddish or yellowish?
            if (0 == pn % 8)
              mono_palette_buffer[pn / 8] = 0;
            mono_palette_buffer[pn / 8] |= whitish << pn % 8;
            if (0 == pn % 8)
              color_palette_buffer[pn / 8] = 0;
            color_palette_buffer[pn / 8] |= colored << pn % 8;
          }
        }
        uint32_t rowPosition =
          flip ? imageOffset + (height - h) * rowSize : imageOffset;
        for (uint16_t row = 0; row < h;
             row++, rowPosition += rowSize) // for each line
        {
          uint32_t in_remain = rowSize;
          uint32_t in_idx = 0;
          uint32_t in_bytes = 0;
          uint8_t in_byte = 0;           // for depth <= 8
          uint8_t in_bits = 0;           // for depth <= 8
          uint8_t out_byte = 0xFF;       // white (for w%8!=0 border)
          uint8_t out_color_byte = 0xFF; // white (for w%8!=0 border)
          uint32_t out_idx = 0;
          file.seek(rowPosition);
          for (uint16_t col = 0; col < w; col++) // for each pixel
          {
            // Time to read more pixel data?
            if (in_idx >= in_bytes) // ok, exact match for 24bit also (size IS
                                    // multiple of 3)
            {
              in_bytes =
                file.read(input_buffer, in_remain > sizeof(input_buffer)
                                          ? sizeof(input_buffer)
                                          : in_remain);
              in_remain -= in_bytes;
              in_idx = 0;
            }
            switch (depth) {
              case 32:
                blue = input_buffer[in_idx++];
                green = input_buffer[in_idx++];
                red = input_buffer[in_idx++];
                if (invert) {
                  blue = 255 - blue;
                  green = 255 - green;
                  red = 255 - red;
                }
                in_idx++; // skip alpha
                whitish = with_color
                            ? ((red > 0x80) && (green > 0x80) && (blue > 0x80))
                            : ((red + green + blue) > 3 * 0x80); // whitish
                colored =
                  (red > 0xF0) ||
                  ((green > 0xF0) && (blue > 0xF0)); // reddish or yellowish?
                break;
              case 24:
                blue = input_buffer[in_idx++];
                green = input_buffer[in_idx++];
                red = input_buffer[in_idx++];
                if (invert) {
                  blue = 255 - blue;
                  green = 255 - green;
                  red = 255 - red;
                }
                whitish = with_color
                            ? ((red > 0x80) && (green > 0x80) && (blue > 0x80))
                            : ((red + green + blue) > 3 * 0x80); // whitish
                colored =
                  (red > 0xF0) ||
                  ((green > 0xF0) && (blue > 0xF0)); // reddish or yellowish?
                break;
              case 16: {
                uint8_t lsb = input_buffer[in_idx++];
                uint8_t msb = input_buffer[in_idx++];
                if (format == 0) // 555
                {
                  blue = (lsb & 0x1F) << 3;
                  green = ((msb & 0x03) << 6) | ((lsb & 0xE0) >> 2);
                  red = (msb & 0x7C) << 1;
                } else // 565
                {
                  blue = (lsb & 0x1F) << 3;
                  green = ((msb & 0x07) << 5) | ((lsb & 0xE0) >> 3);
                  red = (msb & 0xF8);
                }
                if (invert) {
                  blue = 255 - blue;
                  green = 255 - green;
                  red = 255 - red;
                }
                whitish = with_color
                            ? ((red > 0x80) && (green > 0x80) && (blue > 0x80))
                            : ((red + green + blue) > 3 * 0x80); // whitish
                colored =
                  (red > 0xF0) ||
                  ((green > 0xF0) && (blue > 0xF0)); // reddish or yellowish?
              } break;
              case 1:
              case 2:
              case 4:
              case 8: {
                if (0 == in_bits) {
                  in_byte = input_buffer[in_idx++];
                  in_bits = 8;
                }
                uint16_t pn = (in_byte >> bitshift) & bitmask;
                whitish = mono_palette_buffer[pn / 8] & (0x1 << pn % 8);
                colored = color_palette_buffer[pn / 8] & (0x1 << pn % 8);
                in_byte <<= depth;
                in_bits -= depth;
              } break;
            }
            if (whitish) {
              // keep white
            } else if (colored && with_color) {
              out_color_byte &= ~(0x80 >> col % 8); // colored
            } else {
              out_byte &= ~(0x80 >> col % 8); // black
            }
            if ((7 == col % 8) ||
                (col == w - 1)) // write that last byte! (for w%8!=0 border)
            {
              output_row_color_buffer[out_idx] = out_color_byte;
              output_row_mono_buffer[out_idx++] = out_byte;
              out_byte = 0xFF;       // white (for w%8!=0 border)
              out_color_byte = 0xFF; // white (for w%8!=0 border)
            }
          } // end pixel
          uint16_t yrow = y + (flip ? h - row - 1 : row);
          // display.writeImage(output_row_mono_buffer, output_row_color_buffer,
          // x,
          //                    yrow, w, 1);
          canvas.drawBitmap(x, yrow, output_row_mono_buffer, w, 1, GxEPD_BLACK);
        } // end line
        // Serial.print("loaded in ");
        // Serial.print(millis() - startTime);
        // Serial.println(" ms");
      }
    } else {
      Serial.printf("Error display bitmap %s\n", filename);
      Serial.println("Invalid bitmap format and plane count");
      Serial.print("planes: ");
      Serial.println(planes);
      Serial.print("format: ");
      Serial.println(format);
    }
  } else {
    Serial.printf("Error display bitmap %s\n", filename);
    Serial.println("Not a bitmap");
  }
  file.close();
  if (!valid) {
    Serial.printf("Error display bitmap %s\n", filename);
    Serial.println("bitmap format not handled.");
  }
}

void blitCanvasToDisplay() {
  uint8_t pageTimes = 0;
  const uint32_t startTime = millis();
  display.setFullWindow();
  do {
    display.fillScreen(GxEPD_BLACK);
    display.drawBitmap(0, 0, canvas.getBuffer(), 800, 480, GxEPD_WHITE);
    pageTimes++;
  } while (display.nextPage());
  Serial.printf("Used %d pages to blit canvas to display in %d ms\n", pageTimes,
                millis() - startTime);
  // display.display(false);
}

void displayEnd() {
  display.hibernate();
}

int16_t cursorXFromCenter(const char* text, uint16_t centerX,
                          uint8_t scaleText /* = 1*/) {
  return centerX - u8g2.getUTF8Width(text) * scaleText / 2;
}
