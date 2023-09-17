
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

    // If there's valid data and it's above the minimum power threshold
    if (v && v->power >= minPower) {
      lastUpdateTime = millis();
      clearedPrinted = false;

      bool entry = false;
      if (!detectedPrinted) {
        entry = true;
        detectedPrinted = true;
      }
      String type(v->etype());
      ep->Detected(type, v->value, v->power, entry);
    }

    // Check if cleared should be printed based on a timeout condition
    if ((!clearedPrinted && ((millis() - lastUpdateTime) >= silence)) ||
        (v && v->etype() == "no")) {
      ep->Cleared();
      detectedPrinted = false;
      clearedPrinted = true;
    }
  }
};


