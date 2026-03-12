#include "signalk_put_request_listener.h"

namespace sensesp {

std::vector<SKPutListener*> SKPutListener::listeners_;

SKPutListener::SKPutListener(const String& sk_path) : sk_path{sk_path} {
  
  SKPutListener::listeners_.push_back(this);
  debugI("SKPutListener(%s)",sk_path.c_str());
   debugI("===========   SKPutListener::listeners_.size()=%d    ===========",SKPutListener::listeners_.size());
}

bool SKPutListener::to_json(JsonObject &root) {
  root["sk_path"] = this->get_sk_path();
  return true;
}

bool SKPutListener::from_json(const JsonObject &config) {
  if (!config["sk_path"].is<String>()) {
    return false;
  }
  this->set_sk_path(config["sk_path"].as<String>());
  return true;
}

void SKPutListener::set_sk_path(const String &path) { sk_path = path; }

const String ConfigSchema(const SKPutListener &obj) {
  return R"({"type":"object","properties":{"listen_delay":{"title":"Listen delay","type":"number","description":"The time, in milliseconds, between each read of the input"}}  })";
}
}  // namespace sensesp
