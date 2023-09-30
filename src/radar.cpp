#include <Arduino.h>

#include "radar.h"

RadarSensor::RadarSensor(EventProc* ep) : ep(ep) {}

void RadarSensor::set_silence_period(int silence_period) {
    detectionTimeout = silence_period;
}

void RadarSensor::detected() {
    Serial.println("Detected");
}

void RadarSensor::Cleared() {
    Serial.println("Cleared");
}

bool RadarSensor::areValuesListsSame(const std::vector<std::unique_ptr<Value>>& list1,
                                     const std::vector<std::unique_ptr<Value>>& list2) {
    if (list1.size() != list2.size()) return false;

    for (size_t i = 0; i < list1.size(); i++) {
        if (!list1[i]->isEqual(*list2[i])) return false;
    }

    return true;
}

void RadarSensor::process(float minPower) {
    auto valuesList = get_decoded_radar_data();

    if (areValuesListsSame(valuesList, lastValuesList)) {
        return; // Exit early if the lists are the same
    }

    bool noTargetFound = true;
    for (auto &v : valuesList) {
        if (v->etype() == "no") {
            if (currentState == STATE_DETECTED || currentState == STATE_DETECTED_ONCE) {
                Cleared();
                currentState = STATE_CLEARED_ONCE;
                return;
            } else {
                currentState = STATE_NOT_DETECTED;
                return;
            }
        }
        if (v->power >= minPower) {
            noTargetFound = false;
            break;
        }
    }

    switch (currentState) {
        case STATE_NOT_DETECTED:
            if (!noTargetFound) {
                detected();
                currentState = STATE_DETECTED_ONCE;
            }
            break;

        case STATE_DETECTED_ONCE:
            currentState = STATE_DETECTED;
            lastDetectionTime = millis();
            break;

        case STATE_DETECTED:
            if (noTargetFound) {
                if (millis() - lastDetectionTime > detectionTimeout) {
                    Cleared();
                    currentState = STATE_CLEARED_ONCE;
                }
            } else {
                lastDetectionTime = millis();
            }
            break;

        case STATE_CLEARED_ONCE:
            currentState = STATE_NOT_DETECTED;
            break;
    }

   lastValuesList.clear();
    for (auto& v : valuesList) {
        lastValuesList.push_back(v->clone());
    }
}
