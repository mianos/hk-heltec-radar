#pragma once
#include "radar.h"

class LD1125 : public RadarSensor {
public:
  LD1125(EventProc* ep, int8_t RxPin = 33, int8_t TxPin = 32) : RadarSensor(ep) {
    Serial2.begin(115200, SERIAL_8N1, RxPin, TxPin);
    delay(50);
    Serial2.print("test_mode=1\r\n");
  }

  virtual String decodeRadarDataFSM() {
    static enum State {
      WAIT,
      OCC_MOV,
      DIS,
      STR
    } state = WAIT;
    static String distance, strength;

    while (Serial2.available()) {
      char c = (char)Serial2.read();
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
              strengthValue = strength.toFloat(); // Decode strength here
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

