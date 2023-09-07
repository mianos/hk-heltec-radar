#pragma once

class EventProc {
public:
  virtual void Detected(String& type, float distanceValue, float strengthValue, bool entry) = 0;
  virtual void Cleared() = 0;
};

class RadarSensor {
  EventProc* ep;
public:
  RadarSensor(EventProc* ep) : ep(ep), distanceValue(0.0), strengthValue(0.0) {}

  virtual String get_decoded_radar_data() = 0;

protected:
  float distanceValue = 0.0;
  float strengthValue = 0.0;
  String type;
  int silence = 2000;
public:
  void set_silence_period(int silence_period) {
    silence = silence_period;
  }

  void process(float minStrength = 0.0) {
    static bool motionDetected = false;
    static bool occupancyDetected = false;
    static unsigned long lastUpdateTime = 0;
    static bool detectedPrinted = false;
    static bool clearedPrinted = false;

    String type = get_decoded_radar_data();

    if ((type == "mov" || type == "occ")  && strengthValue >= minStrength) {
      lastUpdateTime = millis(); // Update the time of the last radar data received
      clearedPrinted = false;

      if (type == "mov") {
        motionDetected = true;
      } else if (type == "occ") {
        occupancyDetected = true;
      }
      bool entry = false;
      if ((motionDetected || occupancyDetected) && !detectedPrinted) {
        entry = true;
        detectedPrinted = true;
      }
      ep->Detected(type, distanceValue, strengthValue, entry);
    }

    // Checking if there's no motion or occupancy detected for 2 seconds
    if ((!clearedPrinted && type == "no") || ((millis() - lastUpdateTime) >= silence && detectedPrinted)) {
      ep->Cleared();
      detectedPrinted = false;
      motionDetected = false;
      occupancyDetected = false;
      clearedPrinted = true;
    }
  }
};
