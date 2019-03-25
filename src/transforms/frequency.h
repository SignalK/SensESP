#ifndef _frequency_H_
#define _frequency_H_

#include "transform.h"

// Frequency transform divides its input value by the time elapsed since
// the last reading
class Frequency : public Transform {
 public:
  Frequency(String sk_path, float k=1, String id="", String schema="");
  float get() { return output; }
  void set_input(uint input);
  String as_json() override final;
  void enable() override final;
  virtual JsonObject& get_configuration(JsonBuffer& buf) override final;
  virtual bool set_configuration(const JsonObject& config) override final;
 private:
  float k;
  int ticks = 0;
  uint last_update = 0;
  float output;
};

#endif
