#include "config_item.h"

namespace sensesp {

std::map<String, std::shared_ptr<ConfigItemBase>> ConfigItemBase::config_items_;

template <>
const char* get_schema_type_string(const int /*dummy*/) {
  return "number";
}

template <>
const char* get_schema_type_string(const float /*dummy*/) {
  return "number";
}

template <>
const char* get_schema_type_string(const String& /*dummy*/) {
  return "string";
}

template <>
const char* get_schema_type_string(const bool /*dummy*/) {
  return "boolean";
}

}  // namespace sensesp
