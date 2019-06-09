#ifndef _difference_H_
#define _difference_H_

#include "transform.h"
#include "system/valueconsumer.h"

// y = k1 * x1 - k2 * x2
class Difference : public SymmetricTransform<float> {
 public:
  Difference(String sk_path, float k1, float k2, String config_path="");
  virtual void set_input(float input, uint8_t inputChannel) override final;
  virtual String as_signalK() override final;
  virtual JsonObject& get_configuration(JsonBuffer& buf) override final;
  virtual bool set_configuration(const JsonObject& config) override final;
  virtual String get_config_schema() override;

 private:
  uint8_t received = 0;
  float inputs[2];
  float k1;
  float k2;
};

#endif
