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

    virtual const char* etype() const = 0;

    virtual bool isEqual(const Value& other) const {
        return this->power == other.power;
    }

    virtual Value* clone() const = 0;

    virtual void print() {
        Serial.printf("un-overridden '%s' dpower %g\n", etype(), power);
    }

    Value(float power=0.0) : power(power) {}
};

struct Movement : public Value {
    float distance = 0.0;

    const char* etype() const override { return "mov"; }

    bool isEqual(const Value& other) const override {
        const Movement* otherMov = static_cast<const Movement*>(&other);
        return this->distance == otherMov->distance && this->power == other.power;
    }

    Movement* clone() const override {
        return new Movement(*this);
    }

    void print() override {
        Serial.printf("mov distance %2.1f power %2.1f\n", distance, power);
    }

    Movement(float distance, float power=0.0) : Value(power), distance(distance) {}
};

struct Occupancy : public Value {
    float distance = 0.0;

    const char* etype() const override { return "occ"; }

    bool isEqual(const Value& other) const override {
        const Occupancy* otherOcc = static_cast<const Occupancy*>(&other);
        return this->distance == otherOcc->distance && this->power == other.power;
    }

    Occupancy* clone() const override {
        return new Occupancy(*this);
    }

    void print() override {
        Serial.printf("occ distance %2.1f power %2.1f\n", distance, power);
    }

    Occupancy(float distance, float power=0.0) : Value(power), distance(distance) {}
};

struct Speed : public Value {
    float speed = 0.0;

    const char* etype() const override { return "spd"; }

    bool isEqual(const Value& other) const override {
        const Speed* otherSpd = static_cast<const Speed*>(&other);
        return this->speed == otherSpd->speed;
    }

    Speed* clone() const override {
        return new Speed(*this);
    }

    void print() override {
        Serial.printf("spd %2.2f\n", speed);
    }

    Speed(float speed) : speed(speed) {}
};

struct Range : public Value {
    float x = 0.0;
    float y = 0.0;
    float speed;
    int reference;

    const char* etype() const override { return "rng"; }

    bool isEqual(const Value& other) const override {
        const Range* otherRange = static_cast<const Range*>(&other);
        return this->x == otherRange->x && this->y == otherRange->y &&
               this->speed == otherRange->speed && this->reference == otherRange->reference &&
               this->power == other.power;
    }

    Range* clone() const override {
        return new Range(*this);
    }

    void print() override {
        Serial.printf("speed %1.2f x pos %1.2f Y pos %1.2f %2d\n", speed, x, y, reference);
    }

    Range(float x, float y, float speed, int reference=0) : Value(0), x(x), y(y), speed(speed), reference(reference) {}
};

struct NoTarget : public Value {
    const char* etype() const override { return "none"; }

    NoTarget* clone() const override {
        return new NoTarget();
    }

    void print() override {
        Serial.printf("no target\n");
    }
};


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
        if (v->etype() == "none") {
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
        lastValuesList.push_back(std::unique_ptr<Value>(v->clone()));
    }
  }

  //ep->Detected(eventType, eventValue, eventPower, true, speed_type);
  //  ep->Cleared();
};


