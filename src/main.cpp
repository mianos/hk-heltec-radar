#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <LinkedList.h>

#define SSD1306_I2C_ADDRESS 0x3C

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    16

#define CHAR_WIDTH 6     // Width of character in pixels
#define CHAR_HEIGHT 8    // Height of character in pixels

TwoWire twi = TwoWire(1); // create our own TwoWire instance
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &twi, OLED_RESET);


LinkedList<String> textList = LinkedList<String>();



String text; // Current text to scroll
int textPosition; // Position in the current text
int currentTextIndex = 0; // Index of the current text in the list


const int charWidth = 6; // Assuming text size of 1
char displayBuffer[SCREEN_WIDTH / charWidth + 1]; // Buffer to hold what's currently displayed
bool finishedScrolling = false;

unsigned long lastScrollTime = 0; // To keep track of the last time scrollText was called
const unsigned long scrollDelay = 100; // Time between scroll steps in milliseconds



void setup() {
  twi.begin(4, 15); // SDA, SCL
  twi.setClock(1000000L); 
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.printf("SSD1306 allocation failed\n");
    for(;;) {}
	}
	display.clearDisplay();
  display.display();
  delay(2000);

  // Example: You can put static content here that will remain on the display
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 10);
  display.print(F("Static Content"));
  display.display();

  textList.add("First string");
  textList.add("Second string");
  textList.add("Third string");
	text = textList.get(currentTextIndex);
	textPosition = text.length() - 1; // Start from the end of the text

}


void scrollText() {
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
  display.display();
}




void loop() {
  if (millis() - lastScrollTime >= scrollDelay) {
    scrollText();
    lastScrollTime = millis(); // Update the last scroll time
  }
  delay(10);
}
