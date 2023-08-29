#include <Arduino.h>
#include <Stream.h>

#include <ESPDateTime.h>
#include <WiFiManager.h>
#include <ArduinoOTA.h>
#include <ArduinoJson.h>
#include <FS.h>
#include <SPIFFS.h>

#include "scroller.h"
#include "lwifi.h"
#include "lcd_debug.h"

constexpr int PROG_BUTTON_PIN = 0; // GPIO0

LcdDebugStream lcdDebugStream(scroller);
WiFiManager wifiManager(lcdDebugStream);


// char mqtt_server[64] = "mqtt2.mianos.com"; // Default MQTT server
bool shouldSaveConfig = false;
char mqtt_server[40] = "mqtt";
char mqtt_port[6] = "1883";
char mqtt_topic[40] = "presence/radar";
char sensor_name[40] = "sensorA";
char radar_module[30] = "undefined";


// Saves custom parameters to /config.json on SPIFFS
void save_settings() {
  DynamicJsonDocument doc(1024);

  doc["mqtt_server"] = mqtt_server;
  doc["mqtt_port"] = mqtt_port;
  doc["mqtt_topic"] = mqtt_topic;
  doc["sensor_name"] = sensor_name;
  doc["radar_module"] = radar_module;

  File file = SPIFFS.open("/config.json", "w");
  if (!file) {
    scroller.taf("Failed to open config file for writing\n");
    scroller.force();
    return;
  }
  if (serializeJson(doc, file) == 0) {
    scroller.taf("Failed to write to file\n");
    scroller.force();
  }
  file.close();
}

// Loads custom parameters from /config.json on SPIFFS
void load_settings() {
    if (SPIFFS.exists("/config.json")) {
        File configFile = SPIFFS.open("/config.json", "r");
        
        if (configFile) {
            size_t size = configFile.size();
            std::unique_ptr<char[]> buf(new char[size]);
            configFile.readBytes(buf.get(), size);
            configFile.close();

						DynamicJsonDocument doc(1024);
						DeserializationError error = deserializeJson(doc, buf.get());
						if (error) {
							scroller.taf("Failed to parse config file\n");
              scroller.force();
							return;
						}
						strlcpy(mqtt_server, doc["mqtt_server"], sizeof(mqtt_server));
						strlcpy(mqtt_port, doc["mqtt_port"], sizeof(mqtt_port));
						strlcpy(mqtt_topic, doc["mqtt_topic"], sizeof(mqtt_topic));
						strlcpy(sensor_name, doc["sensor_name"], sizeof(sensor_name));
						strlcpy(radar_module, doc["radar_module"], sizeof(radar_module));
        }
    }
}


void configModeCallback (WiFiManager *myWiFiManager) {
  scroller.taf("Entered config mode\n");
  scroller.taf("AP IP %s\n", WiFi.softAPIP().toString().c_str());
  scroller.taf("portal ssid %\ns", myWiFiManager->getConfigPortalSSID().c_str());
  radar_minimal();
}
// WiFiManager requiring config save callback
void saveConfigCallback () {
    shouldSaveConfig = true;
}


void wifi_connect() {
  if (!SPIFFS.begin(true)) {
      scroller.taf("An error has occurred while mounting SPIFFS\n");
      scroller.force();
      return;
    }
    load_settings();
    wifiManager.setAPCallback(configModeCallback);

    wifiManager.setSaveConfigCallback(saveConfigCallback);

    scroller.taf("press and hold prog now to reset\n");
    radar_minimal();
    pinMode(PROG_BUTTON_PIN, INPUT_PULLUP); // Set the PROG button as an input with pull-up resistor
    // Check if the PROG button is pressed
    if (digitalRead(PROG_BUTTON_PIN) == LOW) {
      wifiManager.resetSettings();
      scroller.taf("Resetting\n");
      radar_minimal();
      ESP.restart(); // Restart to apply changes
    }

    WiFiManagerParameter custom_mqtt_server("server", "mqtt server", mqtt_server, 40);
    wifiManager.addParameter(&custom_mqtt_server);
    WiFiManagerParameter custom_mqtt_port("port", "mqtt port", mqtt_port, 6);
    wifiManager.addParameter(&custom_mqtt_port);
    WiFiManagerParameter custom_mqtt_topic("topic", "mqtt topic", mqtt_topic, 40);
    wifiManager.addParameter(&custom_mqtt_topic);
    WiFiManagerParameter custom_sensor_name("sensor_name", "sensor name", sensor_name, 40);
    wifiManager.addParameter(&custom_sensor_name);
    WiFiManagerParameter custom_radar_module("radar_module", "radar module", radar_module, 30);
    wifiManager.addParameter(&custom_radar_module);
        
    // try to connect or fallback to ESP+ChipID AP config mode.
    if (!wifiManager.autoConnect()) {
        // reset and try again, or maybe put it to deep sleep
        scroller.taf("restarting\n");
        radar_minimal();
        ESP.restart();
    }
    
    strcpy(mqtt_server, custom_mqtt_server.getValue());
    strcpy(mqtt_port, custom_mqtt_port.getValue());
    strcpy(mqtt_topic, custom_mqtt_topic.getValue());
    strcpy(sensor_name, custom_sensor_name.getValue());
    
    // if we went through config, we should save our changes.
    if (shouldSaveConfig)
        save_settings();
}

