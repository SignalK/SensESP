#include "constant_sensor.h"

#include <Arduino.h>

namespace sensesp {

template <>
const String ConstantSensor<String>::sensor_type_ = "string";
template <>
const String ConstantSensor<int>::sensor_type_ = "integer";
template <>
const String ConstantSensor<float>::sensor_type_ = "number";
template <>
const String ConstantSensor<bool>::sensor_type_ = "boolean";

}  // namespace sensesp
