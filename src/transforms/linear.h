#ifndef _linear_H_
#define _linear_H_

#include "lambda_transform.h"

/**
 * Perform a linear transform on the input value. The transform
 * is of the form \f$y = k * x + c\f$, where k is the input
 * coefficient and c is a bias (offset) value.
 **/
class Linear : public LambdaTransform<float, float, float, float> {
 public:
  Linear(float k, float c, String config_path = "");

 private:
  static float (*function)(float, float, float);
  static const ParamInfo param_info[];
};

#endif
