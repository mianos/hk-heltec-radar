#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ESPDateTime.h>

#include "lwifi.h"
#include "radar.h"
#include "ld2125.h"


#include "bigtext.h"
#include "scroller.h"
#include "powerline.h"
#include "mqtt.h"

TwoWire twi = TwoWire(1); // create our own TwoWire instance
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &twi, OLED_RESET);

ScrollingText scroller{display};
BigText bigText{display};
PowerLine powerLine{display};
RadarMqtt mqtt{scroller};


struct LRadar : public LD2125 {
  Adafruit_SSD1306& display;

  LRadar(Adafruit_SSD1306& displayInstance) : LD2125(), display(displayInstance) {
    mqtt.add_radar(this);
  }

  virtual void Detected(String& type, float distanceValue, float strengthValue, bool entry) {
    bigText.displayLargeDistance(distanceValue, 10, 8);
    powerLine.show(strengthValue / 4);
    if (entry) {
      mqtt.mqtt_update_presence(entry, false, distanceValue, strengthValue);
    } else {
      mqtt.mqtt_update_presence(entry, true, distanceValue, strengthValue);
    }
  }
  virtual void Cleared() {
    bigText.displayLargeDistance(0.0, 10, 8);
    powerLine.show(0);
    mqtt.mqtt_update_presence(false);
  }
} *radarSensor;




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
  setenv("TZ", "AEST-10AEDT,M10.1.0,M4.1.0/3", 1);
  tzset();
  DateTime.begin(/* timeout param */);
  radarSensor = new LRadar{display};
}


void loop() {
  scroller.scroll();
  radarSensor->processRadarData();
  display.display();
  mqtt.handle();
  delay(10);
}
