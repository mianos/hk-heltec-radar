
#pragma once
#include "radar.h"

class LD2410 : public RadarSensor {
public:
  LD2410(EventProc* ep, int8_t RxPin = 33, int8_t TxPin = 32) : RadarSensor(ep) {
    Serial2.begin(256000, SERIAL_8N1, RxPin, TxPin);
  }

  String decodeRadarDataFSM() {
    enum State {
      WAIT_HEADER_F4,
      WAIT_HEADER_F3,
      WAIT_HEADER_F2,
      WAIT_HEADER_F1,
      READ_DATA_LENGTH_LSB,
      READ_DATA_LENGTH_MSB,
      READ_TYPE,
      READ_TARGET_HEADER,
      READ_TARGET_TYPE,
      READ_MOVING_DISTANCE_LSB,
      READ_MOVING_DISTANCE_MSB,
      READ_MOVING_ENERGY,
      READ_STATIONARY_DISTANCE_LSB,
      READ_STATIONARY_DISTANCE_MSB,
      READ_STATIONARY_ENERGY,
      READ_DETECTION_DISTANCE_LSB,
      READ_DETECTION_DISTANCE_MSB,
      READ_CHECKSUM,
      WAIT_TRAILER_F8,
      WAIT_TRAILER_F7,
      WAIT_TRAILER_F6,
      WAIT_TRAILER_F5
    };

    State state = WAIT_HEADER_F4;
    uint16_t movingDistance = 0, stationaryDistance = 0, detectionDistance = 0;
    uint8_t targetType = 0, movingEnergy = 0, stationaryEnergy = 0, checksum = 0;
    uint16_t dataLength = 0;

    while (Serial2.available()) {
      uint8_t currentByte = Serial2.read();

      switch (state) {
        case WAIT_HEADER_F4:
          if (currentByte == 0xF4) state = WAIT_HEADER_F3;
          break;
        case WAIT_HEADER_F3:
          if (currentByte == 0xF3) state = WAIT_HEADER_F2;
          break;
        case WAIT_HEADER_F2:
          if (currentByte == 0xF2) state = WAIT_HEADER_F1;
          break;
        case WAIT_HEADER_F1:
          if (currentByte == 0xF1) state = READ_DATA_LENGTH_LSB;
          break;
        case READ_DATA_LENGTH_LSB:
          dataLength = currentByte;
          state = READ_DATA_LENGTH_MSB;
          break;
        case READ_DATA_LENGTH_MSB:
          dataLength |= static_cast<uint16_t>(currentByte) << 8;
          state = READ_TYPE;
          break;
        case READ_TYPE:
          if (currentByte == 0x01 || currentByte == 0x02) state = READ_TARGET_HEADER;
          break;
        case READ_TARGET_HEADER:
          if (currentByte == 0xAA) state = READ_TARGET_TYPE;
          break;
        case READ_TARGET_TYPE:
          targetType = currentByte;
          state = READ_MOVING_DISTANCE_LSB;
          break;
        case READ_MOVING_DISTANCE_LSB:
          movingDistance = currentByte;
          state = READ_MOVING_DISTANCE_MSB;
          break;
        case READ_MOVING_DISTANCE_MSB:
          movingDistance |= static_cast<uint16_t>(currentByte) << 8;
          state = READ_MOVING_ENERGY;
          break;
        case READ_MOVING_ENERGY:
          movingEnergy = currentByte;
          state = READ_STATIONARY_DISTANCE_LSB;
          break;
        case READ_STATIONARY_DISTANCE_LSB:
          stationaryDistance = currentByte;
          state = READ_STATIONARY_DISTANCE_MSB;
          break;
        case READ_STATIONARY_DISTANCE_MSB:
          stationaryDistance |= static_cast<uint16_t>(currentByte) << 8;
          state = READ_STATIONARY_ENERGY;
          break;
        case READ_STATIONARY_ENERGY:
          stationaryEnergy = currentByte;
          state = READ_DETECTION_DISTANCE_LSB;
          break;
        case READ_DETECTION_DISTANCE_LSB:
          detectionDistance = currentByte;
          state = READ_DETECTION_DISTANCE_MSB;
          break;
        case READ_DETECTION_DISTANCE_MSB:
          detectionDistance |= static_cast<uint16_t>(currentByte) << 8;
          state = READ_CHECKSUM;
          break;
        case READ_CHECKSUM:
          checksum = currentByte;
          state = WAIT_TRAILER_F8;
          break;
        case WAIT_TRAILER_F8:
          if (currentByte == 0xF8) state = WAIT_TRAILER_F7;
          break;
        case WAIT_TRAILER_F7:
          if (currentByte == 0xF7) state = WAIT_TRAILER_F6;
          break;
        case WAIT_TRAILER_F6:
          if (currentByte == 0xF6) state = WAIT_TRAILER_F5;
          break;
        case WAIT_TRAILER_F5:
          if (currentByte == 0xF5) {
#if 0
            if (targetType) {
              Serial.printf("Target Type: %d\n", targetType);
            }
            if (movingEnergy) {
              Serial.printf("Moving Distance: %d energy %d\n", movingDistance, movingEnergy);
            } 
            if (stationaryEnergy) {
              Serial.printf("Stationary Distance: %d Energy %d\n", stationaryDistance, stationaryEnergy);
            }
            if (detectionDistance) {
              Serial.printf("Detection Distance: %d\n", detectionDistance);
            }
#endif
            switch (targetType) {
            case 0:
              strengthValue = 0.0;
              distanceValue = 0.0;
              strengthValue =0.0;
              return "";
            case 1:
              distanceValue = static_cast<float>(movingDistance) / 100.0;
              strengthValue = static_cast<float>(movingEnergy) / 100.0;
              return "mov"; 
            case 2:
              distanceValue = static_cast<float>(stationaryDistance) / 100.0;
              strengthValue = static_cast<float>(stationaryEnergy) / 100.0;
              return "occ";
            case 3: // Mov and occ
              distanceValue = static_cast<float>(movingDistance) / 100.0;
              strengthValue = static_cast<float>(movingEnergy) / 100.0;
              return "mov";
            }
          }
          break;
      }
    }
    return "";
  }
};
