#ifndef _linear_H_
#define linear_H_

#include "lambda_transform.h"

namespace sensesp {

/**
 * @brief Performs a linear transform on the input value:
 * output = (input * multiplier) + offset.
 *
 * @param multiplier The input is multiplied this value.
 *
 * @param offset This is added to (input * multiplier)
 *
 * @param config_path The path to configure this transform in the Config UI
 **/
class Linear : public LambdaTransform<float, float, float, float> {
 public:
  Linear(float multiplier, float offset, const String& config_path = "");

 private:
  static float (*function_)(float, float, float);
  static const ParamInfo param_info_[];
};

}  // namespace sensesp

#endif
