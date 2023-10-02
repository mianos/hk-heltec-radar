#include <Arduino.h>

#include "radar.h"

RadarSensor::RadarSensor(EventProc* ep) : ep(ep) {}

void RadarSensor::set_silence_period(int silence_period) {
    detectionTimeout = silence_period;
}


void RadarSensor::process(float minPower) {
    auto valuesList = get_decoded_radar_data();

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
        if (v->get_power() >= minPower) {
            noTargetFound = false;
            break;
        }
    }
    // If tracking is on, don't send a second update if a detection event is sent
    bool sent_detected_event = false;
    switch (currentState) {
        case STATE_NOT_DETECTED:
            if (!noTargetFound) {
                for (auto &v : valuesList) {
                    if (v->etype() != "no") {
                      ep->Detected(v.get());  // pass unique_ptr
                      sent_detected_event = true;
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

    if (!sent_detected_event) {
      for (auto& v : valuesList) {
        if (v->etype() != "no") {
          ep->TrackingUpdate(v.get());
        }
      }
   }
}
