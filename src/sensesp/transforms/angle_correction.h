#ifndef _angle_correction_H_
#define _angle_correction_H_

#include "transform.h"

namespace sensesp {

/**
 * @brief Add a value to an angle input (in radians). The output
 * value is wrapped to a range between [0, 2*pi]. This transform
 * is useful for correcting e.g. a heading or a wind direction reading.
 *
 * @param offset Value to be added, in radians.
 *
 * @param min_angle If you have output between -pi and pi, use -3.14159265,
 * otherwise use 0. Default value is 0.
 *
 * @param config_path Path to configure this transform in the Config UI.
 */
class AngleCorrection : public FloatTransform {
 public:
  AngleCorrection(float offset, float min_angle = 0, String config_path = "");
  virtual void set(const float& input) override;
  virtual void get_configuration(JsonObject& doc) override;
  virtual bool set_configuration(const JsonObject& config) override;
  virtual String get_config_schema() override;

 private:
  float offset_;
  float min_angle_;
};

}  // namespace sensesp

#endif
