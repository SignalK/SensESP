#include "signalk_listener.h"

namespace sensesp {

std::vector<SKListener *> SKListener::listeners_;

SemaphoreHandle_t SKListener::semaphore_ = xSemaphoreCreateRecursiveMutex();

SKListener::SKListener(const String &sk_path, int listen_delay,
                       const String &config_path)
    : Configurable(config_path), sk_path{sk_path}, listen_delay{listen_delay} {
  listeners_.push_back(this);
  this->load_configuration();
}

bool SKListener::take_semaphore(uint64_t timeout_ms) {
  if (timeout_ms == 0) {
    return xSemaphoreTakeRecursive(semaphore_, portMAX_DELAY) == pdTRUE;
  } else {
    return xSemaphoreTakeRecursive(semaphore_, timeout_ms) == pdTRUE;
  }
}

void SKListener::release_semaphore() { xSemaphoreGiveRecursive(semaphore_); }

String SKListener::get_config_schema() { return SIGNALKINPUT_SCHEMA; }

void SKListener::get_configuration(JsonObject &root) {
  root["sk_path"] = this->get_sk_path();
}

bool SKListener::set_configuration(const JsonObject &config) {
  if (!config["sk_path"].is<String>()) {
    return false;
  }
  this->set_sk_path(config["sk_path"].as<String>());
  return true;
}

void SKListener::set_sk_path(const String &path) { sk_path = path; }

}  // namespace sensesp
