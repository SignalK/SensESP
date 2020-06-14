#ifndef _ultrasonic_input_H_
#define _ultrasonic_input_H_

#define TRIGGER_PIN 15
#define INPUT_PIN 14

#include "sensor.h"

class UltrasonicInput : public NumericSensor {
 public:
  UltrasonicInput(uint read_delay = 200, String config_path = "");
  void enable() override final;

 private:
  uint read_delay;
  virtual JsonObject& get_configuration(JsonBuffer& buf) override;
  virtual bool set_configuration(const JsonObject& config) override;
  virtual String get_config_schema() override;
  void update();
  int ultrasonicRead();
  volatile long startTime;
  volatile int pulseWidth;
};

#endif
