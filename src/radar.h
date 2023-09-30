#pragma once
#include <vector>
#include <memory>


class EventProc {
public:
  virtual void Detected(String& type, float value, float power, bool entry, bool speed_type=false) = 0;
  virtual void Cleared() = 0;
};

struct Value {
  float power;
  virtual const char* etype() = 0;
  virtual void print() {
    Serial.printf("un-overridden '%s' dpower %g\n", etype(), power);
  }
  Value(float power=0.0) : power(power) {}
};

struct Movement : public Value {
  float distance = 0.0;
  const char* etype() override { return "mov"; }
  Movement(float distance, float power=0.0) : Value(power), distance(distance) {}
  virtual void print() {
    Serial.printf("mov distance %2.1f power %2.1f\n", distance, power);
  }
};

struct Occupancy : public Value {
  float distance = 0.0;
  const char* etype() override { return "occ"; }
  float value() {
    return distance;
  }
  Occupancy(float distance, float power=0.0) : Value(power), distance(distance) {}
  virtual void print() {
    Serial.printf("occ distance %2.1f power %2.1f\n", distance, power);
  }
};


struct Speed : public Value {
  float speed = 0.0;
  const char* etype() override { return "spd"; }
  Speed(float speed) : speed(speed) {}
  virtual void print() {
    Serial.printf("spd %2.2f\n", speed);
  }
};


struct Range : public Value {
  float x = 0.0;
  float y = 0.0;
  float speed;
  int reference;
  const char* etype() override { return "rng"; }
  virtual void print() {
    Serial.printf("speed %1.2f x pos %1.2f Y pos %1.2f %2d\n", speed, x, y, reference);
  }
  Range(float x, float y, float speed, int reference=0) : x(x), y(y), speed(speed), reference(reference) {}
};


struct NoTarget : public Value {
  const char* etype() override { return "no"; }
  virtual void print() {
    Serial.printf("no target\n");
  }
};

class RadarSensor {
  EventProc* ep;
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

  void process(float minPower = 0.0) {
    auto valuesList = get_decoded_radar_data();

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

  }
  //ep->Detected(eventType, eventValue, eventPower, true, speed_type);
  //  ep->Cleared();
};


