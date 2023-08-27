#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ESPDateTime.h>
#include "freertos/queue.h"

#include "lwifi.h"
#include "radar.h"


#include "bigtext.h"
#include "scroller.h"
#include "powerline.h"
#include "mqtt.h"
#include "ld2411.h"

TwoWire twi = TwoWire(1); // create our own TwoWire instance
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &twi, OLED_RESET);

ScrollingText scroller{display};
BigText bigText{display};
PowerLine powerLine{display};
RadarMqtt mqtt{scroller};


class LocalEP : public EventProc {
public:
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
} lep;

LD2411 *radarSensor;


static const int PIN=13;
static int iseen = 0;
QueueHandle_t interruptQueue;

void IRAM_ATTR handleInterrupt() {
	iseen++;
  int value = digitalRead(PIN);
  xQueueSendFromISR(interruptQueue, &value, NULL);
}


void setup() {
  Serial.begin(115200);

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

  interruptQueue = xQueueCreate(10, sizeof(int));
  pinMode(PIN, INPUT_PULLUP);  // Set pin as input with pullup
  attachInterrupt(digitalPinToInterrupt(PIN), handleInterrupt, HIGH);
  radarSensor = new LD2411{&lep};
}


void loop() {
  auto receivedValue = -1;

  scroller.scroll();
  //radarSensor->mirror();
  if (xQueueReceive(interruptQueue, &receivedValue, 10 / portTICK_PERIOD_MS)) {
    Serial.printf("seen by interupt\n");
	}
  radarSensor->processRadarData();
  display.display();
  mqtt.handle();
  delay(10);
}
