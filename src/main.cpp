#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "lwifi.h"


#include <Fonts/FreeSansBold24pt7b.h>

#include "scroller.h"




TwoWire twi = TwoWire(1); // create our own TwoWire instance
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &twi, OLED_RESET);


ScrollingText scroller(display);

static uint16_t num_height;

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

static int max_width = 0; // Keep track of the maximum width

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
  display.fillRect(x, 8 + y, max_width + 1, num_height + 2, SSD1306_BLACK);

  // Display the distance below the scroller, starting at the specified x and y offset
  display.setCursor(x, 8 + y + num_height);
  display.println(distanceStr);
  display.display();

  // Update the maximum width if the current width is greater
  if (w > max_width) {
    max_width = w;
  }
}



float ff = 0.0;

float getDistance() {
  // Replace this with your method to get the actual distance
	ff += 0.01;
	if (ff > 9.9) {
		ff = 0.0;
	}
  return ff;
}

void setup() {

  Serial.begin(115200);
  Serial.println("Start");

  twi.begin(4, 15); // SDA, SCL
  twi.setClock(1000000L); 
  if(!display.begin(SSD1306_SWITCHCAPVCC, SSD1306_I2C_ADDRESS)) {
    Serial.printf("SSD1306 allocation failed\n");
    for(;;) {}
	}
	calculateHeight();
	display.clearDisplay();
  display.display();

  // Example: You can put static content here that will remain on the display
  scroller.startScrolling();
  wifi_connect();
}




void loop() {
  scroller.scroll();
  float distance = getDistance(); // Replace with your method to get the distance
  displayLargeDistance(distance, 10, 8);
  delay(10);
}
