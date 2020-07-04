#include "sensesp_app.h"

#ifdef ESP8266
#include "FS.h"
#elif defined(ESP32)
#include "SPIFFS.h"
#endif

#include "sensors/analog_input.h"
#include "sensors/digital_input.h"
#include "sensors/system_info.h"
#include "net/discovery.h"
#include "net/ota.h"
#include "net/networking.h"
#include "system/spiffs_storage.h"
#include "transforms/transform.h"
#include "transforms/difference.h"
#include "transforms/frequency.h"
#include "transforms/linear.h"
#include "signalk/signalk_output.h"

#ifndef DEBUG_DISABLED
RemoteDebug Debug;
#endif

// FIXME: Setting up the system is too verbose and repetitive

SensESPApp::SensESPApp(StdSensors_t stdSensors) : stdSensors{stdSensors} {
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
  networking = new Networking("/system/networking");
  ObservableValue<String>* hostname = networking->get_hostname();

  // setup standard sensors and their transforms

  setup_standard_sensors(hostname, stdSensors);

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
    "/system/sk",
    sk_delta, ws_connected_cb, ws_delta_cb);
}

void SensESPApp::setup_standard_sensors(ObservableValue<String>* hostname, StdSensors_t stdSensors) {

  if (stdSensors == noStdSensors) {return;};

  if (stdSensors == allStdSensors) {
    // connect systemhz

    connect_1to1_h<SystemHz, SKOutput<float>>(
      new SystemHz(),
      new SKOutput<float>(),
      hostname
    );

    // connect freemem

    connect_1to1_h<FreeMem, SKOutput<float>>(
      new FreeMem(),
      new SKOutput<float>(),
      hostname
    );

    // connect ip address

    connect_1to1_h<IPAddrDev, SKOutput<String>>(
      new IPAddrDev(),
      new SKOutput<String>(),
      hostname
    );
  }

  if (stdSensors == allStdSensors || stdSensors == uptimeOnly) {
  // connect uptime

    connect_1to1_h<Uptime, SKOutput<float>>(
      new Uptime(),
      new SKOutput<float>(),
      hostname
    );
  }
  
}

void SensESPApp::enable() {
  this->led_blinker.set_wifi_disconnected();

  // connect all transforms to the Signal K delta output

  // ObservableValue<String>* hostname = networking->get_hostname();

  for (auto const& sigkSource : SKEmitter::get_sources()) {
    if (sigkSource->get_sk_path() != "") {
      debugI("Connecting SignalK source %s", sigkSource->get_sk_path().c_str());
      sigkSource->attach([sigkSource, this](){
        this->sk_delta->append(sigkSource->as_signalK());
      });
    }
  }

  debugI("Enabling subsystems");

  debugI("Subsystem: setup_discovery()");
  setup_discovery(networking->get_hostname()->get().c_str());

  debugI("Subsystem: networking->setup()");
  networking->setup([this](bool connected) {
    if (connected) {
      this->led_blinker.set_wifi_connected();
    } else {
      this->led_blinker.set_wifi_disconnected();
      debugD("Not connected to wifi");
    }
  });

  debugI("Subsystem: setup_OTA()");
  setup_OTA();
  
  debugI("Subsystem: http_server()");
  this->http_server->enable();
  debugI("Subsystem: ws_client()");
  this->ws_client->enable();

  debugI("WS client enabled");

  // initialize remote debugging

  #ifndef DEBUG_DISABLED
  Debug.begin(networking->get_hostname()->get());
  Debug.setResetCmdEnabled(true);
  app.onRepeat(1, [](){ Debug.handle(); });
  #endif

  Enable::enableAll();
  debugI("All sensors and transforms enabled");

}

void SensESPApp::reset() {
  debugW("Resetting the device configuration.");
  networking->reset_settings();
  SPIFFS.format();
  app.onDelay(1000, [](){ ESP.restart(); });
}

String SensESPApp::get_hostname() {
  return networking->get_hostname()->get();
}

SensESPApp* sensesp_app;
