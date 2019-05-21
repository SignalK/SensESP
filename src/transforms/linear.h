#ifndef _linear_H_
#define _linear_H_

#include "transform.h"
#include "system/valueconsumer.h"

// y = k * x + c
class Linear : public OneToOneTransform<float>  {
 public:
  Linear(String sk_path, float k, float c, String id="", String schema="");
  virtual void set_input(float input, uint8_t idx = 0) override final;
  virtual String as_json() override final;
  virtual JsonObject& get_configuration(JsonBuffer& buf) override final;
  virtual bool set_configuration(const JsonObject& config) override final;
 private:
  float k;
  float c;
};

#endif
