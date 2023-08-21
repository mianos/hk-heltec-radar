#pragma once
#include <WiFi.h>
#include <PubSubClient.h>

#include "scroller.h"
#include "ld2125.h"

struct RadarMqtt {
  ScrollingText& scroller;
  WiFiClient espClient;
  PubSubClient client;
  bool report_ranges = false;
  LD2125 *radar;
  static constexpr const char* dname = "radar";
  static constexpr const char* mqtt_server = "mqtt2.mianos.com";

  void callback(char *topic_str, byte *payload, unsigned int length);
  RadarMqtt(ScrollingText& scroller);
  void add_radar(LD2125 *new_radar);

  void reconnect();
  void handle();

  void mqtt_update_presence(bool entry, bool other=false, float distance=0.0, float strengthValue=0.0);
};

