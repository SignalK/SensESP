#ifndef change_filter_H
#define change_filter_H

#include "transform.h"

namespace sensesp {

/**
 * @brief A float passthrough transform that will only
 * pass that value through if it is "sufficiently different" from
 * the last value passed through.
 *
 * More specifically, the absolute
 * value of the difference between the new value and the last passed
 * value has to be greater than or equal to the specified min_delta,
 * and less than or equal to the specified max_delta. If
 * the consecutive skip count ever reaches max_skips, the value
 * will be let through regardless.
 *
 * @param min_delta If the change from the last passed value is less
 * than min_delta, it will not be passed. Default = 0.0.
 *
 * @param max_delta If the change from the last passed value is more
 * than max_delta, it will not be passed. Default = 9999.0.
 *
 * @param max_skips If max_skips inputs have already been skipped, the
 * current input will be passed even if it's between min_delta and
 * max_delta, and max_skips will be reset to 0.
 *
 * @param config_path The path to configure this transform in the
 * Config UI.
 */
class ChangeFilter : public FloatTransform {
 public:
  ChangeFilter(float min_delta = 0.0, float max_delta = 9999.0,
               int max_skips = 99, String config_path = "");

  virtual void set_input(float new_value, uint8_t input_channel = 0) override;
  virtual void get_configuration(JsonObject& doc) override;
  virtual bool set_configuration(const JsonObject& config) override;
  virtual String get_config_schema() override;

 protected:
  float min_delta_;
  float max_delta_;
  int max_skips_;
  int skips_;
};

}  // namespace sensesp

#endif
