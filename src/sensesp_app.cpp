#include "sensesp_app.h"

#ifdef ESP8266
#include "FS.h"
#elif defined(ESP32)
#include "SPIFFS.h"
#endif

#include "net/discovery.h"
#include "net/networking.h"
#include "net/ota.h"
#include "sensors/analog_input.h"
#include "sensors/digital_input.h"
#include "sensors/system_info.h"
#include "signalk/signalk_output.h"
#include "system/spiffs_storage.h"
#include "transforms/difference.h"
#include "transforms/frequency.h"
#include "transforms/linear.h"
#include "transforms/transform.h"

#ifndef DEBUG_DISABLED
RemoteDebug Debug;
#endif

void SetupSerialDebug(uint32_t baudrate) {
  Serial.begin(baudrate);

  // A small delay and one debugI() are required so that
  // the serial output displays everything
  delay(100);
  Debug.setSerialEnabled(true);
  delay(100);
  debugI("\nSerial debug enabled");
}

static char* permission_strings[] = {"readonly", "readwrite", "admin"};

/*
 * This constructor must be only used in SensESPAppBuilder
 */
SensESPApp::SensESPApp(bool defer_setup) {}

SensESPApp::SensESPApp(String preset_hostname, String ssid,
                       String wifi_password, String sk_server_address,
                       uint16_t sk_server_port)
    : preset_hostname{preset_hostname},
      ssid{ssid},
      wifi_password{wifi_password},
      sk_server_address{sk_server_address},
      sk_server_port{sk_server_port} {
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
  networking = new Networking("/system/networking", ssid, wifi_password,
                              preset_hostname);

  ObservableValue<String>* hostname = networking->get_hostname();

  // setup standard sensors and their transforms
  setup_standard_sensors(hostname, sensors);

  // create the SK delta object

  sk_delta = new SKDelta(hostname->get());

  // listen for hostname updates

  hostname->attach(
      [hostname, this]() { this->sk_delta->set_hostname(hostname->get()); });

  // create the HTTP server

  this->http_server = new HTTPServer(std::bind(&SensESPApp::reset, this));

  // create the websocket client

  this->ws_client =
      new WSClient("/system/sk", sk_delta, sk_server_address, sk_server_port,
                                 permission_strings[(int)requested_permissions]);

  // create a led controller and connect it to its data sources

  led_controller = new LedController(led_pin);
  this->networking->connect_to(led_controller);
  this->ws_client->connect_to(led_controller);
  this->ws_client->get_delta_count_producer().connect_to(led_controller);

}

void SensESPApp::setup_standard_sensors(ObservableValue<String>* hostname,
                                        StandardSensors enabled_sensors) {
  if ((enabled_sensors & FREQUENCY) != 0) {
    connect_1to1_h<SystemHz, SKOutput<float>>(new SystemHz(),
                                              new SKOutput<float>(), hostname);
  }

  if ((enabled_sensors & UPTIME) != 0) {
    connect_1to1_h<Uptime, SKOutput<float>>(new Uptime(), new SKOutput<float>(),
                                            hostname);
  }

  // connect freemem
  if ((enabled_sensors & FREE_MEMORY) != 0) {
    connect_1to1_h<FreeMem, SKOutput<float>>(new FreeMem(),
                                             new SKOutput<float>(), hostname);
  }

  // connect ip address

  if ((enabled_sensors & IP_ADDRESS) != 0) {
    connect_1to1_h<IPAddrDev, SKOutput<String>>(
        new IPAddrDev(), new SKOutput<String>(), hostname);
  }

  if ((enabled_sensors & WIFI_SIGNAL) != 0) {
    connect_1to1_h<WifiSignal, SKOutput<float>>(
        new WifiSignal(), new SKOutput<float>(), hostname);
  }
}

void SensESPApp::enable() {
  // connect all transforms to the Signal K delta output

  // ObservableValue<String>* hostname = networking->get_hostname();

  for (auto const& sk_source : SKEmitter::get_sources()) {
    if (sk_source->get_sk_path() != "") {
      debugI("Connecting Signal K source %s", sk_source->get_sk_path().c_str());
      sk_source->attach([sk_source, this]() {
        this->sk_delta->append(sk_source->as_signalk());
      });
    }
  }

  debugI("Enabling subsystems");

  // FIXME: Setting up mDNS discovery before networking can't work!
  setup_discovery(networking->get_hostname()->get().c_str());

  networking->setup();

  setup_ota();

  this->http_server->enable();
  this->ws_client->enable();

  // initialize remote debugging

#ifndef DEBUG_DISABLED
  Debug.begin(networking->get_hostname()->get());
  Debug.setResetCmdEnabled(true);
  app.onRepeat(1, []() { Debug.handle(); });
#endif

  Enable::enable_all();
  debugI("All sensors and transforms enabled");
}

void SensESPApp::reset() {
  debugW("Resetting the device configuration.");
  networking->reset_settings();
  SPIFFS.format();
  app.onDelay(1000, []() { ESP.restart(); });
}

String SensESPApp::get_hostname() { return networking->get_hostname()->get(); }

SensESPApp* sensesp_app;
