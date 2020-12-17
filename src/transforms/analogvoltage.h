#ifndef _analog_voltage_H
#define _analog_voltage_H

#include "transform.h"

#ifdef ESP32
#define MAX_ANALOG_OUTPUT 4096
#else
#define MAX_ANALOG_OUTPUT 1024
#endif

/**
 * @brief A transform that takes the output of the built-in
 * analog-to-digital converter on the ESP and outputs the voltage that went into
 * it. It can also be used like the Linear transport since it has a multiplier
 * and an offset. max_voltage is the maximum voltage allowable on the Analog
 * Input pin of the microcontroller, which is 3.3V on most ESP's, but only 1.0V
 * on some.
 */

class AnalogVoltage : public NumericTransform {
 public:
  AnalogVoltage(float max_voltage = 3.3, float multiplier = 1.0,
                float offset = 0.0, String config_path = "");
  virtual void set_input(float input, uint8_t inputChannel = 0) override;
  virtual void get_configuration(JsonObject& doc) override;
  virtual bool set_configuration(const JsonObject& config) override;
  virtual String get_config_schema() override;

 private:
  float max_voltage;
  float multiplier;
  float offset;
};

#endif