#include "sensesp_app.h"

#include "sensesp/net/discovery.h"
#include "sensesp/net/networking.h"
#include "sensesp/net/ota.h"
#include "sensesp/net/web/autogen/frontend_files.h"
#include "sensesp/system/button.h"
#include "sensesp/system/system_status_led.h"
#include "sensesp/transforms/debounce.h"

namespace sensesp {

SensESPApp* SensESPApp::get() {
  if (instance_ == nullptr) {
    instance_ = new SensESPApp();
  }
  return static_cast<SensESPApp*>(instance_);
}

/**
 * @brief Perform initialization of SensESPApp once builder configuration is
 * done.
 *
 * This should be only called from the builder!
 *
 */
void SensESPApp::setup() {
  // call the parent setup()
  SensESPBaseApp::setup();

  // create the networking object
  networking_ =
      new Networking("/System/WiFi Settings", ssid_, wifi_client_password_);

  if (ota_password_ != nullptr) {
    // create the OTA object
    ota_ = new OTA(ota_password_);
  }

  bool captive_portal_enabled = networking_->is_captive_portal_enabled();

  // create the HTTP server
  this->http_server_ = new HTTPServer();
  this->http_server_->set_captive_portal(captive_portal_enabled);

  // Add the default HTTP server response handlers
  add_static_file_handlers(this->http_server_);
  add_base_app_http_command_handlers(this->http_server_);
  add_app_http_command_handlers(this->http_server_);
  add_config_handlers(this->http_server_);

  // create the SK delta object
  sk_delta_queue_ = new SKDeltaQueue();

  // create the websocket client
  bool const use_mdns = sk_server_address_ == "";
  this->ws_client_ =
      new SKWSClient("/System/Signal K Settings", sk_delta_queue_,
                     sk_server_address_, sk_server_port_, use_mdns);

  // connect the system status controller
  WiFiStateProducer::get_singleton()->connect_to(&system_status_controller_);
  this->ws_client_->connect_to(&system_status_controller_);

  // create the MDNS discovery object
  mdns_discovery_ = new MDNSDiscovery();

  // create a system status led and connect it

  if (system_status_led_ == NULL) {
    system_status_led_ = new SystemStatusLed(LED_PIN);
  }
  this->system_status_controller_.connect_to(system_status_led_);
  this->ws_client_->get_delta_tx_count_producer().connect_to(
      system_status_led_);

  // create the button handler
  if (button_gpio_pin_ != -1) {
    button_handler_ = new ButtonHandler(button_gpio_pin_);
  }

  // connect status page items
  connect_status_page_items();
}

void SensESPApp::connect_status_page_items() {
  this->hostname_->connect_to(this->hostname_ui_output_);
  this->event_loop_.onRepeat(
      4999, [this]() { wifi_ssid_ui_output_->set(WiFi.SSID()); });
  this->event_loop_.onRepeat(
      4998, [this]() { free_memory_ui_output_->set(ESP.getFreeHeap()); });
  this->event_loop_.onRepeat(
      4997, [this]() { wifi_rssi_ui_output_->set(WiFi.RSSI()); });
  this->event_loop_.onRepeat(4996, [this]() {
    sk_server_address_ui_output_->set(ws_client_->get_server_address());
  });
  this->event_loop_.onRepeat(4995, [this]() {
    sk_server_port_ui_output_->set(ws_client_->get_server_port());
  });
  this->event_loop_.onRepeat(4994, [this]() {
    sk_server_connection_ui_output_->set(ws_client_->get_connection_status());
  });
  ws_client_->get_delta_tx_count_producer().connect_to(
      delta_tx_count_ui_output_);
  ws_client_->get_delta_rx_count_producer().connect_to(
      delta_rx_count_ui_output_);
}

ObservableValue<String>* SensESPApp::get_hostname_observable() {
  return hostname_;
}

SensESPApp* sensesp_app;

}  // namespace sensesp
