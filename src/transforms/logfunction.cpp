#include "logfunction.h"

LogFunction::LogFunction(float multiplier, float offset,
                             String config_path)
    : NumericTransform(config_path),
      multiplier{multiplier},
      offset{offset}{
  className = "LogFunction";
  load_configuration();
}

void LogFunction::set_input(float input, uint8_t inputChannel) {
  output = ( multiplier * log(input) ) + offset;
  notify();
}

void LogFunction::get_configuration(JsonObject& root) {
  root["multiplier"] = multiplier;
  root["offset"] = offset;
  root["value"] = output;
}

static const char SCHEMA[] PROGMEM = R"({
    "type": "object",
    "properties": {
        "constant_1": { "title": "multiplier", "type": "number", "description": "The (m)ultiplier in the equation f(x) = m*log10(x) + o" },
        "offset": { "title": "offset", "type": "number", "description": "The (o)ffset in the equation f(x) = m*log10(x) + 0" },
        "value": { "title": "Last value", "type" : "number", "readOnly": true }
    }
  })";

String LogFunction::get_config_schema() { return FPSTR(SCHEMA); }

bool LogFunction::set_configuration(const JsonObject& config) {
  String expected[] = {"multiplier", "offset"};
  for (auto str : expected) {
    if (!config.containsKey(str)) {
      return false;
    }
  }
  multiplier = config["multiplier"];
  offset = config["offset"];
  return true;
}
