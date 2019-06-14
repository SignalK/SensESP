#ifndef _linear_H_
#define _linear_H_

#include "transform.h"

// y = k * x + c
class Linear : public NumericTransform  {
 public:
  Linear(float k, float c, String config_path="");
  virtual void set_input(float input, uint8_t inputChannel = 0) override;
  virtual JsonObject& get_configuration(JsonBuffer& buf) override;
  virtual bool set_configuration(const JsonObject& config) override;
  virtual String get_config_schema() override;

 private:
  float k;
  float c;
};

#endif
