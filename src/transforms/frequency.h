#ifndef _frequency_H_
#define _frequency_H_

#include "transform.h"


// Frequency transform divides its input value by the time elapsed since
// the last reading
class Frequency : public IntegerConsumer, public NumericTransform {
 public:
  Frequency(String sk_path, float k=1, String config_path="");
  virtual void set_input(int input, uint8_t inputChannel = 0) override final;
  virtual String as_signalK() override final;
  virtual void enable() override final;
  virtual JsonObject& get_configuration(JsonBuffer& buf) override final;
  virtual bool set_configuration(const JsonObject& config) override final;
 private:
  float k;
  int ticks = 0;
  uint last_update = 0;
};

#endif
