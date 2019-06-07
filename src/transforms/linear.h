#ifndef _linear_H_
#define _linear_H_

#include "transform.h"
#include "system/valueconsumer.h"

// y = k * x + c
class Linear : public SymmetricTransform<float>  {
 public:
  Linear(String sk_path, float k, float c, String config_path="");
  virtual void set_input(float input, uint8_t inputChannel = 0) override final;
  virtual String as_signalK() override final;
  virtual JsonObject& get_configuration(JsonBuffer& buf) override final;
  virtual bool set_configuration(const JsonObject& config) override final;
  virtual String get_config_schema() override;

 private:
  float k;
  float c;
};

#endif
