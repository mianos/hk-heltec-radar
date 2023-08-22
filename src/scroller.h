#pragma once
#include <Adafruit_SSD1306.h>
#include <LinkedList.h>

constexpr int SCREEN_WIDTH = 128;
constexpr int SCREEN_HEIGHT = 64;
constexpr int OLED_RESET = 16;
constexpr int SSD1306_I2C_ADDRESS = 0x3C;

class ScrollingText {
private:
  static constexpr int charWidth = 6;
  static constexpr int scrollDelay = 100;
  static constexpr int MAX_STRING_SIZE = 120;
  Adafruit_SSD1306& display;
  LinkedList<String> textList;
  String text;
  int textPosition = 0;
  bool finishedScrolling = false;
  char displayBuffer[SCREEN_WIDTH / charWidth + 1] = {};
  unsigned long lastScrollTime = 0;

public:
  ScrollingText(Adafruit_SSD1306& displayInstance) : display(displayInstance) {
    for (auto i = 0; i < SCREEN_WIDTH / charWidth; i++) {
      displayBuffer[i] = ' ';
    }
    displayBuffer[SCREEN_WIDTH / charWidth] = '\0'; // Null-terminate the buffer
  }

  void startScrolling() {
    text = textList.shift();
    textPosition = 0;
  }

  void taf(const char *format, ...) {
    char str[MAX_STRING_SIZE];
    va_list args;
    va_start(args, format);
    vsnprintf(str, MAX_STRING_SIZE, format, args);
    va_end(args);
    textList.add(str);
  }

  void force() {
    while (!scroll()) {
      display.display();
      delay(10);
    }
  }

  bool scroll(bool no_clear=false) {
    if (millis() - lastScrollTime < scrollDelay)
      return false;

    if (finishedScrolling) {
      if (textList.size() == 0) {
        return true; // All texts have been displayed
      }
      text = textList.shift();
      textPosition = 0; // Start at the beginning of the text
      finishedScrolling = false;

      // Clear the display buffer
      for (int i = 0; i < SCREEN_WIDTH / charWidth; i++) {
        displayBuffer[i] = ' ';
      }
    }

    // Shift everything in the buffer to the left
    for (int i = 0; i < SCREEN_WIDTH / charWidth - 1; i++) {
      displayBuffer[i] = displayBuffer[i + 1];
    }

    // Add the next character from the text to the buffer, or a space if the text is done
    if (textPosition < text.length()) {
      displayBuffer[SCREEN_WIDTH / charWidth - 1] = text[textPosition];
      textPosition++;
    } else {
      displayBuffer[SCREEN_WIDTH / charWidth - 1] = ' ';
    }

    // Check if the buffer is empty (all spaces), indicating the scrolling is finished
    bool bufferEmpty = true;
    for (int i = 0; i < SCREEN_WIDTH / charWidth; i++) {
      if (displayBuffer[i] != ' ') {
        bufferEmpty = false;
        break;
      }
    }
    if (bufferEmpty) {
      finishedScrolling = true;
    }

    // Clear the top line
    display.fillRect(0, 0, SCREEN_WIDTH, 8, SSD1306_BLACK);

    // Draw the buffer
    display.setFont();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.print(displayBuffer);

    lastScrollTime = millis(); // Update the last scroll time
    return false;
  }
};

extern ScrollingText scroller;

