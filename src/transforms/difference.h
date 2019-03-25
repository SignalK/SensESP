#ifndef _difference_H_
#define _difference_H_

#include "transform.h"

// y = k1 * x1 - k2 * x2
class Difference : public Transform {
 public:
  Difference(String sk_path, float k1, float k2, String id="", String schema="");
  float get() { return output; }
  void set_input(uint8_t idx, float input);
  String as_json() override final;
  virtual JsonObject& get_configuration(JsonBuffer& buf) override final;
  virtual bool set_configuration(const JsonObject& config) override final;
 private:
  uint8_t received = 0;
  float inputs[2];
  float k1;
  float k2;
  float output;
};

#endif
