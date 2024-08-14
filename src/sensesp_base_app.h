#ifndef _sensesp_base_app_H_
#define _sensesp_base_app_H_

#ifndef SENSESP_BUTTON_PIN
// Default button pin is 0 (GPIO0), normally connected to the BOOT button
#define SENSESP_BUTTON_PIN 0
#endif

#include "sensesp.h"

#include "esp_log.h"
#include "sensesp/system/filesystem.h"
#include "sensesp/system/observablevalue.h"

namespace sensesp {

constexpr auto kDefaultHostname = "SensESP";

void SetupSerialDebug(uint32_t baudrate);
void SetupLogging(esp_log_level_t default_level = ESP_LOG_VERBOSE);

/**
 * @brief The base class for SensESP applications.
 *
 * SensESPBaseApp is the base class for all apps. It can be either
 * expanded in child classes to provide additional functionality,
 * or used as is to provide a bare-bones application.
 *
 * @see SensESPApp
 */
class SensESPBaseApp {
 public:
  /**
   * @brief Get the singleton instance of the SensESPBaseApp
   */
  static SensESPBaseApp* get();

  /**
   * @brief Start the app (activate all the subcomponents)
   *
   */
  virtual void start();

  /**
   * @brief Reset the device to factory defaults
   *
   */
  virtual void reset();

  /**
   * @brief Get the hostname observable object
   *
   * @return ObservableValue<String>*
   */
  ObservableValue<String>* get_hostname_observable();

  static String get_hostname();

 protected:
  /**
   * @brief Construct a new SensESP Base App object
   *
   * This constructor must only be used in SensESPBaseAppBuilder, and must
   * be called only once. For compatibility reasons, the class hasn't been
   * refactored into a singleton.
   */
  SensESPBaseApp();

  virtual void setup();

  static SensESPBaseApp* instance_;

  PersistingObservableValue<String>* hostname_;

  Filesystem* filesystem_;

  const SensESPBaseApp* set_hostname(String hostname) {
    hostname_->set(hostname);
    return this;
  }
};

}  // namespace sensesp

#endif
