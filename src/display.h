#pragma once
#include <memory>
#include "bigtext.h"
#include "scroller.h"
#include "powerline.h"

class Display {
public:
  virtual void display() {
  }
  virtual void scroller_add(const char *str) {
    Serial.print(str);
  }
  void taf(const char *format, ...) {
    va_list args;
    va_start(args, format);

    int len = vsnprintf(nullptr, 0, format, args);
    va_end(args);

    std::unique_ptr<char> buffer(new char[len + 1]);

    va_start(args, format);
    vsnprintf(buffer.get(), len + 1, format, args);
    va_end(args);

    scroller_add(buffer.get());
  }
  virtual void scroller_start() {
  }
  virtual void scroller_run() {
  }
  virtual void scroll_now() {
    // If we don't have a display, sleep a bit to make the time similar to having one
    delay(1000);
  }
  virtual void show_large_distance(float distance, int x = 0, int y = 0) {
  }
  virtual void show_power_line(int percentage = 0) {
  }
};



class SSDisplay : public Display {
  TwoWire twi;
  Adafruit_SSD1306 ssd_display;

  ScrollingText scroller;
  BigText bigText;
  PowerLine powerLine;

public:
  SSDisplay() : twi(1), ssd_display(SCREEN_WIDTH, SCREEN_HEIGHT, &twi, OLED_RESET), scroller(ssd_display), bigText(ssd_display), powerLine(ssd_display) {
    twi.begin(4, 15); // SDA, SCL
    twi.setClock(1000000L); 
    if(!ssd_display.begin(SSD1306_SWITCHCAPVCC, SSD1306_I2C_ADDRESS)) {
      Serial.printf("SSD1306 allocation failed\n");
      for(;;) {}
    }
    ssd_display.clearDisplay();
    ssd_display.display();
  }
  virtual void display() {
    ssd_display.display();
  }
  virtual void scroller_add(const char *str)  {
    scroller.textList.push(str);
  }

  void scroller_start() {
    scroller.startScrolling();
  }
  void scroller_run() {
    scroller.scroll();
  }
  void scroll_now() {
    scroller.force();
  }
  void show_large_distance(float distance, int x = 0, int y = 0) {
    bigText.displayLargeDistance(distance, x, y);
  }
  void show_power_line(int percentage = 0) {
    powerLine.show(percentage);
  }
};

extern Display* display;
