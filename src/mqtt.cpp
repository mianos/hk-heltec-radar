#include <ESPDateTime.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include <StringSplitter.h>

#include "radar.h"
#include "mqtt.h"



void RadarMqtt::callback(char *topic_str, byte *payload, unsigned int length) {
  //scroller.taf("Message arrived, topic '%s'\n", topic_str);
  auto topic = String(topic_str);

  auto splitter = StringSplitter(topic, '/', 4); //string_to_split, delimiter, limit)
  int itemCount = splitter.getItemCount();
  if (itemCount < 3) {
    scroller.taf("Item count less than 3 %d '%s'\n", itemCount, topic_str);
    return;
  }
#if 1
  for (int i = 0; i < itemCount; i++) {
    String item = splitter.getItemAtIndex(i);
    Serial.printf("Item '%s' index %d\n", item.c_str(), i);
  }
#endif
  if (splitter.getItemAtIndex(0) == "cmnd") {
    DynamicJsonDocument jpl(1024);
    auto err = deserializeJson(jpl, payload, length);
    if (err) {
      scroller.taf("deserializeJson() failed: '%s'\n", err.c_str());
      return;
    }
    String output;
    serializeJson(jpl, output);
    scroller.taf("payload '%s'\n", output.c_str());
    auto dest = splitter.getItemAtIndex(2);
    if (dest == "ranges") {
      if (jpl.containsKey("report")) {
        report_ranges = jpl["report"].as<bool>();
        scroller.taf("reporting  ranges '%d'\n", report_ranges);
      }
    }
  }
}

RadarMqtt::RadarMqtt(ScrollingText& scroller) : client(espClient), scroller(scroller) {
  client.setServer(mqtt_server, 1883);
  client.setCallback([this](char *topic_str, byte *payload, unsigned int length) {
                callback(topic_str, payload, length);
  });
}
void RadarMqtt::add_radar(LD2125 *new_radar) {
  radar = new_radar;
}

void RadarMqtt::reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    scroller.taf("Attempting MQTT connection...\n");
    // Create a random client ID
    String clientId = String(dname) + '-' + String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str())) {

      String cmnd_topic = String("cmnd/") + dname + "/#";
      client.subscribe(cmnd_topic.c_str());

      scroller.taf("mqtt connected\n");
      // Once connected, publish an announcement...

      StaticJsonDocument<200> doc;
      doc["version"] = 2;
      doc["time"] = DateTime.toISOString();
      String status_topic = "tele/" + String(dname) + "/init";
      String output;
      serializeJson(doc, output);
      client.publish(status_topic.c_str(), output.c_str());
    } else {
      scroller.taf("failed, rc=%d, sleeping 5 seconds\n", client.state());
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
  //lv_obj_add_state(ui_mqtton, LV_STATE_CHECKED); 
}


void RadarMqtt::handle() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
}

void RadarMqtt::send() {
  if (!client.connected()) {
    reconnect();
  }
  if (report_ranges) {
    StaticJsonDocument<200> sdoc;
/*      dr.build_json_data(sdoc);
    String o2;
    serializeJson(sdoc, o2);
    String t2 = String("tele/") + dname + "/ranges";
    client.publish(t2.c_str(), o2.c_str()); */
  }
}


void RadarMqtt::mqtt_update_presence(bool state, float distance, float strengthValue) {
  if (!client.connected()) {
    reconnect();
  }
  StaticJsonDocument<200> doc;
  doc["state"] = state;
  if (distance != 0.0) {
    doc["distance"] =  (int)(distance * 100 + 0.5) / 100.0;
  }
  doc["time"] = DateTime.toISOString();
  String status_topic = "tele/" + String(dname) + "/presence";
  String output;
  serializeJson(doc, output);
  client.publish(status_topic.c_str(), output.c_str());
}
