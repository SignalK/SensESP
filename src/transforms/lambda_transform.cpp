#include "lambda_transform.h"

namespace sensesp {

template <class T>
const char* get_schema_type_string(const T dummy) {
  return "string";
}

template <>
const char* get_schema_type_string(const int dummy) {
  return "number";
}

template <>
const char* get_schema_type_string(const float dummy) {
  return "number";
}

template <>
const char* get_schema_type_string(const String dummy) {
  return "string";
}

template <>
const char* get_schema_type_string(const bool dummy) {
  return "boolean";
}

}  // namespace sensesp
