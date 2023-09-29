#pragma once
#include <memory>
#include <vector>
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
            std::unique_ptr<Value> val;
            String retType = rtype.substring(0, rtype.length() - 1); // Removing the comma
            rtype = "";

            if (retType == "occ") {
              val.reset(new Occupancy());
            } else {
              val.reset(new Movement());
            }
            val->value = distance.toFloat();
            distance = "";
            if (strength != "") {
              val->power = strength.toFloat() / 10.0; // Decode strength here
              strength = "";
            }
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

