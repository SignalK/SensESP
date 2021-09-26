#include "sensesp_app.h"

#include "net/discovery.h"
#include "net/networking.h"
#include "net/ota.h"
#include "system/system_status_led.h"
#include "transforms/debounce.h"

/*
 * This constructor must be only used in SensESPAppBuilder
 */
SensESPApp::SensESPApp() : SensESPBaseApp() {}

SensESPApp* SensESPApp::get() {
  if (instance_ == nullptr) {
    instance_ = new SensESPApp();
  }
  return (SensESPApp*)instance_;
}

void SensESPApp::setup() {
  // call the parent setup()
  SensESPBaseApp::setup();

  // create the networking object
  networking_ = new Networking("/system/networking", ssid_, wifi_password_,
                               preset_hostname_);

  // create the OTA object
  ota_ = new OTA();

  // create the HTTP server
  // TODO: make conditional
  this->http_server_ = new HTTPServer();

  // create the SK delta object
  // TODO: one queue per output path
  sk_delta_queue_ = new SKDeltaQueue();

  // create the websocket client
  // TODO: make conditional
  this->ws_client_ = new WSClient("/system/sk", sk_delta_queue_,
                                  sk_server_address_, sk_server_port_);

  // connect the system status controller
  // TODO: make conditional
  this->networking_->connect_to(&system_status_controller_);
  this->ws_client_->connect_to(&system_status_controller_);

  // create the MDNS discovery object
  mdns_discovery_ = new MDNSDiscovery();

  // create the wifi disconnect watchdog
  // TODO: make conditional
  this->system_status_controller_
      .connect_to(new DebounceTemplate<SystemStatus>(
          3 * 60 * 1000,  // 180 s = 180000 ms = 3 minutes
          "/system/wifi_reboot_watchdog"))
      ->connect_to(new LambdaConsumer<SystemStatus>([](SystemStatus input) {
        debugD("Got system status: %d", (int)input);
        if (input == SystemStatus::kWifiDisconnected ||
            input == SystemStatus::kWifiNoAP) {
          debugW("Unable to connect to wifi for too long; restarting.");
          app.onDelay(1000, []() { ESP.restart(); });
        }
      }));

  // create a system status led and connect it

  // TODO: must not depend on networking and ws_client etc
  if (system_status_led_ == NULL) {
    system_status_led_ = new SystemStatusLed(LED_PIN);
  }
  this->system_status_controller_.connect_to(system_status_led_);
  this->ws_client_->get_delta_count_producer().connect_to(system_status_led_);
}

ObservableValue<String>* SensESPApp::get_hostname_observable() {
  return hostname_;
}

SensESPApp* sensesp_app;
