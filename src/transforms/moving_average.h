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
 * 
 * @param n is the number of samples to average
 * 
 * @param k is the "scale" - defaults to 1.0, so has no effect unless you use other than 1.0
 * 
 */

// y = k * 1/n * \sum_k=1^n(x_k)
class MovingAverage : public NumericTransform {

 public:
  MovingAverage(int n, float k=1., String config_path="");

  /**
   * set_input is used to input a new value to this transform. After the
   * transformation, notify() is called to output the new value.
   */
  virtual void set_input(float input, uint8_t inputChannel = 0) override;

  /** 
   * For reading and writing the configuration of this transform
   */
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
