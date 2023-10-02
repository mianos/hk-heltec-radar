#pragma once
#include <WiFi.h>
#include <PubSubClient.h>

#include "display.h"
#include "radar.h"
#include "settings.h"

struct RadarMqtt {
  WiFiClient espClient;
  PubSubClient client;
  RadarSensor* radar = nullptr;
  Display* display;
  SettingsManager *settings;
  int tracking_interval = 0;

  unsigned long lastTimeCalled = 0;  // Store the last time the function was called
  const unsigned long interval = 250;  // Interval in milliseconds (1000 ms / 4 = 250 ms)

  void callback(char* topic_str, byte* payload, unsigned int length);
  RadarMqtt(Display* display, SettingsManager *settings);

  bool reconnect();
  void handle();

  void mqtt_update_presence(bool entry, const Value *vv = nullptr);
  void mqtt_track(const Value *vv);
};

