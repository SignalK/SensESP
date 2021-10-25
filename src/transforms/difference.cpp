#include "difference.h"

// Difference

Difference::Difference(float k1, float k2, String config_path)
    : FloatTransform(config_path), k1{k1}, k2{k2} {
  load_configuration();
}

void Difference::set_input(float input, uint8_t inputChannel) {
  inputs[inputChannel] = input;
  received |= 1 << inputChannel;
  if (received == 0b11) {
    received = 0;
    this->emit(k1 * inputs[0] - k2 * inputs[1]);
  }
}

void Difference::get_configuration(JsonObject& root) {
  root["k1"] = k1;
  root["k2"] = k2;
  root["value"] = output;
}

static const char SCHEMA[] PROGMEM = R"({
    "type": "object",
    "properties": {
        "k1": { "title": "Input #1 multiplier", "type": "number" },
        "k2": { "title": "Input #2 multiplier", "type": "number" },
        "value": { "title": "Last value", "type" : "number", "readOnly": true }
    }
  })";

String Difference::get_config_schema() { return FPSTR(SCHEMA); }

bool Difference::set_configuration(const JsonObject& config) {
  String expected[] = {"k1", "k2"};
  for (auto str : expected) {
    if (!config.containsKey(str)) {
      return false;
    }
  }
  k1 = config["k1"];
  k2 = config["k2"];
  return true;
}
