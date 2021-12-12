#ifndef _sensesp_app_H_
#define _sensesp_app_H_

#ifdef LED_BUILTIN
#define LED_PIN LED_BUILTIN
#define ENABLE_LED true
#else
#define LED_PIN 0
#define ENABLE_LED false
#endif

#include "controllers/system_status_controller.h"
#include "net/discovery.h"
#include "net/http_server.h"
#include "net/networking.h"
#include "net/ota.h"
#include "net/debug_output.h"
#include "net/ws_client.h"
#include "sensesp_base_app.h"
#include "sensors/sensor.h"
#include "signalk/signalk_delta_queue.h"
#include "system/system_status_led.h"

namespace sensesp {

/**
 * The default SensESP application object with networking and Signal K
 * communication.
 * @see SensESPAppBuilder
 */
class SensESPApp : public SensESPBaseApp {
 public:
  /**
   * Singletons should not be cloneable
   */
  SensESPApp(SensESPApp& other) = delete;

  /**
   * Singletons should not be assignable
   */
  void operator=(const SensESPApp&) = delete;

  /**
   * @brief Get the singleton instance of the SensESPApp
   */
  static SensESPApp* get();

  ObservableValue<String>* get_hostname_observable();

  // getters for internal members
  SKDeltaQueue* get_sk_delta() { return this->sk_delta_queue_; }
  SystemStatusController* get_system_status_controller() {
    return &(this->system_status_controller_);
  }
  Networking* get_networking() { return this->networking_; }
  WSClient* get_ws_client() { return this->ws_client_; }

 protected:
  /**
   * @brief SensESPApp constructor
   *
   * Note that the constructor is protected, so SensESPApp should only
   * be instantiated using SensESPAppBuilder.
   *
   */
  SensESPApp() : SensESPBaseApp() {}

  // setters for all constructor arguments

  const SensESPApp* set_hostname(String hostname) {
    this->SensESPBaseApp::set_hostname(hostname);
    return this;
  }
  const SensESPApp* set_ssid(String ssid) {
    this->ssid_ = ssid;
    return this;
  }
  const SensESPApp* set_wifi_password(String wifi_password) {
    this->wifi_password_ = wifi_password;
    return this;
  }
  const SensESPApp* set_sk_server_address(String sk_server_address) {
    this->sk_server_address_ = sk_server_address;
    return this;
  }
  const SensESPApp* set_sk_server_port(uint16_t sk_server_port) {
    this->sk_server_port_ = sk_server_port;
    return this;
  }
  const SensESPApp* set_system_status_led(SystemStatusLed* system_status_led) {
    this->system_status_led_ = system_status_led;
    return this;
  }
  const SensESPApp* enable_ota(const char* password) {
    ota_password_ = password;
  }
  const SensESPApp* set_wifi_manager_password(const char* password) {
    wifi_manager_password_ = password;
  }

  void setup();



  String ssid_ = "";
  String wifi_password_ = "";
  String sk_server_address_ = "";
  uint16_t sk_server_port_ = 0;
  const char* ota_password_ = nullptr;
  const char* wifi_manager_password_ = "thisisfine";

  Filesystem* filesystem_;
  DebugOutput* debug_output_;
  MDNSDiscovery* mdns_discovery_;
  HTTPServer* http_server_;
  SystemStatusLed* system_status_led_ = NULL;
  SystemStatusController system_status_controller_;
  Networking* networking_;
  OTA* ota_;
  SKDeltaQueue* sk_delta_queue_;
  WSClient* ws_client_;

  friend class HTTPServer;
  friend class SensESPAppBuilder;
};

extern SensESPApp* sensesp_app;

}  // namespace sensesp

#endif
