#include <ESPDateTime.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include <StringSplitter.h>

#include "mqtt.h"
#include "lwifi.h"

void RadarMqtt::callback(char* topic_str, byte* payload, unsigned int length) {
  auto topic = String(topic_str);
  auto splitter = StringSplitter(topic, '/', 4);
  int itemCount = splitter.getItemCount();
  if (itemCount < 3) {
    display->taf("Item count less than 3 %d '%s'\n", itemCount, topic_str);
    return;
  }
#if 0
  for (int i = 0; i < itemCount; i++) {
    String item = splitter.getItemAtIndex(i);
    Serial.println("Item @ index " + String(i) + ": " + String(item));
  }
  Serial.printf("command '%s'\n", splitter.getItemAtIndex(itemCount - 1).c_str());
#endif
  
  if (splitter.getItemAtIndex(0) == "cmnd") {
    DynamicJsonDocument jpl(1024);
    auto err = deserializeJson(jpl, payload, length);
    if (err) {
      display->taf("deserializeJson() failed: '%s'\n", err.c_str());
      return;
    }
    String output;
    serializeJson(jpl, output);
    auto dest = splitter.getItemAtIndex(itemCount - 1);
    if (dest == "distance") {
      if (jpl.containsKey("report")) {
        report_ranges = jpl["report"].as<bool>();
        display->taf("reporting  ranges: %s\n", report_ranges ? "true" : "false");
      }
    }
  }
}

RadarMqtt::RadarMqtt(Display* display) : client(espClient), display(display) {
  client.setServer(mqtt_server, atoi(mqtt_port));
  client.setCallback([this](char* topic_str, byte* payload, unsigned int length) {
    callback(topic_str, payload, length);
  });
}


bool RadarMqtt::reconnect() {
  display->taf("Attempting MQTT connection...\n");
  String clientId = String(sensor_name) + '-' + String(random(0xffff), HEX);
  if (client.connect(clientId.c_str())) {
    String cmnd_topic = String("cmnd/") + sensor_name + "/#";
    client.subscribe(cmnd_topic.c_str());
    display->taf("mqtt connected\n");
    StaticJsonDocument<200> doc;
    doc["version"] = 3;
    doc["time"] = DateTime.toISOString();
    String status_topic = "tele/" + String(sensor_name) + "/init";
    String output;
    serializeJson(doc, output);
    client.publish(status_topic.c_str(), output.c_str());
    return true;
  } else {
    display->taf("failed to connect to %s\n", mqtt_server);
    display->scroll_now();
    return false;
  }
}

void RadarMqtt::handle() {
  if (!client.connected()) {
    if (!reconnect()) {
      return;
    }
  }
  client.loop();
}


void RadarMqtt::mqtt_update_presence(bool entry, bool other, float distance, float strengthValue) {
  if (!client.connected()) {
    if (!reconnect()) {
      return;
    }
  }
  if (other && !report_ranges) {
    return;
  }
  unsigned long currentTime = millis();
  if (currentTime - lastTimeCalled < interval) {
    return;
  }

  StaticJsonDocument<200> doc;
  doc["entry"] = entry || other;
  if (distance != 0.0) {
    doc["distance"] = (int)(distance * 100.0 + 0.5) / 100.0;
  }
  if (strengthValue != 0.0) {
    doc["strength"] = (int)(strengthValue * 10.0 + 0.5) / 10.0;
  }
  doc["time"] = DateTime.toISOString();
  String status_topic = "tele/" + String(sensor_name) + "/presence";
  String output;
  serializeJson(doc, output);
  client.publish(status_topic.c_str(), output.c_str());
  lastTimeCalled = currentTime;
}

