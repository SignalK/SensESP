#ifndef _integrator_H_
#define _integrator_H_

#include "transform.h"
#include <system/valueconsumer.h>

// y = k * sum(x_t)
class Integrator : public NumericConsumer, public NumericTransform {
 public:
  Integrator(String sk_path, float k=1, float value=0, String id="", String schema="", uint8_t valueIdx = 0);
  virtual void enable() override final;
  virtual void set_input(float input, uint8_t idx = 0) override final;
  String as_json() override final;
  virtual JsonObject& get_configuration(JsonBuffer& buf) override final;
  virtual bool set_configuration(const JsonObject& config) override final;
  
 private:
  float k;
};

#endif
