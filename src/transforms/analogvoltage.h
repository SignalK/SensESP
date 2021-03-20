#ifndef _analog_voltage_H
#define _analog_voltage_H

#include "transform.h"

#ifdef ESP32
#define MAX_ANALOG_OUTPUT 4096
#else
#define MAX_ANALOG_OUTPUT 1024
#endif

/**
 * @brief A transform that takes the output of an
 * analog-to-digital converter and outputs the voltage that went into
 * it. (This can be the built-in ADC on the ESP, or it can be an external
 * ADC like the ADS1015/1115.)
 * 
 * It can also be used like the Linear transport since it has a multiplier
 * and an offset.
 * 
 * If you don't need to use the mulitplier or offset, you probably don't need
 * this transform, because the AnalogInput sensor now has the ability to output
 * the original voltage that came into it, using its `output_scale` parameter.
 * 
 * @param max_voltage is the maximum voltage allowable on the Analog
 * Input pin of the microcontroller, which is 3.3V on most ESP's, but only 1.0V
 * on some.
 * 
 * @param multiplier The raw output of the transform is multiplied by multiplier.
 * Default value is 1.0, so it does nothing if you don't change it.
 * 
 * @param offset Added to (or subtracted from, if it's negative) the value after
 * multiplier has been applied. Default value is 0.0, so it does nothing if you
 * don't change it.
 * 
 * @param config_path The path in the Config UI to configure this transform.
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
  float max_voltage_;
  float multiplier_;
  float offset_;
};

#endif