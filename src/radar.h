#pragma once

class RadarSensor {
public:
  RadarSensor() : distanceValue(0.0), strengthValue(0.0) {}

  virtual String decodeRadarDataFSM() = 0;

protected:
  float distanceValue;
  float strengthValue;
  String type;
  int silence = 2000;
public:
  void set_silence_period(int silence_period) {
    silence = silence_period;
  }

  void processRadarData(float minStrength = 0.0) {
    static bool motionDetected = false;
    static bool occupancyDetected = false;
    static unsigned long lastUpdateTime = 0;
    static bool detectedPrinted = false;

    String type = decodeRadarDataFSM();

    if (type != "" && strengthValue >= minStrength) {
      lastUpdateTime = millis(); // Update the time of the last radar data received

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
      Detected(type, distanceValue, strengthValue, entry);
    }

    // Checking if there's no motion or occupancy detected for 2 seconds
    if ((millis() - lastUpdateTime) >= silence && detectedPrinted) {
      Cleared();
      detectedPrinted = false;
      motionDetected = false;
      occupancyDetected = false;
    }
  }
  virtual void Detected(String& type, float distanceValue, float strengthValue, bool entry) = 0;
  virtual void Cleared() = 0;
};
