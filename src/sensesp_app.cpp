#include "sensesp_app.h"

#include "FS.h"

#include "devices/analog_input.h"
#include "devices/digital_input.h"
#include "devices/system_info.h"
#include "net/discovery.h"
#include "net/ota.h"
#include "net/networking.h"
#include "system/spiffs_storage.h"
#include "transforms/transform.h"
#include "transforms/difference.h"
#include "transforms/frequency.h"
#include "transforms/linear.h"
#include "transforms/passthrough.h"

#ifndef DEBUG_DISABLED
RemoteDebug Debug;
#endif

// FIXME: Setting up the system is too verbose and repetitive

SensESPApp::SensESPApp() {
  // initialize filesystem

  if (!SPIFFS.begin()) {
    debugE("FATAL: Filesystem initialization failed.");
    ESP.restart();
  }

  // create the networking object
  networking = new Networking("/system/networking", "");
  ObservableValue<String>* hostname = networking->get_hostname();

  // setup standard devices and their transforms

  setup_standard_devices(hostname);

  // create the SK delta object

  sk_delta = new SKDelta(hostname->get());

  // listen for hostname updates

  hostname->attach([hostname, this](){
    this->sk_delta->set_hostname(hostname->get());
  });

  // create the HTTP server

  this->http_server = new HTTPServer(std::bind(&SensESPApp::reset, this));

  // create the websocket client

  auto ws_connected_cb = [this](bool connected){
    if (connected) {
      this->led_blinker.set_server_connected();
    } else {
      this->led_blinker.set_server_disconnected();
    }
  };
  auto ws_delta_cb = [this](){
    this->led_blinker.flip();
  };
  this->ws_client = new WSClient(
    "/system/sk", "",
    sk_delta, ws_connected_cb, ws_delta_cb);
}

void SensESPApp::setup_standard_devices(ObservableValue<String>* hostname) {

  // connect systemhz

  connect_1to1_h<SystemHz, Passthrough<float>>(
    new SystemHz(),
    new Passthrough<float>(),
    hostname
  );

  String hostname_str = hostname->get();

  // connect freemem

  connect_1to1_h<FreeMem, Passthrough<float>>(
    new FreeMem(),
    new Passthrough<float>(),
    hostname
  );

  // connect uptime

  connect_1to1_h<Uptime, Passthrough<float>>(
    new Uptime(),
    new Passthrough<float>(),
    hostname
  );

  // connect ip address

  connect_1to1_h<IPAddrDev, Passthrough<String>>(
    new IPAddrDev(),
    new Passthrough<String>(),
    hostname
  );
}

void SensESPApp::enable() {
  this->led_blinker.set_wifi_disconnected();

  // connect all transforms to the Signal K delta output

  ObservableValue<String>* hostname = networking->get_hostname();

  for (auto const& transf : TransformBase::get_transforms()) {
    if (transf->get_sk_path()!="") {
      transf->attach([transf, this](){
        this->sk_delta->append(transf->as_json());
      });
    }
  }

  debugI("Enabling subsystems");

  networking->setup([this](bool connected) {
    if (connected) {
      this->led_blinker.set_wifi_connected();
    } else {
      this->led_blinker.set_wifi_disconnected();
    }
  });

  setup_OTA();
  setup_discovery(networking->get_hostname()->get().c_str());

  this->http_server->enable();
  this->ws_client->enable();

  debugI("WS client enabled");

  // initialize remote debugging

  #ifndef DEBUG_DISABLED
  Debug.begin(networking->get_hostname()->get());
  Debug.setResetCmdEnabled(true);
  app.onRepeat(1, [](){ Debug.handle(); });
  #endif

  for (auto const& dev : Device::get_devices()) {
    dev->enable();
  }

  debugI("All devices enabled");

  for (auto const& transf : TransformBase::get_transforms()) {
    transf->enable();
  }
  debugI("All transforms enabled");
}

void SensESPApp::reset() {
  debugW("Resetting the device configuration.");
  networking->reset_settings();
  SPIFFS.format();
  app.onDelay(1000, [](){ ESP.reset(); });
}

String SensESPApp::get_hostname() {
  return networking->get_hostname()->get();
}

SensESPApp* sensesp_app;
