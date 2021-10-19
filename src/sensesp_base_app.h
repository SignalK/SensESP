#ifndef _sensesp_base_app_H_
#define _sensesp_base_app_H_

// Required for RemoteDebug
#define USE_LIB_WEBSOCKET true

#include "net/remote_debugger.h"

#include "sensesp.h"
#include "system/filesystem.h"
#include "system/observablevalue.h"


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
   * @brief Construct a new SensESP Base App object
   *
   * This constructor must only be used in SensESPBaseAppBuilder
   * 
   * @param defer_setup If set to true, the setup() method will not be 
   *  called by the constructor.
   */
  SensESPBaseApp(bool defer_setup);

  /**
   * @brief Construct a new SensESP Base App object
   * 
   * This is a general-use constructor for direct instantiation of
   * SensESPBaseApp objects.
   * 
   */
  SensESPBaseApp(String preset_hostname = "SensESP");

  /**
   * @brief Initialize the app
   * 
   */
  virtual void setup();

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

protected:
  String preset_hostname_ = "SensESP";
  ObservableValue<String>* hostname_;

  Filesystem* filesystem_;
  RemoteDebugger* remote_debugger_;

  const SensESPBaseApp* set_preset_hostname(String preset_hostname) {
    preset_hostname_ = preset_hostname;
    return this;
  }
};

#endif