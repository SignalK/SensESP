
#include "signalk_position.h"

String SKOutputPosition::as_signalK() {
    DynamicJsonBuffer jsonBuffer;
    String json;
    JsonObject& root = jsonBuffer.createObject();
    root.set("path", this->get_sk_path());
    JsonObject& value = root.createNestedObject("value");
    value.set("latitude", output.latitude);
    value.set("longitude", output.longitude);
    if (output.altitude > -10000) {
      value.set("altitude", output.altitude);
    }
    root.printTo(json);
    return json;
}

JsonObject& SKOutputPosition::get_configuration(JsonBuffer& buf) {
  JsonObject& root = buf.createObject();
  root["sk_path"] = this->get_sk_path();
  return root;
}

static const char SCHEMA[] PROGMEM = R"({
    "type": "object",
    "properties": {
        "sk_path": { "title": "SignalK Path", "type": "string" }
    }
  })";

String SKOutputPosition::get_config_schema() {
  return FPSTR(SCHEMA);
}

bool SKOutputPosition::set_configuration(const JsonObject& config) {
  if (!config.containsKey("sk_path")) {
    return false;
  }
  this->set_sk_path(config["sk_path"].as<String>());
  return true;
}
