#include "linear.h"

// Keys and descriptions of constant parameters

const ParamInfo Linear::param_info[] = {{"k", "Multiplier"},
                                        {"c", "Constant offset"}};

// Function implementing the linear transform

float (*Linear::function)(float, float, float) =
    [](float input, float k, float c) { return k * input + c; };

// Constructor definition

Linear::Linear(float k, float c, String config_path)
    : LambdaTransform<float, float, float, float>(function, k, c, param_info,
                                                  config_path) {}
