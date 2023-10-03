#pragma once

#include <vector>
#include <memory>
#include "events.h"
#include "settings.h"

class RadarSensor {
  SettingsManager* settings;
public:
    RadarSensor(EventProc* ep, SettingsManager* settings);
    virtual std::vector<std::unique_ptr<Value>> get_decoded_radar_data() = 0;
    void process(float minPower = 0.0);
    bool tracking = false;

protected:
    enum DetectionState {
        STATE_NOT_DETECTED,
        STATE_DETECTED_ONCE,
        STATE_DETECTED,
        STATE_CLEARED_ONCE
    };

    EventProc* ep;
    DetectionState currentState = STATE_NOT_DETECTED;
    uint32_t lastDetectionTime = 0;
};

