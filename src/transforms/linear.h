#ifndef _linear_H_
#define _linear_H_

#include "transform.h"

// y = k * x + c
class Linear : public Transform {
 public:
  Linear(String sk_path, float k, float c, String id="", String schema="");
  float get() { return output; }
  void set_input(float input);
  String as_json() override final;
  virtual JsonObject& get_configuration(JsonBuffer& buf) override final;
  virtual bool set_configuration(const JsonObject& config) override final;
 private:
  float k;
  float c;
  float output;
};

#endif
