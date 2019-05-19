#include "difference.h"

// Difference

Difference::Difference(String path, float k1, float k2, String id, String schema, uint8_t valueIdx)
    : OneToOneTransform<float>{ path, id, schema, valueIdx },
      k1{ k1 },
      k2{ k2 } {
  inputs = new float[2];
  load_configuration();
}

void Difference::set_input(float input, uint8_t idx) {
  inputs[idx] = input;
  received |= 1<<idx;
  if (received==0b11) {
    output = k1 * inputs[0] - k2 * inputs[1];
    received = 0;
    notify();
  }
}

String Difference::as_json() {
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
