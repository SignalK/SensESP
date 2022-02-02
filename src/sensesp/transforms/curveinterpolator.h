#ifndef _CurveInterpolator_H_
#define _CurveInterpolator_H_

#include "transform.h"

namespace sensesp {

/**
 * @brief Implement a piecewise linear interpolation transform.
 *
 * Piecewise linear functions are defined by a collection of x,y pairs, in which
 * the x values are monotonically increasing. The y values are the corresponding
 * values of the function at the x values. The interpolator will linearly
 * interpolate between the y values to produce a value for any x value.
 *
 * As a practical example, consider the following samples:
 *
 * - 0, 0
 * - 1, 0
 * - 2, 1
 * - 3, 1
 *
 * At different input values, the following happens:
 *
 * - At x = 0, the interpolator will return 0.
 * - At x = 0.5, the interpolator will still return 0 because both adjacent samples have y = 0.
 * - At x = 1.4, the adjacent outputs are 0 and 1, so the interpolator will return 0.4.
 * - At x = 2.5, the interpolator will return 1.0 because the adjacent outputs are both 1.
 *
 * It can be used e.g. for
 * non-linear analog gauges such as temperature gauges and oil pressure gauges,
 * which get their input from analog sensors that are variable resistors.
 */

class CurveInterpolator : public FloatTransform {
 public:
  class Sample {
   public:
    float input;
    float output;

    Sample();
    Sample(float input, float output);
    Sample(JsonObject& obj);

    friend bool operator<(const Sample& lhs, const Sample& rhs) {
      return lhs.input < rhs.input;
    }
  };

 public:
  CurveInterpolator(std::set<Sample>* defaults = NULL, String config_path = "");

  // Set and retrieve the transformed value
  void set_input(float input, uint8_t input_channel = 0) override;

  // Web UI configuration methods
  CurveInterpolator* set_input_title(String input_title) {
    input_title_ = input_title;
    return this;
  }
  CurveInterpolator* set_output_title(String output_title) {
    output_title_ = output_title;
    return this;
  }

  // For reading and writing the configuration of this transformation
  virtual void get_configuration(JsonObject& doc) override;
  virtual bool set_configuration(const JsonObject& config) override;
  virtual String get_config_schema() override;

  // For manually adding sample points
  void clear_samples();

  void add_sample(const Sample& new_sample);

  const std::set<Sample>& get_samples() const { return samples_; }

 protected:
  std::set<Sample> samples_;
  String input_title_ = "Input";
  String output_title_ = "Output";
};

}  // namespace sensesp

#endif
