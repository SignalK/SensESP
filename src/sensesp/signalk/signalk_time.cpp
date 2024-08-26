
#include "signalk_time.h"

namespace sensesp {

SKOutputTime::SKOutputTime(const String& sk_path, const String& config_path)
    : TimeString(config_path), SKEmitter(sk_path) {
  load_configuration();
}

String SKOutputTime::as_signalk() {
  JsonDocument json_doc;
  String json;
  json_doc["path"] = this->sk_path_;
  json_doc["value"] = output;
  serializeJson(json_doc, json);
  return json;
}

void SKOutputTime::get_configuration(JsonObject& doc) {
  doc["sk_path"] = sk_path_;
}

static const char kSchema[] = R"({
    "type": "object",
    "properties": {
        "sk_path": { "title": "Signal K Path", "type": "string" },
        "value": { "title": "Last value", "type" : "string", "readOnly": true }
    }
  })";

String SKOutputTime::get_config_schema() { return kSchema; }

bool SKOutputTime::set_configuration(const JsonObject& config) {
  const String expected[] = {"sk_path"};
  for (auto str : expected) {
    if (!config.containsKey(str)) {
      return false;
    }
  }
  sk_path_ = config["sk_path"].as<String>();
  return true;
}

}  // namespace sensesp
