#ifndef _sensesp_app_H_
#define _sensesp_app_H_

#ifdef LED_BUILTIN
#define LED_PIN LED_BUILTIN
#define ENABLE_LED true
#else
#define LED_PIN 0
#define ENABLE_LED false
#endif

#include "sensesp/controllers/system_status_controller.h"
#include "sensesp/net/debug_output.h"
#include "sensesp/net/discovery.h"
#include "sensesp/net/http_server.h"
#include "sensesp/net/web/static_file_handler.h"
#include "sensesp/net/web/config_handler.h"
#include "sensesp/net/web/base_command_handler.h"
#include "sensesp/net/web/app_command_handler.h"
#include "sensesp/net/networking.h"
#include "sensesp/net/ota.h"
#include "sensesp/net/ws_client.h"
#include "sensesp/sensesp_version.h"
#include "sensesp/sensors/sensor.h"
#include "sensesp/signalk/signalk_delta_queue.h"
#include "sensesp/system/system_status_led.h"
#include "sensesp/system/button.h"
#include "sensesp/ui/ui_output.h"
#include "sensesp_base_app.h"

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
  SensESPApp() : SensESPBaseApp() {
    hostname_->connect_to(hostname_ui_output_);
  }

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
    this->wifi_client_password_ = wifi_password;
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
    return this;
  }
  const SensESPApp* set_button_pin(int pin) {
    button_gpio_pin_ = pin;
    return this;
  }
  const SensESPApp* set_wifi_manager_password(const char* password) {
    wifi_manager_password_ = password;
    return this;
  }

  void setup();

  String ssid_ = "";
  String wifi_client_password_ = "";
  String sk_server_address_ = "";
  uint16_t sk_server_port_ = 0;
  const char* ota_password_ = nullptr;
  const char* wifi_manager_password_ = "thisisfine";

  MDNSDiscovery* mdns_discovery_;
  HTTPServer* http_server_;
  HTTPStaticFileHandler* http_static_file_handler_ = nullptr;

  HTTPConfigHandler* http_config_handler_ = nullptr;

  SystemStatusLed* system_status_led_ = NULL;
  SystemStatusController system_status_controller_;
  int button_gpio_pin_ = SENSESP_BUTTON_PIN;
  ButtonHandler* button_handler_ = nullptr;

  Networking* networking_ = NULL;
  OTA* ota_;
  SKDeltaQueue* sk_delta_queue_;
  WSClient* ws_client_;

  UIOutput<String>* build_info_ui_output_ =
      new UIOutput<String>("Build date", __DATE__ " " __TIME__, "Software", 2000);
  UIOutput<String>* sensesp_version_ui_output_ = new UIOutput<String>(
      "SenseESP version", kSensESPVersion, "Software", 1900);
  UILambdaOutput<String>* hostname_ui_output_ = new UILambdaOutput<String>(
      "Hostname", [this]() { return this->hostname_->get(); }, "Network", 500);
  UIOutput<String>* mac_address_ui_output_ =
      new UIOutput<String>("MAC Address", WiFi.macAddress(), "Network", 1100);
  UILambdaOutput<String>* wifi_ssid_ui_output_ = new UILambdaOutput<String>(
      "SSID", [this]() { return WiFi.SSID(); }, "Network", 1200);
  UILambdaOutput<int8_t>* wifi_rssi_ui_output_ = new UILambdaOutput<int8_t>(
      "WiFi signal strength", [this]() { return WiFi.RSSI(); }, "Network",
      1300);
  UILambdaOutput<String>* sk_server_address_ui_output_ =
      new UILambdaOutput<String>(
          "Signal K server address",
          [this]() { return ws_client_->get_server_address(); }, "Network",
          1400);
  UILambdaOutput<uint16_t>* sk_server_port_ui_output_ =
      new UILambdaOutput<uint16_t>(
          "Signal K server port",
          [this]() { return ws_client_->get_server_port(); }, "Network", 1500);
  UILambdaOutput<String>* sk_server_connection_ui_output_ =
      new UILambdaOutput<String>(
          "SK connection status",
          [this]() { return ws_client_->get_connection_status(); }, "Network",
          1600);

  friend class WebServer;
  friend class SensESPAppBuilder;
};

extern SensESPApp* sensesp_app;

}  // namespace sensesp

#endif
