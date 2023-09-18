
#pragma once
#include <memory>
#include <Arduino.h>

class EventProc {
public:
  virtual void Detected(String& type, float value, float power, bool entry) = 0;
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
public:
  RadarSensor(EventProc* ep) : ep(ep) {}

  virtual std::unique_ptr<Value> get_decoded_radar_data() = 0;

protected:
  int silence = 2000;

public:
  void set_silence_period(int silence_period) {
    silence = silence_period;
  }
  
  void process(float minPower = 0.0) {
    std::unique_ptr<Value> v = get_decoded_radar_data();

    if (v == nullptr) {
      return;
    }

    String eventType = v->etype();
    float eventValue = v->value;
    float eventPower = v->power;

    // If there's valid data and it's above the minimum power threshold
    if (eventPower >= minPower && eventType != "no") {
      lastUpdateTime = millis();
      clearedPrinted = false;

      if (!detectedPrinted) {
        ep->Detected(eventType, eventValue, eventPower, true);
        detectedPrinted = true;
      } else {
        ep->Detected(eventType, eventValue, eventPower, false);
      }
    } 
    else if (eventType == "no" && !clearedPrinted) {
      ep->Cleared();
      detectedPrinted = false;
      clearedPrinted = true;
    }
    
    if ((millis() - lastUpdateTime >= silence) && !clearedPrinted) {
      ep->Cleared();
      detectedPrinted = false;
      clearedPrinted = true;
    }
  }

};


