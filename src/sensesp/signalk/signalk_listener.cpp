#include "signalk_listener.h"

#include "sensesp/system/saveable.h"

namespace sensesp {

std::vector<SKListener *> SKListener::listeners_;

SemaphoreHandle_t SKListener::semaphore_ = xSemaphoreCreateRecursiveMutex();

SKListener::SKListener(const String &sk_path, int listen_delay,
                       const String &config_path)
    : FileSystemSaveable(config_path),
      sk_path{sk_path},
      listen_delay{listen_delay} {
  listeners_.push_back(this);
  this->load();
}

bool SKListener::take_semaphore(uint64_t timeout_ms) {
  if (timeout_ms == 0) {
    return xSemaphoreTakeRecursive(semaphore_, portMAX_DELAY) == pdTRUE;
  } else {
    return xSemaphoreTakeRecursive(semaphore_, timeout_ms) == pdTRUE;
  }
}

void SKListener::release_semaphore() { xSemaphoreGiveRecursive(semaphore_); }

bool SKListener::to_json(JsonObject &root) {
  root["sk_path"] = this->get_sk_path();
  return true;
}

bool SKListener::from_json(const JsonObject &config) {
  if (!config["sk_path"].is<String>()) {
    return false;
  }
  this->set_sk_path(config["sk_path"].as<String>());
  return true;
}

void SKListener::set_sk_path(const String &path) { sk_path = path; }

const String ConfigSchema(const SKListener &obj) {
  return R"({"type":"object","properties":{"listen_delay":{"title":"Listen delay","type":"number","description":"The time, in milliseconds, between each read of the input"}}  })";
}

}  // namespace sensesp
