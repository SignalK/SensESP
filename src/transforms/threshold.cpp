#include "threshold.h"

template <class C, class P>
void ThresholdTransform<C, P>::set_input(C input, uint8_t inputChannel) {
  
  if(input >= minValue && input <= maxValue)
  {
      this->output = inRange;
  }
  else
  {
      this->output = outRange;
  }
  
  this->notify();
}


JsonObject& NumericThreshold::get_configuration(JsonBuffer& buf)
{
    JsonObject& root = buf.createObject();
    root["min"] = minValue;
    root["max"] = maxValue;
    root["in_range"] = inRange;
    root["value"] = output;
    return root;
}

static const char NUMERIC_SCHEMA[] PROGMEM = R"({
    "type": "object",
    "properties": {
        "min": { "title": "Minimum value", "type": "number", "description" : "Minimum value to be 'in range'" },
        "max": { "title": "Maximum value", "type": "number", "description" : "Maximum value to be 'in range'" },
        "in_range": { "title": "In range value", "type": "boolean", "description" : "Output value when input value is 'in range'" },
        "value": { "title": "Last value", "type" : "boolean", "readOnly": true }
    }
  })";

  bool NumericThreshold::set_configuration(const JsonObject& config)
  {
      String expected[] = {"min", "max", "in_range", "value"};
      for (auto str : expected) {
        if (!config.containsKey(str)) {
          return false;
        }
      }
      minValue = config["min"];
      maxValue = config["max"];
      inRange = config["in_range"];
      output = config["value"];

      return true;
  }

  String NumericThreshold::get_config_schema()
  {
    return FPSTR(NUMERIC_SCHEMA);
  }


  JsonObject& IntegerThreshold::get_configuration(JsonBuffer& buf)
  {
      JsonObject& root = buf.createObject();
      root["min"] = minValue;
      root["max"] = maxValue;
      root["in_range"] = inRange;
      root["value"] = output;
      return root;
  }

  static const char INTEGER_SCHEMA[] PROGMEM = R"({
      "type": "object",
      "properties": {
          "min": { "title": "Minimum value", "type": "number", "description" : "Minimum value to be 'in range'" },
          "max": { "title": "Maximum value", "type": "number", "description" : "Maximum value to be 'in range'" },
          "in_range": { "title": "In range value", "type": "boolean", "description" : "Output value when input value is 'in range'" },
          "value": { "title": "Last value", "type" : "boolean", "readOnly": true }
      }
    })";

  bool IntegerThreshold::set_configuration(const JsonObject& config)
  {
      String expected[] = {"min", "max", "in_range"};
      for (auto str : expected) {
        if (!config.containsKey(str)) {
          return false;
        }
      }
      minValue = config["min"];
      maxValue = config["max"];
      inRange = config["in_range"];
      output = config["value"];

      return true;
  }

  String IntegerThreshold::get_config_schema()
  {
    return FPSTR(INTEGER_SCHEMA);
  }