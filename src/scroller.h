#pragma once
#include <Adafruit_SSD1306.h>
#include <LinkedList.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    16
#define SSD1306_I2C_ADDRESS 0x3C

class ScrollingText {
private:
  static constexpr int charWidth = 6;
  static constexpr int scrollDelay = 100;
  static constexpr int MAX_STRING_SIZE = 120;
  Adafruit_SSD1306& display;
  LinkedList<String> textList;
  String text;
  int textPosition;
  int currentTextIndex;
  bool finishedScrolling;
  char displayBuffer[SCREEN_WIDTH / charWidth + 1];
                               //
  unsigned long lastScrollTime;
  
public:
  ScrollingText(Adafruit_SSD1306& displayInstance) : display(displayInstance) {
    currentTextIndex = 0;
    finishedScrolling = false;
    lastScrollTime = 0;
    
    // Initialize the buffer with spaces
    for (int i = 0; i < SCREEN_WIDTH / charWidth; i++) {
      displayBuffer[i] = ' ';
    }
    displayBuffer[SCREEN_WIDTH / charWidth] = '\0'; // Null-terminate the buffer
  }

  void addText(const String newText) {
    textList.add(newText);
  }

  void taf(const char *format, ...) {
      char str[MAX_STRING_SIZE];
      va_list args;
      va_start(args, format);
      vsnprintf(str, MAX_STRING_SIZE, format, args);
      va_end(args);
      textList.add(str);
  }

  void startScrolling() {
    text = textList.get(currentTextIndex);
    textPosition = text.length() - 1;
  }

	void scroll() {
			if (millis() - lastScrollTime < scrollDelay) return;

			if (finishedScrolling) {
				currentTextIndex++;
				if (currentTextIndex >= textList.size()) {
					return; // All texts have been displayed
				}
				text = textList.get(currentTextIndex);
				textPosition = 0;  // Start at the beginning of the text
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
	}



  void scrollR() {
    if (millis() - lastScrollTime < scrollDelay) return;

    if (finishedScrolling) {
      currentTextIndex++;
      if (currentTextIndex >= textList.size()) {
        return; // All texts have been displayed
      }
      text = textList.get(currentTextIndex);
      textPosition = text.length() - 1;
      finishedScrolling = false;
      // Clear the display buffer
      for (int i = 0; i < SCREEN_WIDTH / charWidth; i++) {
        displayBuffer[i] = ' ';
      }
    }

    // Shift everything in the buffer to the right
    for (int i = SCREEN_WIDTH / charWidth - 1; i > 0; i--) {
      displayBuffer[i] = displayBuffer[i - 1];
    }

    // Add the next character from the text to the buffer, or a space if the text is done
    if (textPosition >= 0) {
      displayBuffer[0] = text[textPosition];
      textPosition--;
    } else {
      displayBuffer[0] = ' ';
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
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.print(displayBuffer);

    lastScrollTime = millis(); // Update the last scroll time
  }
};

extern ScrollingText scroller;
