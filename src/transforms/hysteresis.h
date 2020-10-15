#ifndef _hysteresis_H_
#define _hysteresis_H_

#include "lambda_transform.h"

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
            &(Hysteresis::function), lower_threshold, upper_threshold,
            low_output, high_output, this->param_info, config_path) {}

 private:
  // Web UI parameter name definition
  const ParamInfo param_info[4] = {{"lower_threshold", "Lower threshold"},
                                   {"upper_threshold", "Upper threshold"},
                                   {"low_output", "Low output"},
                                   {"high_output", "High output"}};

  // Function implementing the actual hysteresis logic
  static OUT function(IN input, IN lower_threshold, IN upper_threshold,
                      OUT low_output, OUT high_output) {
    static OUT last_value = low_output;
    if (input < lower_threshold) {
      last_value = low_output;
    } else if (upper_threshold <= input) {
      last_value = high_output;
    }
    // If neither of the above conditions were met, input is between the
    // lower and upper thresholds (in the hysteresis region)
    return last_value;
  };
};

#endif
