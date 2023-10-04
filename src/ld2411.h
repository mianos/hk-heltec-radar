#pragma once
#include <vector>
#include "radar.h"

class LD2411 : public RadarSensor {
  HardwareSerial SerialR;
public:
  LD2411(EventProc* ep, SettingsManager *settings, int8_t RxPin = LD_RX, int8_t TxPin = LD_TX) : RadarSensor(ep, settings), SerialR(1)  {
    SerialR.begin(256000, SERIAL_8N1, RxPin, TxPin);
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

  std::vector<std::unique_ptr<Value>>   get_decoded_radar_data() {
    std::vector<std::unique_ptr<Value>> valuesList;
    uint8_t type = 0;
    uint16_t distance = 0;

    while (SerialR.available()) {
      uint8_t currentByte = SerialR.read();
      
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
            if (type == 1) {
              valuesList.push_back(std::unique_ptr<Value>(
                    new Occupancy(static_cast<float>(distance) / 100.0)));
            } else if (type == 2) {
              valuesList.push_back(std::unique_ptr<Value>(
                    new Movement(static_cast<float>(distance) / 100.0)));
            } else {
              valuesList.push_back(std::unique_ptr<Value>(
                    new NoTarget()));
            }
            return valuesList;
          } else {
            state = WAIT_HEADER_1;
          }
      }
    }
    return valuesList;
  }
};
