#ifndef SENSESP_TRANSFORMS_MEDIAN_H_
#define SENSESP_TRANSFORMS_MEDIAN_H_

#include <vector>

#include "sensesp/ui/config_item.h"
#include "transform.h"

namespace sensesp {

/**
 * @brief Outputs the median value of sample_size inputs.
 *
 * Creates a sorted list of sample_size values and outputs the one
 * in the middle (i.e. element number 'sample_size / 2').
 *
 * @param sample_size Number of values you want to take the median of.
 *
 * @param config_path Path to configure this transform in the Config UI.
 */
class Median : public FloatTransform {
 public:
  Median(unsigned int sample_size = 10, const String& config_path = "");
  virtual void set(const float& input) override;
  virtual bool to_json(JsonObject& root) override;
  virtual bool from_json(const JsonObject& config) override;

 private:
  std::vector<float> buf_{};
  unsigned int sample_size_;
};

const String ConfigSchema(const Median& obj);

inline bool ConfigRequiresRestart(const Median& obj) {
  return true;
}

}  // namespace sensesp
#endif
