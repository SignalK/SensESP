#ifndef _sensesp_app_H_
#define _sensesp_app_H_

// Required for RemoteDebug
#define USE_LIB_WEBSOCKET true

#ifdef LED_BUILTIN
#define LED_PIN LED_BUILTIN
#define ENABLE_LED true
#else
#define LED_PIN 0
#define ENABLE_LED false
#endif

#include <forward_list>

#include "controllers/system_status_controller.h"
#include "net/http.h"
#include "net/networking.h"
#include "net/ota.h"
#include "net/ws_client.h"
#include "sensesp.h"
#include "sensors/sensor.h"
#include "signalk/signalk_delta_queue.h"
#include "system/observablevalue.h"
#include "system/system_status_led.h"
#include "system/valueconsumer.h"
#include "system/valueproducer.h"

void SetupSerialDebug(uint32_t baudrate);

/**
 * The main SensESP application object.
 * @see SensESPAppBuilder
 */
class SensESPApp {
 public:
  SensESPApp(bool defer_setup);
  SensESPApp(String hostname = "SensESP", String ssid = "",
             String wifi_password = "", String sk_server_address = "",
             uint16_t sk_server_port = 0);
  void setup();
  void enable();
  void reset();
  String get_hostname();

  // getters for internal members
  SKDeltaQueue* get_sk_delta() { return this->sk_delta_queue_; }
  SystemStatusController* get_system_status_controller() {
    return &(this->system_status_controller_);
  }
  Networking* get_networking() { return this->networking_; }
  WSClient* get_ws_client() { return this->ws_client_; }

 protected:
  // setters for all constructor arguments

  const SensESPApp* set_preset_hostname(String preset_hostname) {
    this->preset_hostname_ = preset_hostname;
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

 private:
  String preset_hostname_ = "SensESP";
  String ssid_ = "";
  String wifi_password_ = "";
  String sk_server_address_ = "";
  uint16_t sk_server_port_ = 0;

  void initialize();

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

#endif
