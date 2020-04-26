#include "threshold.h"

template <class C, class P>
void ThresholdTransform<C,P>::set_input(C input, uint8_t inputChannel) {
  
  if(input >= this->minValue && input <= this->maxValue)
  {
      this->output = this->inRange;

      debugI("Threshold is in range.");
  }
  else
  {
      this->output = this->outRange;

      debugI("Threshold is out of range.");
  }

  debugI("Threshold updated.");
  
  this->notify();
}


JsonObject& NumericThreshold::get_configuration(JsonBuffer& buf)
{
    JsonObject& root = buf.createObject();
    root["min"] = this->minValue;
    root["max"] = this->maxValue;
    root["in-range"] = this->inRange;
    root["value"] = this->output;
    return root;
}

static const char NUMERIC_SCHEMA[] PROGMEM = R"({
    "type": "object",
    "properties": {
        "min": { "title": "Minimal value", "type": "number" },
        "max": { "title": "Maximal value", "type": "number" },
        "in-range": { "title": "In range value", "type": "boolean" },
        "value": { "title": "Last value", "type": "boolean" }
    }
  })";

  bool NumericThreshold::set_configuration(const JsonObject& config)
  {
      String expected[] = {"min", "max", "in-range", "value"};
      for (auto str : expected) {
        if (!config.containsKey(str)) {
          return false;
        }
      }
      this->minValue = config["min"];
      this->maxValue = config["max"];
      this->inRange = config["in-range"];
      this->output = config["value"];

      return true;
  }

  String NumericThreshold::get_config_schema()
  {
    return FPSTR(NUMERIC_SCHEMA);
  }


  JsonObject& IntegerThreshold::get_configuration(JsonBuffer& buf)
  {
      JsonObject& root = buf.createObject();
      root["min"] = this->minValue;
      root["max"] = this->maxValue;
      root["in-range"] = this->inRange;
      root["value"] = this->output;
      return root;
  }

  static const char INTEGER_SCHEMA[] PROGMEM = R"({
      "type": "object",
      "properties": {
          "min": { "title": "Minimal value", "type": "number" },
          "max": { "title": "Maximal value", "type": "number" },
          "in-range": { "title": "In range value", "type": "boolean" },
          "value": { "title": "Last value", "type": "boolean" }
      }
    })";

  bool IntegerThreshold::set_configuration(const JsonObject& config)
  {
      String expected[] = {"min", "max", "in-range"};
      for (auto str : expected) {
        if (!config.containsKey(str)) {
          return false;
        }
      }
      this->minValue = config["min"];
      this->maxValue = config["max"];
      this->inRange = config["in-range"];
      this->output = config["value"];

      return true;
  }

  String IntegerThreshold::get_config_schema()
  {
    return FPSTR(INTEGER_SCHEMA);
  }