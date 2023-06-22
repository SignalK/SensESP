#include "signalk_listener.h"

namespace sensesp {

std::vector<SKListener *> SKListener::listeners;

SemaphoreHandle_t SKListener::semaphore_ = xSemaphoreCreateRecursiveMutex();

SKListener::SKListener(String sk_path, int listen_delay, String config_path)
    : Configurable(config_path), sk_path{sk_path}, listen_delay{listen_delay} {
  listeners.push_back(this);
  this->load_configuration();
}

bool SKListener::take_semaphore(unsigned long int timeout_ms) {
  if (timeout_ms == 0) {
    return xSemaphoreTakeRecursive(semaphore_, portMAX_DELAY) == pdTRUE;
  } else {
    return xSemaphoreTakeRecursive(semaphore_, timeout_ms) == pdTRUE;
  }
}

void SKListener::release_semaphore() { xSemaphoreGiveRecursive(semaphore_); }

String SKListener::get_config_schema() { return FPSTR(SIGNALKINPUT_SCHEMA); }

void SKListener::get_configuration(JsonObject &root) {
  root["sk_path"] = this->get_sk_path();
}

bool SKListener::set_configuration(const JsonObject &config) {
  if (!config.containsKey("sk_path")) {
    return false;
  }
  this->set_sk_path(config["sk_path"].as<String>());
  return true;
}

void SKListener::set_sk_path(const String &path) { sk_path = path; }

}  // namespace sensesp
