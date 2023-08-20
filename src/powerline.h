#pragma once
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

class PowerLine {
  Adafruit_SSD1306& display;
public:
  PowerLine(Adafruit_SSD1306& displayInstance) : display(displayInstance) {}

  void show(int percentage) {
    if (percentage < 0) percentage = 0;
    if (percentage > 100) percentage = 100;

    int lineLength = (SCREEN_WIDTH * percentage) / 100;

    // Draw the filled part of the line
    display.fillRect(0, SCREEN_HEIGHT - 4, lineLength, 4, SSD1306_WHITE);

    // Clear the rest of the line if it's shorter
    if (lineLength < SCREEN_WIDTH) {
      display.fillRect(lineLength, SCREEN_HEIGHT - 4, SCREEN_WIDTH - lineLength, 4, SSD1306_BLACK);
    }
  }
};

