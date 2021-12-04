#include "sensesp_base_app.h"

namespace sensesp {

void SetupSerialDebug(uint32_t baudrate) {
  Serial.begin(baudrate);

  // A small delay and one debugI() are required so that
  // the serial output displays everything
#ifndef DEBUG_DISABLED
  delay(100);
  Debug.setSerialEnabled(true);
  delay(100);
#endif
  debugI("\nSerial debugging enabled");
}

SensESPBaseApp* SensESPBaseApp::instance_ = nullptr;

SensESPBaseApp::SensESPBaseApp() {
  // create the hostname_ observable - this needs to be done before
  // the builder methods are called
  hostname_ = new ObservableValue<String>(kDefaultHostname);
}

/**
 * Get the singleton SensESPBaseApp singleton instance.
 * The instance must be set by the builder.
 */
SensESPBaseApp* SensESPBaseApp::get() { return instance_; }

void SensESPBaseApp::setup() {
  // initialize the filesystem
  filesystem_ = new Filesystem();

  // create a remote debugger object
  debug_output_ = new DebugOutput();
}

void SensESPBaseApp::start() {
  // start all individual startable components

  debugI("Starting subsystems");
  Startable::start_all();
  debugD("All sensors and transforms started");
}

void SensESPBaseApp::reset() {
  debugW("Resetting the device configuration to system defaults.");
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
