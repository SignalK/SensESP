#include "transform.h"
#ifndef _threshold_h
#define _threshold_h

namespace sensesp {

/**
 * @brief A Transform base class that translates the value of type C into value
 * of type P. Base class for classes FloatThreshold and IntThreshold.
 *
 *   @param min_value Minimum value of input for output to be the value of
 * in_range.
 *
 *   @param max_value Maximum value of input for output to be the value of
 * in_range.
 *
 *   @param in_range Output value if input value is in range.
 */
template <typename C, typename P>
class ThresholdTransform : public Transform<C, P> {
 public:
  ThresholdTransform(C min_value, C max_value, P in_range,
                     String config_path = "")
      : Transform<C, P>(config_path),
        min_value_{min_value},
        max_value_{max_value},
        in_range_{in_range} {
    this->load_configuration();
  };
  virtual void set(const C& new_value) override;

 protected:
  C min_value_;
  C max_value_;
  P in_range_;
};

/**
 * @brief Translates a float value into a boolean value, which depends on
 * whether the float value is "in range" or "out of range".
 *
 * @param min_value The minimum of the range for the input value to be "in
 * range".
 *
 * @param max_value The maximum of the range for the input value to be "in
 * range".
 *
 * @param in_range The output value if the input value is "in range". Default is
 * true. (If the input value is not "in range", the value of output is the
 * opposite of in_range.)
 *
 * @param config_path Path to configure this transform in the Config UI.
 */
class FloatThreshold : public ThresholdTransform<float, bool> {
 public:
  FloatThreshold(float min_value, float max_value, bool in_range = true,
                 String config_path = "")
      : ThresholdTransform<float, bool>(min_value, max_value, in_range,
                                        config_path) {}

  virtual void get_configuration(JsonObject& doc) override;
  virtual bool set_configuration(const JsonObject& config) override;
  virtual String get_config_schema() override;
};

/**
 * @brief Translates an integer value into a boolean value, which depends on
 * whether the integer value is "in range" or "out of range".
 *
 * @param min_value The minimum of the range for the input value to be "in
 * range".
 *
 * @param max_value The maximum of the range for the input value to be "in
 * range".
 *
 * @param in_range The output value if the input value is "in range". Default is
 * true. (If the input value is not "in range", the value of output is the
 * opposite of in_range.)
 *
 * @param config_path Path to configure this transform in the Config UI.
 */
class IntThreshold : public ThresholdTransform<int, bool> {
 public:
  IntThreshold(int min_value, int max_value, bool in_range = true,
               String config_path = "")
      : ThresholdTransform<int, bool>(min_value, max_value, in_range,
                                      config_path) {}

  virtual void get_configuration(JsonObject& doc) override;
  virtual bool set_configuration(const JsonObject& config) override;
  virtual String get_config_schema() override;
};

}  // namespace sensesp
#endif
