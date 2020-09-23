
#include "signalk_time.h"


SKOutputTime::SKOutputTime(String sk_path, String config_path) :
    TimeString(config_path),
    SKEmitter(sk_path)  {
  load_configuration();
}


String SKOutputTime::as_signalK() {
  DynamicJsonDocument jsonDoc(1024);
  String json;
  JsonObject root = jsonDoc.as<JsonObject>();
  root["path"] = this->sk_path;
  root["value"] = output;
  serializeJson(root, json);
  return json;
}

JsonObject SKOutputTime::get_configuration(JsonDocument doc) {
  JsonObject root = doc.as<JsonObject>();
  root["sk_path"] = sk_path;
  root["value"] = output;
  return root;
}

static const char SCHEMA[] PROGMEM = R"({
    "type": "object",
    "properties": {
        "sk_path": { "title": "SignalK Path", "type": "string" },
        "value": { "title": "Last value", "type" : "string", "readOnly": true }
    }
  })";

String SKOutputTime::get_config_schema() {
  return FPSTR(SCHEMA);
}

bool SKOutputTime::set_configuration(const JsonObject config) {
  String expected[] = {"sk_path"};
  for (auto str : expected) {
    if (!config.containsKey(str)) {
      return false;
    }
  }
  sk_path = config["sk_path"].as<String>();
  return true;
}
