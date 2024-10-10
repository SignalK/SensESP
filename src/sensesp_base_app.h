#ifndef SENSESP_BASE_APP_H_
#define SENSESP_BASE_APP_H_

#ifndef SENSESP_BUTTON_PIN
// Default button pin is 0 (GPIO0), normally connected to the BOOT button
#define SENSESP_BUTTON_PIN 0
#endif

#include "sensesp.h"

#include <memory>

#include "esp_log.h"
#include "sensesp/system/filesystem.h"
#include "sensesp/system/observablevalue.h"

namespace sensesp {

constexpr auto kDefaultHostname = "SensESP";

inline void SetupLogging(esp_log_level_t default_level = ESP_LOG_VERBOSE) {
  esp_log_level_set("*", default_level);
}

inline void SetupSerialDebug(uint32_t baudrate) {
  SetupLogging();

  if (baudrate != 115200) {
    ESP_LOGW(__FILENAME__, "SetupSerialDebug baudrate parameter is ignored.");
  }
}

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
 protected:
  reactesp::EventLoop event_loop_;

 public:
  /**
   * @brief Get the singleton instance of the SensESPBaseApp
   */
  static const std::shared_ptr<SensESPBaseApp>& get() { return instance_; }

  /**
   * @brief Destroy the SensESPBaseApp instance
   */
  virtual bool destroy() {
    bool outside_users = instance_.use_count() > 1;

    if (outside_users) {
      ESP_LOGW(__FILENAME__,
               "SensESPBaseApp instance has active references and won't be "
               "properly destroyed.");
    }
    instance_ = nullptr;
    return !outside_users;
  }

  /**
   * @brief Start the app (activate all the subcomponents)
   *
   */
  virtual void start() {
    ESP_LOGW(__FILENAME__, "start() call is deprecated and can be removed.");
  }

  /**
   * @brief Reset the device to factory defaults
   *
   */
  virtual void reset() {
    ESP_LOGW(__FILENAME__,
             "Resetting the device configuration to system defaults.");
    Resettable::reset_all();

    this->event_loop_.onDelay(1000, []() {
      ESP.restart();
      delay(1000);
    });
  }

  /**
   * @brief Get the hostname observable object
   *
   * @return ObservableValue<String>*
   */
  std::shared_ptr<ObservableValue<String>> get_hostname_observable() {
    return std::static_pointer_cast<ObservableValue<String>>(hostname_);
  }

  /**
   * @brief Get the current hostname.
   *
   * @return String
   */
  static String get_hostname() {
    return SensESPBaseApp::get()->get_hostname_observable().get()->get();
  }

  /**
   * @brief Get the event loop object from the singleton SensESPBaseApp
   * instance.
   *
   */
  static reactesp::EventLoop* get_event_loop() {
    return &(SensESPBaseApp::get()->event_loop_);
  }

 protected:
  /**
   * @brief Construct a new SensESP Base App object
   *
   * This constructor must only be used in SensESPBaseAppBuilder, and must
   * be called only once. For compatibility reasons, the class hasn't been
   * refactored into a singleton.
   */
  SensESPBaseApp() : filesystem_{std::make_shared<Filesystem>()} {
    // Instance is now set by the builder
  }

  ~SensESPBaseApp() { instance_ = nullptr; }

  void init_hostname() {
    hostname_ = std::make_shared<PersistingObservableValue<String>>(
        kDefaultHostname, "/system/hostname");
    ConfigItem(hostname_);  // Make hostname configurable
  }

  /**
   * @brief Perform initialization of SensESPBaseApp once builder configuration
   * is done.
   *
   * This should be only called from the builder!
   *
   */
  virtual void setup() {
    if (!hostname_) {
      init_hostname();
    }
  }

  static std::shared_ptr<SensESPBaseApp> instance_;

  void set_instance(const std::shared_ptr<SensESPBaseApp>& instance) {
    instance_ = instance;
  }

  std::shared_ptr<PersistingObservableValue<String>> hostname_;

  std::shared_ptr<Filesystem> filesystem_;

  const SensESPBaseApp* set_hostname(String hostname) {
    if (!hostname_) {
      init_hostname();
    }
    hostname_->set(hostname);
    return this;
  }
};

}  // namespace sensesp

#endif
