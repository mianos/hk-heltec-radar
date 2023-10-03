#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ArduinoOTA.h>
#include <ESPDateTime.h>
#include "freertos/queue.h"

#include "radar.h"

#include "display.h"
#include "settings.h"

#include "mqtt.h"
#include "ld2411.h"
#include "ld2410.h"
#include "ld1125.h"
#include "ld306.h"
#include "ld2450.h"
#include "ldnone.h"


Display *display;

RadarMqtt *mqtt;

bool network_up{false};

class LocalEP : public EventProc {
  Display* display;
  RadarMqtt *mqtt;
  uint32_t lastTrackingUpdateTime = 0;
  SettingsManager *settings;

public:
  LocalEP(Display *display, RadarMqtt *mqtt, SettingsManager *settings) : display(display), mqtt(mqtt), settings(settings) {
  }

  virtual void Detected(Value *vv) {
    display->show_large_distance(vv->get_main(), 10, 8);
    display->show_power_line(vv->get_power());
    if (!network_up) {
      return;
    }
    mqtt->mqtt_update_presence(true, vv);
  }

  virtual void Cleared() {
    display->show_large_distance(0.0, 10, 8);
    display->show_power_line(0);
    if (!network_up) {
      return;
    }
    mqtt->mqtt_update_presence(false);
  }

  virtual void TrackingUpdate(Value *vv) {
    if (!network_up) {
      return;
    }
    uint32_t currentMillis = millis();
    if (settings->tracking && (currentMillis - lastTrackingUpdateTime >= settings->tracking)) {
      mqtt->mqtt_track(vv);
      lastTrackingUpdateTime = currentMillis;
    }
  }
};

RadarSensor *radarSensor;
SettingsManager *settings;

void setup() {
  Serial.begin(115200);
#if defined(NO_DISPLAY)
  Serial.printf("No display\n");
  display = new Display{};
#else
  Serial.printf("SS display display\n");
  display = new SSDisplay{};
#endif
  extern void wifi_connect(Display* display);
  wifi_connect(display);

  settings = new SettingsManager{};
  display->scroller_start();

  mqtt = new RadarMqtt{display, settings};
  auto *lep = new LocalEP{display, mqtt, settings};

  if (settings->radarType == "ld2411") {
    radarSensor = new LD2411{lep, settings};
    Serial.printf("LD2411  radar module type '%s'\n", settings->radarType);
  } else if (settings->radarType == "ld1125") {
    radarSensor = new LD1125{lep, settings};
    Serial.printf("LD1125  radar module type '%s'\n", settings->radarType);
  } else if (settings->radarType == "ld2410") {
    radarSensor = new LD2410{lep, settings};
    Serial.printf("LD2410  radar module type '%s'\n", settings->radarType);
  } else if (settings->radarType == "ld306") {
    radarSensor = new LD306{lep, settings};
    Serial.printf("LD306  radar module type '%s'\n", settings->radarType);
  } else if (settings->radarType == "ld2450") {
    radarSensor = new LD2450{lep, settings};
    Serial.printf("LD2450  radar module type '%s'\n", settings->radarType);
  } else {
    display->taf("Undefined radar module type '%s'\n", settings->radarType);
    Serial.printf("Undefined radar module type '%s' using LDNoRadar\n", settings->radarType);
    radarSensor = new LDNoRadar{lep, settings};
  }

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
