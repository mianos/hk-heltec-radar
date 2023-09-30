#pragma once

class EventProc {
public:
  virtual void Detected(String& type, float value, float power, bool entry, bool speed_type=false) = 0;
  virtual void Cleared() = 0;
};

struct Value {
  float power;
  virtual const char* etype() const = 0;
  virtual void print() const {
    Serial.printf("un-overridden '%s' power %g\n", etype(), power);
  }
  Value(float power=0.0) : power(power) {}

  virtual std::unique_ptr<Value> clone() const = 0;
  virtual bool isEqual(const Value& other) const = 0;
};

struct Movement : public Value {
  float distance = 0.0;

  const char* etype() const override { return "mov"; }
  
  Movement(float distance, float power=0.0) : Value(power), distance(distance) {}

  virtual void print() const override {
    Serial.printf("mov distance %2.1f power %2.1f\n", distance, power);
  }

  std::unique_ptr<Value> clone() const override {
    return std::unique_ptr<Value>(new Movement(*this));
  }

  bool isEqual(const Value& other) const override {
    const Movement& o = static_cast<const Movement&>(other);
    return distance == o.distance && power == o.power;
  }
};

struct Occupancy : public Value {
  float distance = 0.0;

  const char* etype() const override { return "occ"; }

  Occupancy(float distance, float power=0.0) : Value(power), distance(distance) {}

  virtual void print() const override {
    Serial.printf("occ distance %2.1f power %2.1f\n", distance, power);
  }

  std::unique_ptr<Value> clone() const override {
    return std::unique_ptr<Value>(new Occupancy(*this));
  }

  bool isEqual(const Value& other) const override {
    const Occupancy& o = static_cast<const Occupancy&>(other);
    return distance == o.distance && power == o.power;
  }
};

struct Speed : public Value {
  float speed = 0.0;

  const char* etype() const override { return "spd"; }
  
  Speed(float speed) : speed(speed) {}

  virtual void print() const override {
    Serial.printf("spd %2.2f\n", speed);
  }

  std::unique_ptr<Value> clone() const override {
    return std::unique_ptr<Value>(new Speed(*this));
  }

  bool isEqual(const Value& other) const override {
    const Speed& o = static_cast<const Speed&>(other);
    return speed == o.speed && power == o.power;
  }
};

struct Range : public Value {
  float x = 0.0;
  float y = 0.0;
  float speed;
  int reference;

  const char* etype() const override { return "rng"; }

  Range(float x, float y, float speed, int reference=0) : x(x), y(y), speed(speed), reference(reference) {}

  virtual void print() const override {
    Serial.printf("speed %1.2f x pos %1.2f Y pos %1.2f %2d\n", speed, x, y, reference);
  }

  std::unique_ptr<Value> clone() const override {
    return std::unique_ptr<Value>(new Range(*this));
  }

  bool isEqual(const Value& other) const override {
    const Range& o = static_cast<const Range&>(other);
    return x == o.x && y == o.y && speed == o.speed && reference == o.reference && power == o.power;
  }
};

struct NoTarget : public Value {
  const char* etype() const override { return "no"; }

  virtual void print() const override {
    Serial.printf("no target\n");
  }

  std::unique_ptr<Value> clone() const override {
    return std::unique_ptr<Value>(new NoTarget(*this));
  }

  bool isEqual(const Value& other) const override {
    const NoTarget& o = static_cast<const NoTarget&>(other);
    return power == o.power;
  }
};


