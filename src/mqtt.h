#pragma once
#include <WiFi.h>
#include <PubSubClient.h>

#include "scroller.h"

struct RadarMqtt {
  ScrollingText& scroller;
  WiFiClient espClient;
  PubSubClient client;
  bool report_ranges = true;

  void callback(char *topic_str, byte *payload, unsigned int length);
  RadarMqtt(ScrollingText& scroller);

  void reconnect();

  void handle();
  void send();


  void mqtt_update_presence(bool state, int iseen);
};

