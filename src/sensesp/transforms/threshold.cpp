#include "threshold.h"

namespace sensesp {

template <class C, class P>
void ThresholdTransform<C, P>::set(const C& value) {
  if (value >= min_value_ && value <= max_value_) {
    this->output = in_range_;
  } else {
    this->output = !in_range_;
  }

  this->notify();
}

void FloatThreshold::get_configuration(JsonObject& root) {
  root["min"] = min_value_;
  root["max"] = max_value_;
  root["in_range"] = in_range_;
}

static const char kFloatSchema[] = R"({
    "type": "object",
    "properties": {
        "min": { "title": "Minimum value", "type": "number", "description" : "Minimum value to be 'in range'" },
        "max": { "title": "Maximum value", "type": "number", "description" : "Maximum value to be 'in range'" },
        "in_range": { "title": "In range value", "type": "boolean", "description" : "Output value when input value is 'in range'" }
    }
  })";

bool FloatThreshold::set_configuration(const JsonObject& config) {
  String const expected[] = {"min", "max", "in_range"};
  for (auto str : expected) {
    if (!config[str].is<JsonVariant>()) {
      return false;
    }
  }
  min_value_ = config["min"];
  max_value_ = config["max"];
  in_range_ = config["in_range"];
  return true;
}

String FloatThreshold::get_config_schema() { return (kFloatSchema); }

void IntThreshold::get_configuration(JsonObject& root) {
  root["min"] = min_value_;
  root["max"] = max_value_;
  root["in_range"] = in_range_;
}

static const char kIntSchema[] = R"({
      "type": "object",
      "properties": {
          "min": { "title": "Minimum value", "type": "number", "description" : "Minimum value to be 'in range'" },
          "max": { "title": "Maximum value", "type": "number", "description" : "Maximum value to be 'in range'" },
          "in_range": { "title": "In range value", "type": "boolean", "description" : "Output value when input value is 'in range'" }
      }
    })";

bool IntThreshold::set_configuration(const JsonObject& config) {
  String const expected[] = {"min", "max", "in_range"};
  for (auto str : expected) {
    if (!config[str].is<JsonVariant>()) {
      return false;
    }
  }
  min_value_ = config["min"];
  max_value_ = config["max"];
  in_range_ = config["in_range"];
  return true;
}

String IntThreshold::get_config_schema() { return (kIntSchema); }

}  // namespace sensesp
