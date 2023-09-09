#pragma once
#include "radar.h"

class LD1125 : public RadarSensor {
  HardwareSerial SerialR;
public:
  LD1125(EventProc* ep) : RadarSensor(ep), SerialR(1) {
    SerialR.begin(115200, SERIAL_8N1, LD_RX, LD_TX);
    for (auto kk = 0; kk < 5; kk++) {
      for (auto ii = 0; ii < 10000; ii++) {
        if (!SerialR.available()) {
          break;
        }
        char c = (char)SerialR.read();
      }
      SerialR.printf("test_mode=1\r\n");
    }
  }

  virtual String get_decoded_radar_data() {
    static enum State {
      WAIT,
      OCC_MOV,
      DIS,
      STR
    } state = WAIT;
    static String distance, strength;
    const unsigned long timeoutDuration = 5000; 

    unsigned long loopStartTime = millis();
    while (SerialR.available()) {
      if (millis() - loopStartTime >= timeoutDuration) {
        Serial.println("Timeout occurred! Exiting the loop.");
        break; // Exit the loop
      }

      char c = (char)SerialR.read();
      switch (state) {
        case WAIT:
          if (c == 'o' || c == 'm') {
            state = OCC_MOV;
            type += c;
          }
          break;
        case OCC_MOV:
          type += c;
          if (type == "occ," || type == "mov,") {
            state = DIS;
          } else if (type.length() > 4) {
            type = "";
            state = WAIT;
          }
          break;
        case DIS:
          if (c == ',') {
            state = STR;
            distanceValue = distance.toFloat(); // Decode distance here
            distance = "";
          } else if (c != ' ' && c != 'd' && c != 'i' && c != 's' && c != '=') {
            distance += c;
          }
          break;
        case STR:
          if (c == '\n') {
            if (strength != "") {
              strengthValue = strength.toFloat() / 10.0; // Decode strength here
              strength = "";
            }
            String retType = type.substring(0, type.length() - 1); // Removing the comma
            type = "";
            state = WAIT;
            return retType; // Return the detected type (occ or mov)
          } else if (c != ' ' && c != 's' && c != 't' && c != 'r' && c != '=') {
            strength += c;
          }
          break;
      }
    }
    return ""; // Return empty string if no complete message detected
  }
};

