#ifndef _difference_H_
#define _difference_H_

#include "transform.h"
#include "system/valueconsumer.h"

// y = k1 * x1 - k2 * x2
class Difference : public NumericConsumer, public NumericTransform {
 public:
  Difference(String sk_path, float k1, float k2, String id="", String schema="", uint8_t valueIdx = 0);
  virtual void set_input(float input, uint8_t idx) override final;
  virtual String as_json() override final;
  virtual JsonObject& get_configuration(JsonBuffer& buf) override final;
  virtual bool set_configuration(const JsonObject& config) override final;

 private:
  uint8_t received = 0;
  float k1;
  float k2;
};

#endif
