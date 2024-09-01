#ifndef SENSESP_TRANSFORMS_HYSTERESIS_H_
#define SENSESP_TRANSFORMS_HYSTERESIS_H_

#include "lambda_transform.h"

namespace sensesp {

// Web UI parameter name definition
const ParamInfo hysteresis_param_info[4] = {
    {"lower_threshold", "Lower threshold"},
    {"upper_threshold", "Upper threshold"},
    {"low_output", "Low output"},
    {"high_output", "High output"}};

/**
 * @brief Hysteresis function
 *
 * Hysteresis is a threshold switch with a dead zone: to prevent switching
 * back and forth at the threshold value, input has to rise above the
 * upper threshold before being switched high, and has to decrease below
 * the lower threshold before being switched back low.
 *
 * @tparam IN Input variable type
 * @tparam OUT Output variable type
 * @param[in] lower_threshold Lower threshold value
 * @param[in] upper_threshold Upper threshold value
 * @param[in] low_output Output value when hysteresis function output is low
 * @param[in] high_output Output value when hysteresis function output is high
 * @param[in] config_path Configuration path for the transform
 **/
template <class IN, class OUT>
// Hysteresis inherits from a specialized LambdaTransform type
class Hysteresis : public LambdaTransform<IN, OUT, IN, IN, OUT, OUT> {
 public:
  Hysteresis(IN lower_threshold, IN upper_threshold, OUT low_output,
             OUT high_output, String config_path = "")
      : LambdaTransform<IN, OUT, IN, IN, OUT, OUT>(
            // the lambda function needs to be defined in this awkward
            // location because it needs to be able to capture `this`
            [this](IN input, IN lower_threshold, IN upper_threshold,
                   OUT low_output, OUT high_output) {
              if (input < lower_threshold) {
                this->last_value_ = low_output;
              } else if (upper_threshold <= input) {
                this->last_value_ = high_output;
              }
              // If neither of the above conditions were met, input is between
              // the lower and upper thresholds (in the hysteresis region)
              return this->last_value_;
            },
            lower_threshold, upper_threshold, low_output, high_output,
            hysteresis_param_info, config_path) {}

 private:
  OUT last_value_;
};

}  // namespace sensesp
#endif
