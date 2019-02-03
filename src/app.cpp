#include "app.h"

#include "FS.h"

#include "devices/analog_input.h"
#include "devices/digital_input.h"
#include "devices/system_info.h"
#include "net/discovery.h"
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

  // setup all devices and their computations

  setup_standard_devices(hostname);
  setup_custom_devices(hostname);

  // connect all computations to the Signal K delta output

  sk_delta = new SKDelta(hostname->get());
  hostname->attach([hostname, this](){
    this->sk_delta->set_hostname(hostname->get());
  });
  for (auto const& comp : computations) {
    comp->attach([comp, this](){ this->sk_delta->append(comp->as_json()); });
  }

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

void SensESPApp::setup_custom_devices(ObservableValue<String>* hostname) {
  // connect analog input

  connect_1to1<AnalogInput, Passthrough<float>>(
    new AnalogInput(),
    new Passthrough<float>("sensors.indoor.illumination"));

  // connect digital input

  DigitalInput* digital_in_dev = new DigitalInput(D1, INPUT_PULLUP, CHANGE);
  Passthrough<bool>* button_comp = new Passthrough<bool>(
    "sensors.sensesp.button");
  digital_in_dev->attach([digital_in_dev, button_comp](){
    button_comp->set_input(digital_in_dev->get());
  });
  devices.push_back(digital_in_dev);
  computations.push_back(button_comp);

  Frequency* dig_in_freq_comp = new Frequency("sensors.button.frequency");
  digital_in_dev->attach([dig_in_freq_comp](){
    dig_in_freq_comp->tick();
  });
  computations.push_back(dig_in_freq_comp);
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
  setup_discovery(networking->get_hostname()->get().c_str());

  this->http_server->enable();
  this->ws_client->enable();

  Serial.println("WS client enabled");

  for (auto const& dev : devices) {
    dev->enable();
  }

  Serial.println("All devices enabled");
}

void SensESPApp::reset() {
  Serial.println("Resetting the device configuration.");
  networking->reset_settings();
  SPIFFS.format();
  app.onDelay(1000, [](){ ESP.reset(); });
}
