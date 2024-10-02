#ifndef SENSP_TRANSFORMS_MOVING_AVERAGE_H_
#define SENSP_TRANSFORMS_MOVING_AVERAGE_H_

#include <vector>

#include "sensesp/ui/config_item.h"
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
                const String& config_path = "");
  virtual void set(const float& input) override;
  virtual bool to_json(JsonObject& root) override;
  virtual bool from_json(const JsonObject& config) override;

 private:
  std::vector<float> buf_{};
  int ptr_ = 0;
  int sample_size_;
  float multiplier_;
  bool initialized_;
};

const String ConfigSchema(const MovingAverage& obj);

inline bool ConfigRequiresRestart(const MovingAverage& obj) {
  return true;
}

}  // namespace sensesp
#endif
