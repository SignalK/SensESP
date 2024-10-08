
#include "signalk_time.h"

namespace sensesp {

SKOutputTime::SKOutputTime(const String& sk_path, const String& config_path)
    : TimeString(config_path), SKEmitter(sk_path) {
  load();
}

void SKOutputTime::as_signalk_json(JsonDocument& doc){
  doc["path"] = this->sk_path_;
  doc["value"] = output;
}

bool SKOutputTime::to_json(JsonObject& doc) {
  doc["sk_path"] = sk_path_;
  return true;
}

bool SKOutputTime::from_json(const JsonObject& config) {
  const String expected[] = {"sk_path"};
  for (auto str : expected) {
    if (!config[str].is<JsonVariant>()) {
      return false;
    }
  }
  sk_path_ = config["sk_path"].as<String>();
  return true;
}

const String ConfigSchema(const SKOutputTime& obj) {
  return R"({"type":"object","properties":{"sk_path":{"title":"Signal K Path","type":"string"}}})";
}

}  // namespace sensesp
