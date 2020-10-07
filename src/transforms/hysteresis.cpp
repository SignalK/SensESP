#include "hysteresis.h"

template <class C, class P>
void HysteresisTransform<C, P>::set_input(C input, uint8_t inputChannel) {
  debugD("minValue = %f", minValue);
  debugD("maxValue = %f", maxValue);
  debugD("input = %f", input);
  debugD("belowRange %s", x belowRange "true" : "false");
  debugD("aboveRange %s", x aboveRange "true" : "false");
  if (input < minValue) {
    this->output = belowRange;
  } 
  if (input > maxValue) {
    this->output = !belowRange;
  } 
  
  this->notify();
}

void NumericHysteresis::get_configuration(JsonObject& root) {
  root["min"] = minValue;
  root["max"] = maxValue;
  root["belowRange"] = belowRange;
  //root["aboveRange"] = aboveRange;
  root["value"] = output;
}

static const char NUMERIC_SCHEMA[] PROGMEM = R"({
    "type": "object",
    "properties": {
        "min": { "title": "Minimum value", "type": "number", "description" : "Minimum value of the hysteresis range" },
        "max": { "title": "Maximum value", "type": "number", "description" : "Maximum value of the hysteresis range" },
        "belowRange": { "title": "Value when input is below the range", "type": "boolean", "description" : "Output value when input value is below the hysteresis range" },
        "value": { "title": "Last value", "type" : "boolean", "readOnly": true }
    }
  })";

bool NumericHysteresis::set_configuration(const JsonObject& config) {
  String expected[] = {"min", "max", "in_range", "value"};
  for (auto str : expected) {
    if (!config.containsKey(str)) {
      return false;
    }
  }
  minValue = config["min"];
  maxValue = config["max"];
  belowRange = config["belowRange"];
  output = config["value"];

  return true;
}

String NumericHysteresis::get_config_schema() { return FPSTR(NUMERIC_SCHEMA); }

void IntegerHysteresis::get_configuration(JsonObject& root) {
  root["min"] = minValue;
  root["max"] = maxValue;
  root["belowRange"] = belowRange;
  root["value"] = output;
}

static const char INTEGER_SCHEMA[] PROGMEM = R"({
      "type": "object",
      "properties": {
          "min": { "title": "Minimum value", "type": "number", "description" : "Minimum value of the hysteresis range" },
          "max": { "title": "Maximum value", "type": "number", "description" : "Maximum value of the hysteresis range" },
          "belowRange": { "title": "Value when input is below the range", "type": "boolean", "description" : "Output value when input value is below the hysteresis range" },
          "value": { "title": "Last value", "type" : "boolean", "readOnly": true }
      }
    })";

bool IntegerHysteresis::set_configuration(const JsonObject& config) {
  String expected[] = {"min", "max", "belowRange"};
  for (auto str : expected) {
    if (!config.containsKey(str)) {
      return false;
    }
  }
  minValue = config["min"];
  maxValue = config["max"];
  belowRange = config["belowRange"];
  output = config["value"];

  return true;
}

String IntegerHysteresis::get_config_schema() { return FPSTR(INTEGER_SCHEMA); }