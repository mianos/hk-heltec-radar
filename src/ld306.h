#pragma once
#include <vector>
#include "radar.h"

class LD306 : public RadarSensor {
  HardwareSerial SerialR;
  int ldEnablePin;

public:
  LD306(EventProc* ep, int8_t RxPin = 21, int8_t TxPin = 47, int8_t enablePin = 41) : RadarSensor(ep), SerialR(1), ldEnablePin(enablePin)  {
    pinMode(ldEnablePin, OUTPUT);
    digitalWrite(ldEnablePin, LOW); // Initially disable the radar
    SerialR.begin(9600, SERIAL_8N1, RxPin, TxPin);
  }

  enum State {
    WAIT_HEADER_1,
    WAIT_HEADER_2,
    READ_LENGTH,
    READ_ADDRESS,
    READ_SPEED_MSB,
    READ_SPEED_LSB,
    READ_DEBUG,
    READ_CHECKSUM
  };

  State state = WAIT_HEADER_1;
  uint8_t packetLength = 0;
  uint8_t address = 0;
  uint16_t speed = 0;
  uint8_t debugCounter = 0;
  uint8_t checksum = 0;
  uint8_t computed_checksum = 0;

  void enable() {
    digitalWrite(ldEnablePin, HIGH);
  }

  void disable() {
    digitalWrite(ldEnablePin, LOW);
  }

  std::vector<std::unique_ptr<Value>>  get_decoded_radar_data() {
    std::vector<std::unique_ptr<Value>> valuesList;
    while (SerialR.available()) {
      uint8_t currentByte = SerialR.read();
      computed_checksum += currentByte;

      switch (state) {
        case WAIT_HEADER_1:
          if (currentByte == 0x55) state = WAIT_HEADER_2;
          break;
        case WAIT_HEADER_2:
          if (currentByte == 0xA5) state = READ_LENGTH;
          else state = WAIT_HEADER_1;
          break;
        case READ_LENGTH:
          packetLength = currentByte;
          state = READ_ADDRESS;
          break;
        case READ_ADDRESS:
          address = currentByte;
          state = READ_SPEED_MSB;
          break;
        case READ_SPEED_MSB:
          speed = (currentByte << 8);
          state = READ_SPEED_LSB;
          break;
        case READ_SPEED_LSB:
          speed |= currentByte;
          debugCounter = 0;
          state = READ_DEBUG;
          break;
        case READ_DEBUG:
          debugCounter++;
          if (debugCounter >= 6) state = READ_CHECKSUM;
          break;
        case READ_CHECKSUM:
          checksum = currentByte;
          computed_checksum -= checksum;
          computed_checksum &= 0xFF;
          if (checksum == computed_checksum) {
            state = WAIT_HEADER_1;
            computed_checksum = 0;
            if (speed) {
              std::unique_ptr<Value> val(new Speed());
              val->value = static_cast<float>(speed);
              // return val;
              return valuesList;
            } else {
              continue;
            }
          } else {
            state = WAIT_HEADER_1;
            computed_checksum = 0;
          }
          break;
      }
    }
    return valuesList;
  }
};
