#include "threshold.h"

template <class C, class P>
void ThresholdTransform<C, P>::set_input(C input, uint8_t input_channel) {
  if (input >= min_value_ && input <= max_value_) {
    this->output = in_range_;
  } else {
    this->output = out_range_;
  }

  this->notify();
}

void NumericThreshold::get_configuration(JsonObject& root) {
  root["min"] = min_value_;
  root["max"] = max_value_;
  root["in_range"] = in_range_;
}

static const char NUMERIC_SCHEMA[] PROGMEM = R"({
    "type": "object",
    "properties": {
        "min": { "title": "Minimum value", "type": "number", "description" : "Minimum value to be 'in range'" },
        "max": { "title": "Maximum value", "type": "number", "description" : "Maximum value to be 'in range'" },
        "in_range": { "title": "In range value", "type": "boolean", "description" : "Output value when input value is 'in range'" }
    }
  })";

bool NumericThreshold::set_configuration(const JsonObject& config) {
  String expected[] = {"min", "max", "in_range", "value"};
  for (auto str : expected) {
    if (!config.containsKey(str)) {
      return false;
    }
  }
  min_value_ = config["min"];
  max_value_ = config["max"];
  in_range_ = config["in_range"];
  return true;
}

String NumericThreshold::get_config_schema() { return FPSTR(NUMERIC_SCHEMA); }

void IntegerThreshold::get_configuration(JsonObject& root) {
  root["min"] = min_value_;
  root["max"] = max_value_;
  root["in_range"] = in_range_;
}

static const char INTEGER_SCHEMA[] PROGMEM = R"({
      "type": "object",
      "properties": {
          "min": { "title": "Minimum value", "type": "number", "description" : "Minimum value to be 'in range'" },
          "max": { "title": "Maximum value", "type": "number", "description" : "Maximum value to be 'in range'" },
          "in_range": { "title": "In range value", "type": "boolean", "description" : "Output value when input value is 'in range'" }
      }
    })";

bool IntegerThreshold::set_configuration(const JsonObject& config) {
  String expected[] = {"min", "max", "in_range"};
  for (auto str : expected) {
    if (!config.containsKey(str)) {
      return false;
    }
  }
  min_value_ = config["min"];
  max_value_ = config["max"];
  in_range_ = config["in_range"];
  return true;
}

String IntegerThreshold::get_config_schema() { return FPSTR(INTEGER_SCHEMA); }