#ifndef _CurveInterpolator_H_
#define _CurveInterpolator_H_

#include "transforms/transform.h"

/**
 * @brief Uses a collection of input/output samples that approximate
 * a non-linear curve and outputs a value on that curve. 
 * The output is the linear interpolation between the two
 * sample points that the input falls between. It is used primarily for
 * non-linear analog gauges such as temperature gauges and oil pressure gauges,
 * which get their input from analog sensors that are variable resistors.
 */

class CurveInterpolator : public NumericTransform {
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

  // For reading and writing the configuration of this transformation
  virtual void get_configuration(JsonObject& doc) override;
  virtual bool set_configuration(const JsonObject& config) override;
  virtual String get_config_schema() override;

  // For manually adding sample points
  void clear_samples();
  // FIXME: Uncomment the following once the PIO Xtensa toolchain is updated
  // [[deprecated("Use clear_samples() instead.")]]
  void clearSamples() {
    debugW("Use clear_samples() instead.");
    clear_samples();
  }
  void add_sample(const Sample& new_sample);
  // FIXME: Uncomment the following once the PIO Xtensa toolchain is updated
  // [[deprecated("Use add_sample(...) instead.")]]
  void addSample(
      const Sample& new_sample) {
    debugW("Use add_sample(...) instead");
    add_sample(new_sample);
  }

 private:
  std::set<Sample> samples;
};

#endif
