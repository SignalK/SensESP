#include "analogvoltage.h"


AnalogVoltage::AnalogVoltage(float max_voltage, float multiplier, float offset, String config_path) :
    NumericTransform(config_path), max_voltage{max_voltage}, multiplier{multiplier}, offset{offset} {
  className = "AnalogVoltage";
  load_configuration();
}


void AnalogVoltage::set_input(float input, uint8_t inputChannel) {
  output = ((input * (max_voltage / 1024.0)) * multiplier) + offset;
  notify();
}


JsonObject& AnalogVoltage::get_configuration(JsonBuffer& buf) {
  JsonObject& root = buf.createObject();
  root["max_voltage"] = max_voltage;
  root["multiplier"] = multiplier;
  root["offset"] = offset;
  root["value"] = output;
  return root;
}

static const char SCHEMA[] PROGMEM = R"({
    "type": "object",
    "properties": {
        "max_voltage": { "title": "Max voltage", "type": "number", "description": "The maximum voltage allowed into your ESP's Analog Input pin" },
        "multiplier": { "title": "Mulitplier", "type": "number", "description": "Output will be multiplied by this before sending to SK" },
        "offsest": { "title": "Offset", "type": "number", "description": "This will be added to output before sending to SK" },
        "value": { "title": "Last value", "type" : "number", "readOnly": true }
    }
  })";

String AnalogVoltage::get_config_schema() {
  return FPSTR(SCHEMA);
}

bool AnalogVoltage::set_configuration(const JsonObject& config) {
  String expected[] = {"max_voltage", "multiplier", "offset" };
  for (auto str : expected) {
    if (!config.containsKey(str)) {
      return false;
    }
  }
  max_voltage = config["max_voltage"];
  multiplier = config["multiplier"];
  offset = config["offset"];
  return true;
}
