#ifndef _log_function_H
#define _log_function_H

#include "transform.h"
#include <math.h>

/**
 * LogFunction is a transform that returns (multiplier*log(input))+offset.
 * Although a generic function it is useful to define the response curve of NTC thermistors.
 */
 
class LogFunction : public NumericTransform  {
 public:
  LogFunction(float multiplier = 1.0, float offset = 0.0, String config_path = "");
  virtual void set_input(float input, uint8_t inputChannel = 0) override;
  virtual void get_configuration(JsonObject& doc) override;
  virtual bool set_configuration(const JsonObject& config) override;
  virtual String get_config_schema() override;

 private:
  float multiplier;
  float offset;
};

#endif