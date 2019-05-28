#ifndef _moving_average_H_
#define _moving_average_H_

#include <vector>

#include "transform.h"

// y = k * 1/n * \sum_k=1^n(x_k)
class MovingAverage : public SymmetricTransform<float> {

 public:
  MovingAverage(String sk_path, int n, float k=1., String config_path="");
  virtual void set_input(float input, uint8_t inputChannel = 0) override final;
  virtual String as_signalK() override final;
  virtual JsonObject& get_configuration(JsonBuffer& buf) override final;
  virtual bool set_configuration(const JsonObject& config) override final;

 private:
  std::vector<float> buf;
  int ptr = 0;
  const int n;
  float k;
};

#endif
