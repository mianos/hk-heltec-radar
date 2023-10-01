#include <Arduino.h>

#include "radar.h"

RadarSensor::RadarSensor(EventProc* ep) : ep(ep) {}

void RadarSensor::set_silence_period(int silence_period) {
    detectionTimeout = silence_period;
}


bool RadarSensor::areValuesListsSame(const std::vector<std::unique_ptr<Value>>& list1,
                                     const std::vector<std::unique_ptr<Value>>& list2) {
    if (list1.empty() && list2.empty()) return true;

    if (list1.empty()) {
        for (const auto& v : list2) {
            if (v->etype() != "no") return false;
        }
        return true;
    }

    if (list2.empty()) {
        for (const auto& v : list1) {
            if (v->etype() != "no") return false;
        }
        return true;
    }

    if (list1.size() != list2.size()) return false;

    for (size_t i = 0; i < list1.size(); i++) {
        if (!list1[i]->isEqual(*list2[i])) return false;
    }

    return true;
}

void RadarSensor::process(float minPower) {
    auto valuesList = get_decoded_radar_data();

    if (areValuesListsSame(valuesList, lastValuesList)) {
      return;
    }

    bool noTargetFound = true;
    for (auto &v : valuesList) {
        if (v->etype() == "no") {
            if (currentState == STATE_DETECTED || currentState == STATE_DETECTED_ONCE) {
                ep->Cleared();
                currentState = STATE_CLEARED_ONCE;
                return;
            } else {
                currentState = STATE_NOT_DETECTED;
                return;
            }
        }
#if 0
        else {
          v->print();
        }
#endif
        if (v->get_power() >= minPower) {
            noTargetFound = false;
            break;
        }
    }

    switch (currentState) {
        case STATE_NOT_DETECTED:
            if (!noTargetFound) {
                for (auto &v : valuesList) {
                    if (v->etype() != "no") {
                      ep->Detected(v.get());  // pass unique_ptr? 
                    }
                }
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
                    ep->Cleared();
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

   if (!valuesList.empty()) {
     lastValuesList.clear();
      for (auto& v : valuesList) {
          lastValuesList.push_back(v->clone());
      }
   }
}
