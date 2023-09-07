#pragma once
#include <WiFi.h>
#include <PubSubClient.h>

#include "display.h"
#include "radar.h"

struct RadarMqtt {
  WiFiClient espClient;
  PubSubClient client;
  bool report_ranges = false;
  RadarSensor* radar = nullptr; // Use nullptr instead of 0 for pointer initialization
  Display* display;

  unsigned long lastTimeCalled = 0;  // Store the last time the function was called
  const unsigned long interval = 250;  // Interval in milliseconds (1000 ms / 4 = 250 ms)

  void callback(char* topic_str, byte* payload, unsigned int length);
  RadarMqtt(Display* display);

  bool reconnect();
  void handle();

  void mqtt_update_presence(bool entry, bool other = false, float distance = 0.0f, float strengthValue = 0.0f);
};

