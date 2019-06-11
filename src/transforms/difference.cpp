#include "difference.h"

// Difference

Difference::Difference(String path, float k1, float k2, String config_path)
    : SymmetricTransform<float>{ path, config_path },
      k1{ k1 },
      k2{ k2 } {
  load_configuration();
}

void Difference::set_input(float input, uint8_t inputChannel) {
  inputs[inputChannel] = input;
  received |= 1<<inputChannel;
  if (received==0b11) {
    output = k1 * inputs[0] - k2 * inputs[1];
    received = 0;
    notify();
  }
}

String Difference::as_signalK() {
  DynamicJsonBuffer jsonBuffer;
  String json;
  JsonObject& root = jsonBuffer.createObject();
  root.set("path", this->sk_path);
  root.set("value", output);
  root.printTo(json);
  return json;
}

JsonObject& Difference::get_configuration(JsonBuffer& buf) {
  JsonObject& root = buf.createObject();
  root["k1"] = k1;
  root["k2"] = k2;
  root["sk_path"] = sk_path;
  root["value"] = output;
  return root;
}

static const char SCHEMA[] PROGMEM = R"({
    "type": "object",
    "properties": {
        "sk_path": { "title": "SignalK Path", "type": "string" },
        "k1": { "title": "Input #1 multiplier", "type": "number" },
        "k2": { "title": "Input #2 multiplier", "type": "number" },
        "value": { "title": "Last value", "type" : "number", "readOnly": true }
    }
  })";

String Difference::get_config_schema() {
  return FPSTR(SCHEMA);
}

bool Difference::set_configuration(const JsonObject& config) {
  String expected[] = {"k1", "k2", "sk_path"};
  for (auto str : expected) {
    if (!config.containsKey(str)) {
      return false;
    }
  }
  k1 = config["k1"];
  k2 = config["k2"];
  sk_path = config["sk_path"].as<String>();
  return true;
}
