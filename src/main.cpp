#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ArduinoOTA.h>
#include <ESPDateTime.h>
#include "freertos/queue.h"

#include "lwifi.h"
#include "radar.h"

#include "display.h"

#include "mqtt.h"
//#include "ld2411.h"
#include "ld2410.h"
#include "ld1125.h"


Display *display;

RadarMqtt *mqtt;

bool network_up{false};

class LocalEP : public EventProc {
  Display* display;
  RadarMqtt *mqtt;
public:
  LocalEP(Display *display, RadarMqtt *mqtt) : display(display), mqtt(mqtt) {
  }

  virtual void Detected(String& type, float distanceValue, float strengthValue, bool entry) {
    display->show_large_distance(distanceValue, 10, 8);
    display->show_power_line(strengthValue);
    if (network_up) {
      if (entry) {
        mqtt->mqtt_update_presence(entry, false, distanceValue, strengthValue);
      } else {
        mqtt->mqtt_update_presence(entry, true, distanceValue, strengthValue);
      }
    }
  }
  virtual void Cleared() {
    display->show_large_distance(0.0, 10, 8);
    display->show_power_line(0);
    if (network_up) {
      mqtt->mqtt_update_presence(false);
    }
  }
};

RadarSensor *radarSensor;


void setup() {
  Serial.begin(115200);

  delay(5000);
#if defined(NO_DISPLAY)
  Serial.printf("No display\n");
  display = new Display{};
#else
  Serial.printf("SS display display\n");
  display = new SSDisplay{};
#endif
  load_settings();
  display->scroller_start();
  mqtt = new RadarMqtt{display};

  auto *lep = new LocalEP{display, mqtt};

#if 0
  if (!strcmp(radar_module, "ld2411")) {
    radarSensor = new LD2411{lep};
    Serial.printf("LD2411  radar module type '%s'\n", radar_module);
  } else
#endif
  if (!strcmp(radar_module, "ld1125")) {
    radarSensor = new LD1125{lep};
    Serial.printf("LD1125  radar module type '%s'\n", radar_module);
  } else if (!strcmp(radar_module, "ld2410")) {
    radarSensor = new LD2410{lep};
    Serial.printf("LD2410  radar module type '%s'\n", radar_module);
  } else {
    display->taf("Undefined radar module type '%s'\n", radar_module);
    Serial.printf("Undefined radar module type '%s' using LD2410\n", radar_module);
    // Using any one as not to have null calls.
    radarSensor = new LD2410{lep};
  }
  wifi_connect(display);
  DateTime.setTimeZone("AEST-10AEDT,M10.1.0,M4.1.0/3");
  DateTime.begin(/* timeout param */);
  if (!DateTime.isTimeValid()) {
    display->taf("Failed to get time from server\n");
  }
  network_up = true;
}

void loop() {
  if (radarSensor) {
    radarSensor->process();
  }
  display->scroller_run();
  mqtt->handle();
  display->display();
  ArduinoOTA.handle();
  delay(5);
}
