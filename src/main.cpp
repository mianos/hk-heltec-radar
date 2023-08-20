#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "lwifi.h"


#include "bigtext.h"
#include "scroller.h"



TwoWire twi = TwoWire(1); // create our own TwoWire instance
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &twi, OLED_RESET);

ScrollingText scroller(display);
BigText bigText(display);

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
	display.clearDisplay();
  display.display();

  // Example: You can put static content here that will remain on the display
  scroller.startScrolling();
  wifi_connect();
}




void loop() {
  scroller.scroll();
  float distance = getDistance(); // Replace with your method to get the distance
  bigText.displayLargeDistance(distance, 10, 8);
  delay(10);
}
