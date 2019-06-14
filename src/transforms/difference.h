#ifndef _difference_H_
#define _difference_H_

#include "transform.h"

// y = k1 * x1 - k2 * x2
class Difference : public NumericTransform {
 public:
  Difference(float k1, float k2, String config_path="");
  virtual void set_input(float input, uint8_t inputChannel) override;
  virtual JsonObject& get_configuration(JsonBuffer& buf) override;
  virtual bool set_configuration(const JsonObject& config) override;
  virtual String get_config_schema() override;

 private:
  uint8_t received = 0;
  float inputs[2];
  float k1;
  float k2;
};

#endif
