#include "voltage_multiplier.h"

VoltageMultiplier::VoltageMultiplier(uint16_t R1, uint16_t R2, String config_path) :
       NumericTransform(config_path), R1{R1}, R2{R2} {
     className = "VoltageMultiplier";
}

void VoltageMultiplier::set_input(float input, uint8_t inputChannel) {
    // Ohms Law at work!
    output = input * (((float)R1 + (float)R2) / (float)R2);
    notify();
}

JsonObject& VoltageMultiplier::get_configuration(JsonBuffer& buf) {
  JsonObject& root = buf.createObject();
  root["R1"] = R1;
  root["R2"] = R2;
  root["value"] = output;
  return root;
  };

  static const char SCHEMA[] PROGMEM = R"###({
    "type": "object",
    "properties": {
        "R1": { "title": "R1", "type": "number", "description": "The measured value of resistor R1" },
        "R2": { "title": "R2", "type": "number", "description": "The measured value of resistor R2" },
        "value": { "title": "Last value", "type" : "number", "readOnly": true }
    }
  })###";


  String VoltageMultiplier::get_config_schema() {
  return FPSTR(SCHEMA);
}

bool VoltageMultiplier::set_configuration(const JsonObject& config) {
  String expected[] = {"R1", "R2"};
  for (auto str : expected) {
    if (!config.containsKey(str)) {
      return false;
    }
  }
  R1 = config["R1"];
  R2 = config["R2"];
  return true;
}