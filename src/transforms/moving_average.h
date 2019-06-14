#ifndef _moving_average_H_
#define _moving_average_H_

#include <vector>

#include "transform.h"

// y = k * 1/n * \sum_k=1^n(x_k)
class MovingAverage : public NumericTransform {

 public:
  MovingAverage(int n, float k=1., String config_path="");
  virtual void set_input(float input, uint8_t inputChannel = 0) override;
  virtual JsonObject& get_configuration(JsonBuffer& buf) override;
  virtual bool set_configuration(const JsonObject& config) override;
  virtual String get_config_schema() override;

 private:
  std::vector<float> buf;
  int ptr = 0;
  const int n;
  float k;
};

#endif
