#pragma once

#include <SPIFFS.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>

class SettingsManager {
public:
    SettingsManager() : server(80) {
        loadSettings();
        setupServer();
    }

    void serveStaticFile(const String& path);
    void setupServer();
    void updateAndSaveSettings(AsyncWebServerRequest* request);
    void loadSettings();
    void saveSettings();
    void loadDefaultSettings();
    // Accessible settings
    String mqttServer;
    String radarType;
    String mqttPort;
    String sensorName;

private:
    AsyncWebServer server;
    String message;

    String processor(const String& var);
};
