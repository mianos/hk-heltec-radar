#include <ESPDateTime.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include <StringSplitter.h>

#include "mqtt.h"

void RadarMqtt::callback(char* topic_str, byte* payload, unsigned int length) {
  auto topic = String(topic_str);
  auto splitter = StringSplitter(topic, '/', 4);
  int itemCount = splitter.getItemCount();
  if (itemCount < 3) {
    scroller.taf("Item count less than 3 %d '%s'\n", itemCount, topic_str);
    return;
  }

  if (splitter.getItemAtIndex(0) == "cmnd") {
    DynamicJsonDocument jpl(1024);
    auto err = deserializeJson(jpl, payload, length);
    if (err) {
      scroller.taf("deserializeJson() failed: '%s'\n", err.c_str());
      return;
    }
    String output;
    serializeJson(jpl, output);
    auto dest = splitter.getItemAtIndex(2);
    if (dest == "ranges") {
      if (jpl.containsKey("report")) {
        report_ranges = jpl["report"].as<bool>();
        scroller.taf("reporting  ranges: %s\n", report_ranges ? "true" : "false");
      }
    }
  }
}

RadarMqtt::RadarMqtt(ScrollingText& scroller) : client(espClient), scroller(scroller) {
  client.setServer(mqtt_server, 1883);
  client.setCallback([this](char* topic_str, byte* payload, unsigned int length) {
    callback(topic_str, payload, length);
  });
}


void RadarMqtt::reconnect() {
  while (!client.connected()) {
    scroller.taf("Attempting MQTT connection...\n");
    String clientId = String(dname) + '-' + String(random(0xffff), HEX);
    if (client.connect(clientId.c_str())) {
      String cmnd_topic = String("cmnd/") + dname + "/#";
      client.subscribe(cmnd_topic.c_str());
      scroller.taf("mqtt connected\n");
      StaticJsonDocument<200> doc;
      doc["version"] = 3;
      doc["time"] = DateTime.toISOString();
      String status_topic = "tele/" + String(dname) + "/init";
      String output;
      serializeJson(doc, output);
      client.publish(status_topic.c_str(), output.c_str());
    } else {
      scroller.taf("failed, rc=%d, sleeping 5 seconds\n", client.state());
      delay(5000);
    }
  }
}

void RadarMqtt::handle() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
}

void RadarMqtt::mqtt_update_presence(bool entry, bool other, float distance, float strengthValue) {
  if (!client.connected()) {
    reconnect();
  }
  if (other && !report_ranges) {
    return;
  }
  StaticJsonDocument<200> doc;
  doc["entry"] = entry;
  if (distance != 0.0) {
    doc["distance"] = (int)(distance * 100.0 + 0.5) / 100.0;
  }
  if (strengthValue != 0.0) {
    doc["strength"] = (int)(strengthValue * 10.0 + 0.5) / 10.0;
  }
  doc["time"] = DateTime.toISOString();
  String status_topic = "tele/" + String(dname) + "/presence";
  String output;
  serializeJson(doc, output);
  client.publish(status_topic.c_str(), output.c_str());
}

