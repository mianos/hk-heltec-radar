#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "lwifi.h"


#include "bigtext.h"
#include "scroller.h"
#include "powerline.h"


TwoWire twi = TwoWire(1); // create our own TwoWire instance
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &twi, OLED_RESET);


float ff = 0.0;

float getDistance() {
  // Replace this with your method to get the actual distance
	ff += 0.01;
	if (ff > 9.9) {
		ff = 0.0;
	}
  return ff;
}

ScrollingText scroller(display);
BigText bigText(display);
PowerLine powerLine(display);

void setup() {

  Serial.begin(115200);
  Serial.println("Start");

  twi.begin(4, 15); // SDA, SCL
  twi.setClock(1000000L); 
  if(!display.begin(SSD1306_SWITCHCAPVCC, SSD1306_I2C_ADDRESS)) {
    Serial.printf("SSD1306 allocation failed\n");
    for(;;) {}
	}
	display.clearDisplay();
  display.display();

  // Example: You can put static content here that will remain on the display
  scroller.startScrolling();
  wifi_connect();
}



float mapRange(float input) {
  input = fmod(input, 10.0f); // Ensures input is within the range 0 to 10
  if (input <= 2.5f) {
    return 50 + (input / 2.5f) * 50;
  } else if (input <= 5.0f) {
    return 100 - ((input - 2.5f) / 2.5f) * 100;
  } else if (input <= 7.5f) {
    return ((input - 5.0f) / 2.5f) * 50;
  } else {
    return 50 + ((input - 7.5f) / 2.5f) * 50;
  }
}



void loop() {
  scroller.scroll();
  float distance = getDistance(); // Replace with your method to get the distance
  bigText.displayLargeDistance(distance, 10, 8);
	powerLine.show(mapRange(distance));
  display.display();
  delay(10);
}
