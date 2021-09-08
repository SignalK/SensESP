#include "sensesp_app.h"

#ifdef ESP8266
#include "FS.h"
#elif defined(ESP32)
#include "SPIFFS.h"
#endif

#include "net/discovery.h"
#include "net/networking.h"
#include "net/ota.h"
#include "system/spiffs_storage.h"
#include "system/system_status_led.h"
#include "transforms/debounce.h"

#ifndef DEBUG_DISABLED
RemoteDebug Debug;
#endif

void SetupSerialDebug(uint32_t baudrate) {
  Serial.begin(baudrate);

  // A small delay and one debugI() are required so that
  // the serial output displays everything
#ifndef DEBUG_DISABLED
  delay(100);
  Debug.setSerialEnabled(true);
  delay(100);
#endif 
  debugI("\nSerial debug enabled");
}

/*
 * This constructor must be only used in SensESPAppBuilder
 */
SensESPApp::SensESPApp(bool defer_setup) {}

SensESPApp::SensESPApp(String preset_hostname, String ssid,
                       String wifi_password, String sk_server_address,
                       uint16_t sk_server_port)
    : preset_hostname_{preset_hostname},
      ssid_{ssid},
      wifi_password_{wifi_password},
      sk_server_address_{sk_server_address},
      sk_server_port_{sk_server_port} {
  setup();
}

void SensESPApp::setup() {
  // initialize filesystem
#ifdef ESP8266
  if (!SPIFFS.begin()) {
#elif defined(ESP32)
  if (!SPIFFS.begin(true)) {
#endif
    debugE("FATAL: Filesystem initialization failed.");
    ESP.restart();
  }

  // create the networking object
  networking_ = new Networking("/system/networking", ssid_, wifi_password_,
                               preset_hostname_);

  // TODO: hostname should work even without networking
  ObservableValue<String>* hostname = networking_->get_hostname();

  // create the SK delta object
  // TODO: one queue per output path
  sk_delta_queue_ = new SKDeltaQueue(hostname->get());

  // listen for hostname updates

  hostname->attach(
      [hostname, this]() { this->sk_delta_queue_->set_hostname(hostname->get()); });

  // create the HTTP server
  // TODO: make conditional
  this->http_server_ = new HTTPServer([this]() { this->reset(); });

  // create the websocket client
  // TODO: make conditional
  this->ws_client_ =
      new WSClient("/system/sk", sk_delta_queue_, sk_server_address_, sk_server_port_);

  // connect the system status controller
  // TODO: make conditional
  this->networking_->connect_to(&system_status_controller_);
  this->ws_client_->connect_to(&system_status_controller_);

  // create the wifi disconnect watchdog
  // TODO: make conditional
  this->system_status_controller_
    .connect_to(new DebounceTemplate<SystemStatus>(
      3*60*1000, // 180 s = 180000 ms = 3 minutes
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

  if (system_status_led_ == NULL) {
    system_status_led_ = new SystemStatusLed(LED_PIN);
  }
  this->system_status_controller_.connect_to(system_status_led_);
  this->ws_client_->get_delta_count_producer().connect_to(system_status_led_);
}

void SensESPApp::enable() {
  // connect all transforms to the Signal K delta output

  // ObservableValue<String>* hostname = networking->get_hostname();

  this->sk_delta_queue_->connect_emitters();

  debugI("Enabling subsystems");

  // FIXME: Setting up mDNS discovery before networking can't work!
  setup_discovery(networking_->get_hostname()->get().c_str());

  networking_->setup();

  setup_ota();

  this->http_server_->enable();
  this->ws_client_->enable();

  // initialize remote debugging

#ifndef DEBUG_DISABLED
  Debug.begin(networking_->get_hostname()->get());
  Debug.setResetCmdEnabled(true);
  app.onRepeat(1, []() { Debug.handle(); });
#endif

  Enable::enable_all();
  debugI("All sensors and transforms enabled");
}

void SensESPApp::reset() {
  debugW("Resetting the device configuration.");
  networking_->reset_settings();
  SPIFFS.format();
  app.onDelay(1000, []() { ESP.restart(); delay(1000); });
}

String SensESPApp::get_hostname() { return networking_->get_hostname()->get(); }

SensESPApp* sensesp_app;
