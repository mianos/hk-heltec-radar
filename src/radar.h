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
  bool detectedPrinted = false;
  bool clearedPrinted = false;
  unsigned long lastUpdateTime = 0;
  float lastValue = 0.0;  // Add this line to keep track of the last value

public:
  RadarSensor(EventProc* ep) : ep(ep) {}

  virtual std::vector<std::unique_ptr<Value>> get_decoded_radar_data() = 0;

protected:
  int silence = 2000;

public:
  void set_silence_period(int silence_period) {
    silence = silence_period;
  }

  // TODO: rework this so that on non speed types it won't print the distance values too much
  void process(float minPower = 0.0) {
    auto valuesList = get_decoded_radar_data();

    for (auto &v : valuesList) {
      if (v) {
        v->print();
      }
    }
  }
#if 0
        String eventType = v->etype();
        float eventValue = v->value;
        float eventPower = v->power;

        // If there's valid data and it's above the minimum power threshold
        if (eventPower >= minPower && eventType != "no") {
          lastUpdateTime = millis();
          clearedPrinted = false;  // Resetting the flag here

          auto speed_type = eventType == "spd" ? true : false;
          if (!detectedPrinted || eventValue != lastValue) {
            ep->Detected(eventType, eventValue, eventPower, true, speed_type);
            detectedPrinted = true;
            lastValue = eventValue;  // Update the last value
          } else {
            ep->Detected(eventType, eventValue, eventPower, false, speed_type);
          }
        }
        else if (eventType == "no" && !clearedPrinted) {
          ep->Cleared();
          detectedPrinted = false;
          clearedPrinted = true;
        }
      }

      // Handle silence timeout
      if ((millis() - lastUpdateTime >= silence) && !clearedPrinted) {
        ep->Cleared();
        detectedPrinted = false;
        clearedPrinted = true;
      }
    }
#endif
};


