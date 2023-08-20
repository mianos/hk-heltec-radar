#pragma once
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include <Fonts/FreeSansBold24pt7b.h>

class BigText {
  uint16_t num_height;
  uint16_t max_width = 0;

  Adafruit_SSD1306& display;

  void calculateHeight() {
    display.setFont(&FreeSansBold24pt7b);
    
    int16_t x1, y1; // Position of upper-left corner of the bounding box
    uint16_t w, h; // Width and height of bounding box

    // Use a test string with digits and a decimal point
    String testStr = "0.00";

    // Calculate the bounds of the text
    display.getTextBounds(testStr, 0, 0, &x1, &y1, &w, &h);

    // Set the calculated height
    num_height = h;
  }

public:
  BigText(Adafruit_SSD1306& displayInstance) : display(displayInstance) {
    calculateHeight();
  }


  void displayLargeDistance(float distance, int x = 0, int y = 0) {
    // Convert distance to a String with N.NN format
    String distanceStr = String(distance, 2);

    // Set the font for the large text
    display.setFont(&FreeSansBold24pt7b);

    // Calculate the width of the current distance string
    int16_t x1, y1; // Position of upper-left corner of the bounding box
    uint16_t w, h; // Width and height of bounding box
    display.getTextBounds(distanceStr, 0, 0, &x1, &y1, &w, &h);

    // Clear the maximum width so far, starting at the specified x and y offset
    // Extend the clearing area slightly to the right and bottom
    display.fillRect(x, 8 + y, max_width + 2, num_height + 2, SSD1306_BLACK);
	  if (distance == 0.0f) {
			return;
		}

    // Display the distance below the scroller, starting at the specified x and y offset
    display.setCursor(x, 8 + y + num_height);
    display.println(distanceStr);

    // Update the maximum width if the current width is greater
    if (w > max_width) {
      max_width = w;
    }
  }
};

