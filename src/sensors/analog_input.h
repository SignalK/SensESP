#ifndef _analog_input_H_
#define _analog_input_H_

#include "sensor.h"

class AnalogInput : public NumericSensor {

public:
  AnalogInput(uint8_t pin = A0, uint read_delay = 200, String config_path = "");
  void enable() override final;

private:
  uint8_t pin;
  uint read_delay;
  virtual JsonObject& get_configuration(JsonBuffer& buf) override;
  virtual bool set_configuration(const JsonObject& config) override;
  virtual String get_config_schema() override;
  void update();
};


#endif
