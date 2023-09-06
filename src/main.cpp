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
#include "ld2410.h"
#include "ld1125.h"

TwoWire twi = TwoWire(1); // create our own TwoWire instance
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &twi, OLED_RESET);

ScrollingText scroller{display};
BigText bigText{display};
PowerLine powerLine{display};
RadarMqtt mqtt{scroller};
bool network_up{false};

class LocalEP : public EventProc {
  BigText *bigText;
  PowerLine *powerLine;
  RadarMqtt *mqtt;
public:
  LocalEP(BigText *bigText, PowerLine *powerLine, RadarMqtt *mqtt) : bigText(bigText), powerLine(powerLine), mqtt(mqtt) {
  }

  virtual void Detected(String& type, float distanceValue, float strengthValue, bool entry) {
    bigText->displayLargeDistance(distanceValue, 10, 8);
    powerLine->show(strengthValue);
    if (network_up) {
      if (entry) {
        mqtt->mqtt_update_presence(entry, false, distanceValue, strengthValue);
      } else {
        mqtt->mqtt_update_presence(entry, true, distanceValue, strengthValue);
      }
    }
  }
  virtual void Cleared() {
    bigText->displayLargeDistance(0.0, 10, 8);
    powerLine->show(0);
    if (network_up) {
      mqtt->mqtt_update_presence(false);
    }
  }
};

RadarSensor *radarSensor;


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
  load_settings();
  // Example: You can put static content here that will remain on the display
  scroller.startScrolling();

  auto *lep = new LocalEP{&bigText, &powerLine, &mqtt};

  if (!strcmp(radar_module, "ld2411")) {
    radarSensor = new LD2411{lep};
    Serial.printf("LD2411  radar module type '%s'\n", radar_module);
  } else if (!strcmp(radar_module, "ld1125")) {
    radarSensor = new LD1125{lep};
    Serial.printf("LD1125  radar module type '%s'\n", radar_module);
  } else if (!strcmp(radar_module, "ld2410")) {
    radarSensor = new LD2410{lep};
    Serial.printf("LD2410  radar module type '%s'\n", radar_module);
  } else {
    scroller.taf("Undefined radar module type '%s'\n", radar_module);
    Serial.printf("Undefined radar module type '%s' using LD1125\n", radar_module);
    // Using any one as not to have null calls.
    radarSensor = new LD2411{lep};
  }

  wifi_connect();
  DateTime.setTimeZone("AEST-10AEDT,M10.1.0,M4.1.0/3");
  DateTime.begin(/* timeout param */);
  if (!DateTime.isTimeValid()) {
    scroller.taf("Failed to get time from server\n");
  }
  network_up = true;
}

void loop() {
  if (radarSensor) {
    radarSensor->processRadarData();
  }
  scroller.scroll();
  mqtt.handle();
  display.display();
  delay(1);
}
