#include <ESPDateTime.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include <StringSplitter.h>

#include "settings.h"
#include "mqtt.h"

double rr(double value) {
    return (int)(value * 100 + 0.5) / 100.0;
}

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
    if (dest == "tracking") {
      if (jpl.containsKey("interval")) {
        settings->tracking = jpl["interval"].as<int>();
        display->taf("Setting tracking interval to %d\n", settings->tracking);
      }
    }
  }
}


RadarMqtt::RadarMqtt(Display* display, SettingsManager *settings)
    : client(espClient), display(display), settings(settings) {
  client.setServer(settings->mqttServer.c_str(), atoi(settings->mqttPort.c_str()));
  client.setCallback([this](char* topic_str, byte* payload, unsigned int length) {
    callback(topic_str, payload, length);
  });
}


bool RadarMqtt::reconnect() {
  display->taf("Attempting MQTT connection...\n");
  String clientId = settings->sensorName + '-' + String(random(0xffff), HEX);
  if (client.connect(clientId.c_str())) {
    String cmnd_topic = String("cmnd/") + settings->sensorName + "/#";
    client.subscribe(cmnd_topic.c_str());
    display->taf("mqtt connected\n");
    display->scroll_now();
    StaticJsonDocument<200> doc;
    doc["version"] = 4;
    doc["time"] = DateTime.toISOString();
    doc["hostname"] = WiFi.getHostname();
    doc["ip"] = WiFi.localIP().toString();
    String status_topic = "tele/" + settings->sensorName + "/init";
    String output;
    serializeJson(doc, output);
    client.publish(status_topic.c_str(), output.c_str());
    return true;
  } else {
    display->taf("failed to connect to %s\n", settings->mqttServer.c_str());
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


void RadarMqtt::mqtt_update_presence(bool entry, const Value *vv) {
  if (!client.connected()) {
    if (!reconnect()) {
      return;
    }
  }
  StaticJsonDocument<300> doc;
  doc["entry"] = entry;
  doc["time"] = DateTime.toISOString();
  if (entry) {
    vv->toJson(doc);
  }
  String status_topic = "tele/" + settings->sensorName + "/presence";
  String output;
  serializeJson(doc, output);
  client.publish(status_topic.c_str(), output.c_str());
}

void RadarMqtt::mqtt_track(const Value *vv) {
  if (!client.connected()) {
    if (!reconnect()) {
      return;
    }
  }
  StaticJsonDocument<300> doc;
  doc["time"] = DateTime.toISOString();
  vv->toJson(doc);
  String status_topic = "tele/" + settings->sensorName + "/tracking";
  String output;
  serializeJson(doc, output);
  client.publish(status_topic.c_str(), output.c_str());
}

