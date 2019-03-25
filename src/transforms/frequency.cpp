#include "frequency.h"

// Frequency

Frequency::Frequency(String sk_path, float k, String id, String schema)
    : Transform{sk_path, id, schema}, k{k} {
  //load_configuration();
}

void Frequency::enable() {
  last_update = millis();
}

void Frequency::set_input(uint input) {
  unsigned long cur_millis = millis();
  unsigned long elapsed_millis = cur_millis - last_update;
  output = k * input / (elapsed_millis / 1000.);
  last_update = cur_millis;
  notify();
}

String Frequency::as_json() {
  DynamicJsonBuffer jsonBuffer;
  String json;
  JsonObject& root = jsonBuffer.createObject();
  root.set("path", this->sk_path);
  root.set("value", output);
  root.printTo(json);
  return json;
}

JsonObject& Frequency::get_configuration(JsonBuffer& buf) {
  JsonObject& root = buf.createObject();
  root["k"] = k;
  root["sk_path"] = sk_path;
  root["value"] = output;
  return root;
}

bool Frequency::set_configuration(const JsonObject& config) {
  String expected[] = {"k", "c", "sk_path"};
  for (auto str : expected) {
    if (!config.containsKey(str)) {
      return false;
    }
  }
  k = config["k"];
  sk_path = config["sk_path"].as<String>();
  return true;
}
