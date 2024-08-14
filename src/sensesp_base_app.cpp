#include "sensesp_base_app.h"

namespace sensesp {

void SetupSerialDebug(uint32_t baudrate) {
  SetupLogging();

  if (baudrate != 115200) {
    ESP_LOGW(__FILENAME__, "SetupSerialDebug baudrate parameter is ignored.");
  }
}

void SetupLogging(esp_log_level_t default_level) {
  esp_log_level_set("*", default_level);
}

SensESPBaseApp* SensESPBaseApp::instance_ = nullptr;

SensESPBaseApp::SensESPBaseApp() {
  // initialize the filesystem
  filesystem_ = new Filesystem();
  // create the hostname_ observable - this needs to be done before
  // the builder methods are called
  hostname_ = new PersistingObservableValue<String>(kDefaultHostname,
                                                    "/system/hostname");
  hostname_->set_description("Device hostname");
  hostname_->set_sort_order(0);
}

/**
 * Get the singleton SensESPBaseApp singleton instance.
 * The instance must be set by the builder.
 */
SensESPBaseApp* SensESPBaseApp::get() { return instance_; }

/**
 * @brief Perform initialization of SensESPBaseApp once builder configuration is
 * done.
 *
 * This should be only called from the builder!
 *
 */
void SensESPBaseApp::setup() {}

void SensESPBaseApp::start() {
  // start all individual startable components

  ESP_LOGW(__FILENAME__, "start() call is deprecated and can be removed.");
}

void SensESPBaseApp::reset() {
  ESP_LOGW(__FILENAME__, "Resetting the device configuration to system defaults.");
  Resettable::reset_all();

  ReactESP::app->onDelay(1000, []() {
    ESP.restart();
    delay(1000);
  });
}

ObservableValue<String>* SensESPBaseApp::get_hostname_observable() {
  return hostname_;
}

/**
 * @brief Get the current hostname.
 *
 * @return String
 */
String SensESPBaseApp::get_hostname() {
  return SensESPBaseApp::get()->get_hostname_observable()->get();
}

}  // namespace sensesp
