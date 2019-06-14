#ifndef _integrator_H_
#define _integrator_H_

#include "transform.h"

// y = k * sum(x_t)
class Integrator : public NumericTransform {
 public:
  Integrator(float k=1, float value=0, String config_path="");
  virtual void enable() override final;
  virtual void set_input(float input, uint8_t inputChannel = 0) override final;
  virtual JsonObject& get_configuration(JsonBuffer& buf) override final;
  virtual bool set_configuration(const JsonObject& config) override final;
  virtual String get_config_schema() override;

 private:
  float k;
};

#endif
