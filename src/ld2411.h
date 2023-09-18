
#pragma once
#include "radar.h"

class LD2411 : public RadarSensor {
public:
  LD2411(EventProc* ep, int8_t RxPin = 33, int8_t TxPin = 32) : RadarSensor(ep) {
    Serial2.begin(256000, SERIAL_8N1, RxPin, TxPin);
  }

  enum State {
    WAIT_HEADER_1,
    WAIT_HEADER_2,
    READ_TYPE,
    READ_DISTANCE_LSB,
    READ_DISTANCE_MSB,
    READ_END_OF_FRAME_1,
    READ_END_OF_FRAME_2
  };
  
  State state = WAIT_HEADER_1;

  std::unique_ptr<Value>  get_decoded_radar_data() {
    uint8_t type = 0;
    uint16_t distance = 0;

    while (Serial2.available()) {
      uint8_t currentByte = Serial2.read();
      
      switch (state) {
        case WAIT_HEADER_1:
          if (currentByte == 0xAA) {
            state = WAIT_HEADER_2;
          }
          break;
        case WAIT_HEADER_2:
          if (currentByte == 0xAA) {
            state = READ_TYPE;
          } else {
            state = WAIT_HEADER_1;
          }
          break;
        case READ_TYPE:
          type = currentByte;
          if (type != 0x00 && type != 0x01 && type != 0x02) {
            state = WAIT_HEADER_1;
          } else {
            state = READ_DISTANCE_LSB;
          }
          break;
        case READ_DISTANCE_LSB:
          distance = currentByte;
          state = READ_DISTANCE_MSB;
          break;
        case READ_DISTANCE_MSB:
          distance |= static_cast<uint16_t>(currentByte) << 8;
          state = READ_END_OF_FRAME_1;
          break;
        case READ_END_OF_FRAME_1:
          if (currentByte == 0x55) {
            state = READ_END_OF_FRAME_2;
          } else {
            state = WAIT_HEADER_1;
          }
          break;
        case READ_END_OF_FRAME_2:
          if (currentByte == 0x55) {
            state = WAIT_HEADER_1;
            std::unique_ptr<Value> val = nullptr;
            if (type == 1) {
              val.reset(new Occupancy());
              val->value = static_cast<float>(distance) / 100.0;
            } else if (type == 2) {
              val.reset(new Movement());
              val->value = static_cast<float>(distance) / 100.0;
            } else {
              val.reset(new NoTarget());
            }
            return val;
          } else {
            state = WAIT_HEADER_1;
          }
      }
    }
    return nullptr;
  }
};
