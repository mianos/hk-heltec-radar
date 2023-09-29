#pragma once
#include <vector>
#include <memory>
#include "radar.h"

class LDNoRadar : public RadarSensor {
public:
  LDNoRadar(EventProc* ep) : RadarSensor(ep) {}

  std::vector<std::unique_ptr<Value>>  get_decoded_radar_data() override {
    std::vector<std::unique_ptr<Value>> valuesList;
    return valuesList;
  }
};
