#include "frequency.h"

// Frequency

Frequency::Frequency(String sk_path, float k, String config_path) :
    IntegerConsumer(),
    NumericTransform{sk_path, config_path}, k{k} {
    load_configuration();
}

void Frequency::enable() {
  last_update = millis();
}

void Frequency::set_input(int input, uint8_t inputChannel) {
  unsigned long cur_millis = millis();
  unsigned long elapsed_millis = cur_millis - last_update;
  output = k * input / (elapsed_millis / 1000.);
  last_update = cur_millis;
  notify();
}

String Frequency::as_signalK() {
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

static const char SCHEMA[] PROGMEM = R"###({
    "type": "object",
    "properties": {
        "sk_path": { "title": "SignalK Path", "type": "string" },
        "k": { "title": "Multiplier", "type": "number" },
        "value": { "title": "Last value", "type" : "number", "readOnly": true }
    }
  })###";

String Frequency::get_config_schema() {
  return FPSTR(SCHEMA);
}

bool Frequency::set_configuration(const JsonObject& config) {
  String expected[] = {"k", "sk_path"};
  for (auto str : expected) {
    if (!config.containsKey(str)) {
      return false;
    }
  }
  k = config["k"];
  sk_path = config["sk_path"].as<String>();
  return true;
}
