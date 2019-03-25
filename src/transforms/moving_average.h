#ifndef _moving_average_H_
#define _moving_average_H_

#include <vector>

#include "transform.h"

// y = k * 1/n * \sum_k=1^n(x_k)
class MovingAverage : public Transform {
 public:
  MovingAverage(String sk_path, int n, float k=1., String id="", String schema="");
  float get() { return output; }
  void set_input(float input);
  String as_json() override final;
  virtual JsonObject& get_configuration(JsonBuffer& buf) override final;
  virtual bool set_configuration(const JsonObject& config) override final;
 private:
  std::vector<float> buf;
  int ptr = 0;
  const int n;
  float k;
  float output = 0;
};

#endif
