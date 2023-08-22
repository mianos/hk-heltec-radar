#include <Arduino.h>
#include <Stream.h>

#include <ESPDateTime.h>
#include <WiFiManager.h>
#include <ArduinoOTA.h>

#include "scroller.h"
#include "lwifi.h"

class LcdDebugStream : public Stream {
private:
  ScrollingText& _scroller;
  static const size_t _bufferSize = 128;
  char _buffer[_bufferSize];
  size_t _bufferIndex = 0;

  void flushBuffer() {
    if (_bufferIndex > 0) {
      _buffer[_bufferIndex] = '\0';
      _scroller.taf("%s", _buffer);
      _bufferIndex = 0;
    }
  }
public:
  LcdDebugStream(ScrollingText& scroller) : _scroller(scroller) {}

  size_t write(uint8_t data) {
    if (data == '\n') {
      flushBuffer();
    } else {
      _buffer[_bufferIndex++] = data;
      if (_bufferIndex >= _bufferSize - 1) {
        flushBuffer();
      }
    }
    return 1;
  }

  size_t write(const uint8_t *buffer, size_t size) {
    for (size_t i = 0; i < size; i++) {
      write(buffer[i]);
    }
    return size;
  }

  void flush() { flushBuffer(); }
  int available() { return 0; }
  int read() { return -1; }
  int peek() { return -1; }
};


constexpr int PROG_BUTTON_PIN = 0; // GPIO0

LcdDebugStream lcdDebugStream(scroller);
WiFiManager wifiManager(lcdDebugStream);

void configModeCallback (WiFiManager *myWiFiManager) {
  scroller.taf("Entered config mode\n");
  scroller.taf("AP IP %s\n", WiFi.softAPIP().toString().c_str());
  scroller.taf("portal ssid %\ns", myWiFiManager->getConfigPortalSSID().c_str());
}

char mqtt_server[64] = "mqtt2.mianos.com"; // Default MQTT server

void wifi_connect() {
  wifiManager.setAPCallback(configModeCallback);

  WiFiManagerParameter custom_mqtt_server("server", "MQTT Server", mqtt_server, 40);
  wifiManager.addParameter(&custom_mqtt_server);

  pinMode(PROG_BUTTON_PIN, INPUT_PULLUP); // Set the PROG button as an input with pull-up resistor
  // Check if the PROG button is pressed
  if (digitalRead(PROG_BUTTON_PIN) == LOW) {
    wifiManager.resetSettings();
    ESP.restart(); // Restart to apply changes
  }

  auto res = wifiManager.autoConnect("presencedetector");
  if (!res) {
    scroller.taf("Wifi failed to connect\n");
  } else {
    scroller.taf("Connected\n");
  }
  DateTime.begin(/* timeout param */);
  if (!DateTime.isTimeValid()) {
    scroller.taf("Failed to get time from server\n");
  } else {
    scroller.taf("Datetime set\n");
  }
}

