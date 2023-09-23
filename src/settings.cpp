#include <SPIFFS.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>

#include "settings.h"

void SettingsManager::serveStaticFile(const String& path) {
    server.serveStatic(path.c_str(), SPIFFS, path.c_str());
}

void SettingsManager::setupServer() {
    if (!SPIFFS.begin(true)) {
        Serial.println("An error occurred while mounting SPIFFS");
        return;
    }

    // Serve static files
    serveStaticFile("/style.css");
    server.serveStatic("/time", SPIFFS, "/time.html");

    server.on("/status", HTTP_GET, [](AsyncWebServerRequest *request) {
      StaticJsonDocument<200> jsonDoc;  // Create a StaticJsonDocument object. Adjust the capacity as needed.
      jsonDoc["time"] = "NOW";          // Insert the time data into the JSON object.

      String jsonResponse;
      serializeJson(jsonDoc, jsonResponse);  // Serialize the JSON object to a string.

      request->send(200, "application/json", jsonResponse);  // Send the JSON response.
    });

    // Server routing setup
    server.on("/config", HTTP_GET, [&](AsyncWebServerRequest* request) {
        message.clear();
        request->send(SPIFFS, "/config.html", String(), false, [&](const String& var) {
            return processor(var);
        });
    });

    server.serveStatic("/", SPIFFS, "/")
        .setDefaultFile("home.html")
        .setTemplateProcessor([&](const String& var) {
            return processor(var);
        });

    server.on("/save", HTTP_POST, [&](AsyncWebServerRequest* request) {
        updateAndSaveSettings(request);
        message = "Saved";
        request->redirect("/");
    });

    server.on("/clear", HTTP_GET, [&](AsyncWebServerRequest* request) {
        loadDefaultSettings();
        saveSettings();
        message = "Cleared";
        request->redirect("/");
    });

    server.begin();
    Serial.printf("Server started\n");
}

void SettingsManager::updateAndSaveSettings(AsyncWebServerRequest* request) {
    mqttServer = request->arg("MQTT_SERVER");
    radarType = request->arg("RADAR_TYPE");
    mqttPort = request->arg("MQTT_PORT");
    sensorName = request->arg("SENSOR_NAME");
    saveSettings();
}

void SettingsManager::loadSettings() {
    if (!SPIFFS.begin(true)) {
        Serial.println("An error occurred while mounting SPIFFS");
        return;
    }

    File configFile = SPIFFS.open("/config.json", "r");
    if (!configFile) {
        Serial.println("Failed to open config file. Loading default settings.");
        loadDefaultSettings();
        return;
    }

    DynamicJsonDocument doc(1024);
    DeserializationError error = deserializeJson(doc, configFile);
    if (error) {
        Serial.println("Failed to deserialize config file. Loading default settings.");
        loadDefaultSettings();
        return;
    }

    mqttServer = doc["mqtt_server"].as<String>();
    radarType = doc["radar_type"].as<String>();
    mqttPort = doc["mqtt_port"].as<String>();
    sensorName = doc["sensor_name"].as<String>();
    configFile.close();
}

void SettingsManager::saveSettings() {
    DynamicJsonDocument doc(1024);
    doc["mqtt_server"] = mqttServer;
    doc["radar_type"] = radarType;
    doc["mqtt_port"] = mqttPort;
    doc["sensor_name"] = sensorName;

    File configFile = SPIFFS.open("/config.json", "w");
    if (!configFile) {
        Serial.println("Failed to open config file for writing");
        return;
    }

    serializeJson(doc, configFile);
    configFile.close();
}

void SettingsManager::loadDefaultSettings() {
    mqttServer = "default_server";
    radarType = "default_type";
    mqttPort = "1883";
    sensorName = "ldnoradar";
}

// Implementation of private methods
String SettingsManager::processor(const String& var) {
    if (var == "MQTT_SERVER") {
        return mqttServer;
    } else if (var == "RADAR_TYPE") {
        return radarType;
    } else if (var == "MQTT_PORT") {
        return mqttPort;
    } else if (var == "SENSOR_NAME") {
        return sensorName;
    } else if (var == "MESSAGE") {
        return message;
    }
    return String();
}
