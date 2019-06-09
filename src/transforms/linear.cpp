#include "linear.h"

// Linear

Linear::Linear(String path, float k, float c, String config_path) :
    SymmetricTransform<float>{ path, config_path },
      k{ k },
      c{ c } {
  load_configuration();
}


void Linear::set_input(float input, uint8_t inputChannel) {
  output = k * input + c;
  notify();
}

String Linear::as_signalK() {
  DynamicJsonBuffer jsonBuffer;
  String json;
  JsonObject& root = jsonBuffer.createObject();
  root.set("path", this->sk_path);
  root.set("value", output);
  root.printTo(json);
  return json;
}

JsonObject& Linear::get_configuration(JsonBuffer& buf) {
  JsonObject& root = buf.createObject();
  root["k"] = k;
  root["c"] = c;
  root["sk_path"] = sk_path;
  root["value"] = output;
  return root;
}


String Linear::get_config_schema() {
   return R"({
      "type": "object",
      "properties": {
          "sk_path": { "title": "SignalK Path", "type": "string" },
          "k": { "title": "Multiplier", "type": "number" },
          "c": { "title": "Constant offset", "type": "number" },
          "value": { "title": "Last value", "type" : "number", "readOnly": true }
      }
   })";
}


bool Linear::set_configuration(const JsonObject& config) {
  String expected[] = {"k", "c", "sk_path"};
  for (auto str : expected) {
    if (!config.containsKey(str)) {
      return false;
    }
  }
  k = config["k"];
  c = config["c"];
  sk_path = config["sk_path"].as<String>();
  return true;
}

