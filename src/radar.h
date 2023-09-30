#pragma once
#include <vector>
#include <memory>

#include "events.h"

class RadarSensor {
  EventProc* ep;
  std::vector<std::unique_ptr<Value>> lastValuesList;
public:
  RadarSensor(EventProc* ep) : ep(ep) {}

  virtual std::vector<std::unique_ptr<Value>> get_decoded_radar_data() = 0;
protected:
  enum DetectionState {
      STATE_NOT_DETECTED,
      STATE_DETECTED_ONCE,
      STATE_DETECTED,
      STATE_CLEARED_ONCE
  };
  DetectionState currentState = STATE_NOT_DETECTED;
  uint32_t lastDetectionTime = 0;
  uint32_t detectionTimeout = 2000;

public:
  void set_silence_period(int silence_period) {
    detectionTimeout = silence_period;
  }

  void detected() {
    Serial.println("Detected");
  }

  void Cleared() {
    Serial.println("Cleared");
  }


  bool areValuesListsSame(const std::vector<std::unique_ptr<Value>>& list1,
                          const std::vector<std::unique_ptr<Value>>& list2) {
      if (list1.size() != list2.size()) return false;

      for (size_t i = 0; i < list1.size(); i++) {
          if (!list1[i]->isEqual(*list2[i])) return false;
      }

      return true;
  }

  void process(float minPower = 0.0) {
    auto valuesList = get_decoded_radar_data();

    if (areValuesListsSame(valuesList, lastValuesList)) {
      return; // Exit early if the lists are the same
    }

    bool noTargetFound = true;
    for (auto &v : valuesList) {
        //v->print();
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

  //ep->Detected(eventType, eventValue, eventPower, true, speed_type);
  //  ep->Cleared();
};


