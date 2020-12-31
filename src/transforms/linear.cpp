#include "linear.h"

// Keys and descriptions of constant parameters

const ParamInfo Linear::param_info_[] = {{"multiplier", "Multiplier"},
                                        {"offset", "Constant offset"}};

// Function implementing the linear transform

float (*Linear::function_)(float, float, float) =
    [](float input, float multiplier, float offset) { return multiplier * input + offset; };

// Constructor definition

Linear::Linear(float multiplier, float offset, String config_path)
    : LambdaTransform<float, float, float, float>(function_, multiplier, offset, param_info_,
                                                  config_path) {}
