#pragma once
#include <memory>

class EventProc {
public:
  virtual void Detected(String& type, float value, float power, bool entry, bool speed_type=false) = 0;
  virtual void Cleared() = 0;
};

struct Value {
  float value;
  float power;
  virtual const char* etype() = 0;
  void print() {
    Serial.printf("value %g power %g\n", value, power);
  }
};

struct Speed : public Value {
  const char* etype() override { return "spd"; }
};

struct Movement : public Value {
  const char* etype() override { return "mov"; }
};

struct Occupancy : public Value {
  const char* etype() override { return "occ"; }
};

struct NoTarget : public Value {
  const char* etype() override { return "no"; }
};


class RadarSensor {
  EventProc* ep;
  bool detectedPrinted = false;
  bool clearedPrinted = false;
  unsigned long lastUpdateTime = 0;
  float lastValue = 0.0;  // Add this line to keep track of the last value

public:
  RadarSensor(EventProc* ep) : ep(ep) {}

  virtual std::unique_ptr<Value> get_decoded_radar_data() = 0;

protected:
  int silence = 2000;

public:
  void set_silence_period(int silence_period) {
    silence = silence_period;
  }

  // TODO: rework this so that on non speed types it won't print the distance values too much
  void process(float minPower = 0.0) {
    std::unique_ptr<Value> v = get_decoded_radar_data();

    if (v) {
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
};


