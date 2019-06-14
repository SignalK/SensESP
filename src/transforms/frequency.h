#ifndef _frequency_H_
#define _frequency_H_

#include "transform.h"


// Frequency transform divides its input value by the time elapsed since
// the last reading
class Frequency : public Transform<int, float> {
  
 public:
  Frequency(float k=1, String config_path="");
  virtual void set_input(int input, uint8_t inputChannel = 0) override;
  virtual void enable() override;
  virtual JsonObject& get_configuration(JsonBuffer& buf) override;
  virtual bool set_configuration(const JsonObject& config) override;
  virtual String get_config_schema() override;

 private:
  float k;
  int ticks = 0;
  uint last_update = 0;
};

#endif
