#ifndef _moving_average_H_
#define _moving_average_H_

#include <vector>

#include "transform.h"

/**
 * MovingAverage is used to smooth the output of a value (signal) that has frequent
 * variations. For example, the output of a temperature sensor may vary from 180 to 185
 * several times over a short period, but you just want to see the average of that.
 * MovingAverage outputs the average of the most recent n values. It also incorporates
 * a "scale" factor, in case you want to increase or decrease your final output by a
 * fixed percentage.
 */

// y = k * 1/n * \sum_k=1^n(x_k)
class MovingAverage : public NumericTransform {

 public:
  /**
   * @param n is the number of most recent values you want to average for your output
   * 
   * @param the moving average will be multiplied by k before it is output - make it 
   * something other than 1. if you need to scale your output up or down by a fixed
   * percentage
   * 
   * */
  MovingAverage(int n, float k=1., String config_path="");
  virtual void set_input(float input, uint8_t inputChannel = 0) override;
  virtual JsonObject& get_configuration(JsonBuffer& buf) override;
  virtual bool set_configuration(const JsonObject& config) override;
  virtual String get_config_schema() override;

 private:
  std::vector<float> buf;
  int ptr = 0;
  int n;
  float k;
  bool initialized;
};

#endif
