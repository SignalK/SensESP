#ifndef _moving_average_H_
#define _moving_average_H_

#include <vector>

#include "transform.h"

namespace sensesp {

/**
 * @brief Outputs the moving average of the last sample_size inputs.
 *
 * Used to smooth the output of a value (signal) that has
 * frequent variations. For example, the output of a temperature sensor may vary
 * from 180 to 185 several times over a short period, but you just want to see
 * the average of that. MovingAverage outputs the average of the most recent
 * sample_size values. It also incorporates a "scale" factor, in case you want
 * to increase or decrease your final output by a fixed percentage.
 */

// y = k * 1/n * \sum_k=1^n(x_k)
class MovingAverage : public FloatTransform {
 public:
  /**
   * @param sample_size The number of most recent values you want to average for
   * your output.
   *
   * @param multiplier Moving average will be multiplied by multiplier before it
   * is output - make it something other than 1. if you need to scale your
   * output up or down by a fixed percentage.
   *
   * @param config_path The path used to configure this transform in the Config
   * UI.
   * */
  MovingAverage(int sample_size, float multiplier = 1.0,
                String config_path = "");
  virtual void set(const float& input) override;
  virtual void get_configuration(JsonObject& doc) override;
  virtual bool set_configuration(const JsonObject& config) override;
  virtual String get_config_schema() override;

 private:
  std::vector<float> buf_;
  int ptr_ = 0;
  int sample_size_;
  float multiplier_;
  bool initialized_;
};

}  // namespace sensesp
#endif
