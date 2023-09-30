#pragma once
#include <memory>
#include <vector>
#include "radar.h"

class LD1125 : public RadarSensor {
  HardwareSerial SerialR;
public:
  LD1125(EventProc* ep) : RadarSensor(ep), SerialR(1) {
    SerialR.begin(115200, SERIAL_8N1, LD_RX, LD_TX);
    for (auto kk = 0; kk < 3; kk++) {
      delay(500);
      for (auto ii = 0; ii < 10000; ii++) {
        if (!SerialR.available()) {
          break;
        }
        char c = (char)SerialR.read();
        Serial.printf("%c", c);
      }
      SerialR.printf("test_mode=1\r\n");
      Serial.printf("trying test mode\n");
    }
  }
private:
    String distance, strength, rtype;
public:

  std::vector<std::unique_ptr<Value>>   get_decoded_radar_data() {
    std::vector<std::unique_ptr<Value>> valuesList;
    static enum State {
      WAIT,
      OCC_MOV,
      DIS,
      STR
    } state = WAIT;
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
            rtype += c;
          }
          break;
        case OCC_MOV:
          rtype += c;
          if (rtype == "occ," || rtype == "mov,") {
            state = DIS;
          } else if (rtype.length() > 4) {
            rtype = "";
            state = WAIT;
          }
          break;
        case DIS:
          if (c == ',') {
            state = STR;
          } else if (c != ' ' && c != 'd' && c != 'i' && c != 's' && c != '=') {
            distance += c;
          }
          break;
        case STR:
          if (c == '\n') {
            String retType = rtype.substring(0, rtype.length() - 1); // Removing the comma
            rtype = "";

            auto power_f = 0.0;
            if (strength != "") {
              power_f = strength.toFloat() / 10.0;
              strength = "";
            }
            auto distance_f = distance.toFloat();

            if (retType == "occ") {
              valuesList.push_back(std::unique_ptr<Value>(new Occupancy(distance_f, power_f)));
            } else {
              valuesList.push_back(std::unique_ptr<Value>(new Movement(distance_f, power_f)));
            }
            distance = "";
            state = WAIT;
//            val->print();
            return valuesList;

          } else if (c != ' ' && c != 's' && c != 't' && c != 'r' && c != '=') {
            strength += c;
          }
          break;
      }
    }
    return valuesList;
  }
};

