#ifndef _ultrasonic_input_H_
#define _ultrasonic_input_H_
#include "sensor.h"

class UltrasonicSens : public NumericSensor {
 public:
  UltrasonicSens(int8_t trigger_pin, int8_t input_pin, uint read_delay = 1000,
                 String config_path = "");
  void enable() override final;
  uint read_delay;

 private:
  int8_t triggerPin;
  int8_t inputPin;
  virtual JsonObject& get_configuration(JsonBuffer& buf) override;
  virtual bool set_configuration(const JsonObject& config) override;
  virtual String get_config_schema() override;
};

#endif
