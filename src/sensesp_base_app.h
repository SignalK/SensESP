#ifndef _sensesp_base_app_H_
#define _sensesp_base_app_H_

// Required for RemoteDebug
#define USE_LIB_WEBSOCKET true

#include "sensesp.h"
#include "sensesp/net/debug_output.h"
#include "sensesp/system/filesystem.h"
#include "sensesp/system/observablevalue.h"
#include "sensesp/system/system_property.h"

namespace sensesp {

constexpr auto kDefaultHostname = "SensESP";

void SetupSerialDebug(uint32_t baudrate);

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

  ObservableValue<String>* hostname_;

  Filesystem* filesystem_;
  DebugOutput* debug_output_;

  const SensESPBaseApp* set_hostname(String hostname) {
    hostname_->set(hostname);
    return this;
  }
};

}  // namespace sensesp

#endif
