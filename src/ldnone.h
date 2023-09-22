#pragma once
#include <memory>
#include "radar.h"

class LDNoRadar : public RadarSensor {
public:
  LDNoRadar(EventProc* ep) : RadarSensor(ep) {}

  std::unique_ptr<Value> get_decoded_radar_data() override {
    return nullptr;
  }
};
