#include <Arduino.h>
#include <Stream.h>

#include <ESPDateTime.h>
#include <WiFiManager.h>
#include <ArduinoOTA.h>
#include <ArduinoJson.h>
#include <FS.h>
#include <SPIFFS.h>

#include "display.h"
#include "lwifi.h"
#include "lcd_debug.h"

constexpr int PROG_BUTTON_PIN = 0; // GPIO0



// char mqtt_server[64] = "mqtt2.mianos.com"; // Default MQTT server
bool shouldSaveConfig = false;
char mqtt_server[40] = "mqtt";
char mqtt_port[6] = "1883";
char sensor_name[40] = "radar/home";
char radar_module[30] = "undefined";


// Saves custom parameters to /config.json on SPIFFS
void save_settings() {
  DynamicJsonDocument doc(1024);

  doc["mqtt_server"] = mqtt_server;
  doc["mqtt_port"] = mqtt_port;
  doc["sensor_name"] = sensor_name;
  doc["radar_module"] = radar_module;

  File file = SPIFFS.open("/config.json", "w");
  if (!file) {
    display->taf("Failed to open config file for writing\n");
    display->scroll_now();
    return;
  }
  if (serializeJson(doc, file) == 0) {
    display->taf("Failed to write to file\n");
    display->scroll_now();
  }
  file.close();
}

// Loads custom parameters from /config.json on SPIFFS
void load_settings() {
  if (!SPIFFS.begin(true)) {
      Serial.printf("Failed to initialise SPIFFS\n");
      display->taf("Failed to initialise SPIFFS\n");
      display->scroll_now();
      return;
    }
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
							display->taf("Failed to parse config file\n");
              display->scroll_now();
							return;
						}
						strlcpy(mqtt_server, doc["mqtt_server"], sizeof(mqtt_server));
						strlcpy(mqtt_port, doc["mqtt_port"], sizeof(mqtt_port));
						strlcpy(sensor_name, doc["sensor_name"], sizeof(sensor_name));
						strlcpy(radar_module, doc["radar_module"], sizeof(radar_module));
        }
    }
}


void configModeCallback (WiFiManager *myWiFiManager) {
  display->taf("Config mode AP IP %s portal ssid %s \n", WiFi.softAPIP().toString().c_str(),
                                                         myWiFiManager->getConfigPortalSSID().c_str());
  display->scroll_now();
}
// WiFiManager requiring config save callback
void saveConfigCallback () {
    shouldSaveConfig = true;
}


void wifi_connect(Display* display) {
    WiFiManager wifiManager{*(new LcdDebugStream{display})};
    wifiManager.setAPCallback(configModeCallback);

    wifiManager.setSaveConfigCallback(saveConfigCallback);

    display->taf("press and hold prog now to reset .... \n");
    display->scroll_now();
    pinMode(PROG_BUTTON_PIN, INPUT_PULLUP); // Set the PROG button as an input with pull-up resistor
    // Check if the PROG button is pressed
    if (digitalRead(PROG_BUTTON_PIN) == LOW) {
      wifiManager.resetSettings();
      display->taf("Resetting\n");
      display->scroll_now();
      ESP.restart(); // Restart to apply changes
    }

    WiFiManagerParameter custom_mqtt_server("server", "mqtt server", mqtt_server, 40);
    wifiManager.addParameter(&custom_mqtt_server);
    WiFiManagerParameter custom_mqtt_port("port", "mqtt port", mqtt_port, 6);
    wifiManager.addParameter(&custom_mqtt_port);
    WiFiManagerParameter custom_sensor_name("sensor_name", "sensor name", sensor_name, 40);
    wifiManager.addParameter(&custom_sensor_name);
    WiFiManagerParameter custom_radar_module("radar_module", "radar module", radar_module, 30);
    wifiManager.addParameter(&custom_radar_module);
        
    // try to connect or fallback to ESP+ChipID AP config mode.
    if (!wifiManager.autoConnect()) {
        // reset and try again, or maybe put it to deep sleep
        display->taf("restarting 2nd time\n");
        display->scroll_now();
        ESP.restart();
    }
    
    strcpy(mqtt_server, custom_mqtt_server.getValue());
    strcpy(mqtt_port, custom_mqtt_port.getValue());
    strcpy(sensor_name, custom_sensor_name.getValue());
    strcpy(radar_module, custom_radar_module.getValue());
    
    // if we went through config, we should save our changes.
    if (shouldSaveConfig)
        save_settings();

  ArduinoOTA
    .onStart([]() {
      String type;
      if (ArduinoOTA.getCommand() == U_FLASH)
        type = "sketch";
      else
        type = "filesystem";

      Serial.println("Start updating " + type);
    })
    .onEnd([]() {
      Serial.println("\nEnd");
    })
    .onProgress([](unsigned int progress, unsigned int total) {
      Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
    })
    .onError([](ota_error_t error) {
      Serial.printf("Error[%u]: ", error);
      if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
      else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
      else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
      else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
      else if (error == OTA_END_ERROR) Serial.println("End Failed");
    });

  ArduinoOTA.begin();
}

