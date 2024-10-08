#ifndef SENSESP_APP_H_
#define SENSESP_APP_H_

#ifdef LED_BUILTIN
#define LED_PIN LED_BUILTIN
#define ENABLE_LED true
#else
#define LED_PIN 0
#define ENABLE_LED false
#endif

#include "sensesp/controllers/system_status_controller.h"
#include "sensesp/net/discovery.h"
#include "sensesp/net/http_server.h"
#include "sensesp/net/networking.h"
#include "sensesp/net/ota.h"
#include "sensesp/net/web/app_command_handler.h"
#include "sensesp/net/web/base_command_handler.h"
#include "sensesp/net/web/config_handler.h"
#include "sensesp/net/web/static_file_handler.h"
#include "sensesp/sensesp_version.h"
#include "sensesp/sensors/sensor.h"
#include "sensesp/signalk/signalk_delta_queue.h"
#include "sensesp/signalk/signalk_ws_client.h"
#include "sensesp/system/button.h"
#include "sensesp/system/system_status_led.h"
#include "sensesp/ui/status_page_item.h"
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
  SKWSClient* get_ws_client() { return this->ws_client_; }

 protected:
  /**
   * @brief SensESPApp constructor
   *
   * Note that the constructor is protected, so SensESPApp should only
   * be instantiated using SensESPAppBuilder.
   *
   */
  SensESPApp() : SensESPBaseApp() {}

  ~SensESPApp();

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
  const SensESPApp* set_ap_ssid(const String& ssid) {
    this->ap_ssid_ = ssid;
    return this;
  }
  const SensESPApp* set_ap_password(const String& password) {
    this->ap_password_ = password;
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
  const SensESPApp* set_admin_user(const char* username, const char* password) {
    this->http_server_->set_auth_credentials(username, password, true);
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

  void setup();
  void connect_status_page_items();

  String ssid_ = "";
  String wifi_client_password_ = "";
  String sk_server_address_ = "";
  uint16_t sk_server_port_ = 0;
  String ap_ssid_ = SensESPBaseApp::get_hostname();
  String ap_password_ = "thisisfine";
  const char* ota_password_ = nullptr;

  MDNSDiscovery* mdns_discovery_;
  HTTPServer* http_server_;

  SystemStatusLed* system_status_led_ = NULL;
  SystemStatusController system_status_controller_;
  int button_gpio_pin_ = SENSESP_BUTTON_PIN;
  ButtonHandler* button_handler_ = nullptr;

  Networking* networking_ = NULL;

  OTA* ota_;
  SKDeltaQueue* sk_delta_queue_;
  SKWSClient* ws_client_;

  StatusPageItem<String> sensesp_version_ui_output_{
      "SenseESP version", kSensESPVersion, "Software", 1900};
  StatusPageItem<String> build_info_ui_output_{
      "Build date", __DATE__ " " __TIME__, "Software", 2000};
  StatusPageItem<String> hostname_ui_output_{"Hostname", "", "Network", 500};
  StatusPageItem<String> mac_address_ui_output_{
      "MAC Address", WiFi.macAddress(), "Network", 1100};
  StatusPageItem<String> wifi_ssid_ui_output_{"SSID", "", "Network", 1200};
  StatusPageItem<int> free_memory_ui_output_{"Free memory (bytes)", 0, "System",
                                             1250};
  StatusPageItem<int8_t> wifi_rssi_ui_output_{"WiFi signal strength (dB)", -128,
                                              "Network", 1300};
  StatusPageItem<String> sk_server_address_ui_output_{"Signal K server address",
                                                      "", "Signal K", 1400};
  StatusPageItem<uint16_t> sk_server_port_ui_output_{"Signal K server port", 0,
                                                     "Signal K", 1500};
  StatusPageItem<String> sk_server_connection_ui_output_{"SK connection status",
                                                         "", "Signal K", 1600};
  StatusPageItem<int> delta_tx_count_ui_output_{"SK Delta TX count", 0,
                                                "Signal K", 1700};
  StatusPageItem<int> delta_rx_count_ui_output_{"SK Delta RX count", 0,
                                                "Signal K", 1800};

  friend class WebServer;
  friend class SensESPAppBuilder;
};

extern SensESPApp* sensesp_app;

}  // namespace sensesp

#endif
