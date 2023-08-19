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

  void flush() {
    flushBuffer();
  }


  int available() {
    return 0;  // Not implemented for output stream
  }

  int read() {
    return -1; // Not implemented for output stream
  }

  int peek() {
    return -1; // Not implemented for output stream
  }

};

LcdDebugStream lcdDebugStream(scroller);

WiFiManager wifiManager(lcdDebugStream);

void configModeCallback (WiFiManager *myWiFiManager) {
  scroller.taf("Entered config mode");
  scroller.taf("AP IP %s", WiFi.softAPIP().toString().c_str());
  scroller.taf("portal ssid %s", myWiFiManager->getConfigPortalSSID().c_str());
}


void wifi_connect() {
  scroller.taf("Wifi connect");
  wifiManager.setAPCallback(configModeCallback);
  auto res = wifiManager.autoConnect("presencedetector");
  if (!res) {
    scroller.taf("Wifi failed to connect");
  } else {
    scroller.taf("Connected");
  }
  DateTime.begin(/* timeout param */);
  if (!DateTime.isTimeValid()) {
    scroller.taf("Failed to get time from server");
  } else {
    scroller.taf("Datetime set");
  }
}

