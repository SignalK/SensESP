#include "app.h"

#include "FS.h"

#include "devices/analog_input.h"
#include "devices/system_info.h"
#include "net/ota.h"
#include "net/networking.h"
#include "system/spiffs_storage.h"


// FIXME: Setting up the system is too verbose and repetitive

SensESPApp::SensESPApp() {
  // initialize filesystem

  if (!SPIFFS.begin()) {
    Serial.println("FATAL: Filesystem initialization failed.");
    ESP.restart();
  }

  // create the networking object
  networking = new Networking("/system/networking", "");
  ObservableValue<String>* hostname = networking->get_hostname();

  // connect systemhz

  SystemHz* syshz = new SystemHz();
  Passthrough<float>* syshz_value = new Passthrough<float>("sensors.unknown.system_hz");
  syshz->attach([syshz, syshz_value](){ syshz_value->set_input(syshz->get()); });
  devices.push_back(syshz);
  computations.push_back(syshz_value);

  // connect freemem

  FreeMem* freemem = new FreeMem();
  Passthrough<float>* freemem_value = new Passthrough<float>("sensors.unknown.freemem");
  freemem->attach([freemem, freemem_value](){ freemem_value->set_input(freemem->get()); });
  devices.push_back(freemem);
  computations.push_back(freemem_value);

  // connect uptime (exaggerate the value!)

  Uptime* uptime = new Uptime();
  Linear* uptime_value = new Linear("sensors.unknown.uptime", 1.2, 3600.,
                                    "/comp/uptime");
  uptime->attach([uptime, uptime_value](){ uptime_value->set_input(uptime->get()); });
  devices.push_back(uptime);
  computations.push_back(uptime_value);

  // connect analog input

  AnalogInput* analog_in_dev = new AnalogInput();
  Passthrough<float>* analog_comp = new Passthrough<float>(
    "sensors.unknown.analog");
  analog_in_dev->attach([analog_in_dev, analog_comp](){ 
    analog_comp->set_input(analog_in_dev->get()); 
  });
  devices.push_back(analog_in_dev);
  computations.push_back(analog_comp);

  // connect all computations to the Signal K delta output

  sk_delta = new SKDelta(hostname->get());
  hostname->attach([hostname, this](){ 
    this->sk_delta->set_hostname(hostname->get()); 
  });
  for (auto const& comp : computations) {
    comp->attach([comp, this](){ this->sk_delta->append(comp->as_json()); });
  }

  // create the HTTP server

  this->http_server = new HTTPServer();

  // create the websocket client

  auto ws_connected_cb = [this](){
    this->led_blinker.set_server_connected();
  };
  auto ws_disconnected_cb = [this](){
    this->led_blinker.set_wifi_connected();
  };
  auto ws_delta_cb = [this](){
    this->led_blinker.flip();
  };
  this->ws_client = new WSClient(
    sk_delta, ws_connected_cb, ws_disconnected_cb, ws_delta_cb);

}

void SensESPApp::enable() {
  this->led_blinker.set_wifi_disconnected();
  
  Serial.println("Enabling subsystems");
  
  networking->setup([this](bool connected) {
    if (connected) {
      this->led_blinker.set_wifi_connected();
    } else {
      this->led_blinker.set_wifi_disconnected();
    }
  });

  setup_OTA();

  this->http_server->enable();
  this->ws_client->enable();

  Serial.println("WS client enabled");

  for (auto const& dev : devices) {
    dev->enable();
  }

  Serial.println("All devices enabled");
}
