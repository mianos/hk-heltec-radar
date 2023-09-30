#pragma once

#include <vector>
#include <memory>
#include "events.h"

class RadarSensor {
public:
    RadarSensor(EventProc* ep);
    virtual std::vector<std::unique_ptr<Value>> get_decoded_radar_data() = 0;
    void set_silence_period(int silence_period);
    void detected();
    void Cleared();
    bool areValuesListsSame(const std::vector<std::unique_ptr<Value>>& list1, 
                            const std::vector<std::unique_ptr<Value>>& list2);
    void process(float minPower = 0.0);

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
    uint32_t detectionTimeout = 2000;
    std::vector<std::unique_ptr<Value>> lastValuesList;
};

