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
  String hostname_str = hostname->get();

  // connect systemhz

  SystemHz* syshz_dev = new SystemHz();
  Passthrough<float>* syshz_comp = new Passthrough<float>(
    "sensors." + hostname_str + ".system_hz");
  // update the SK path if hostname changes
  hostname->attach(
    [hostname, syshz_comp](){
      syshz_comp->set_sk_path(
        "sensors." + hostname->get() + ".system_hz");
  });
  syshz_dev->attach([syshz_dev, syshz_comp](){
    syshz_comp->set_input(syshz_dev->get());
  });
  devices.push_back(syshz_dev);
  computations.push_back(syshz_comp);

  // connect freemem

  FreeMem* freemem_dev = new FreeMem();
  Passthrough<float>* freemem_comp = new Passthrough<float>(
    "sensors." + hostname_str + ".freemem");
  // update the SK path if hostname changes
  hostname->attach(
    [hostname, freemem_comp](){
      freemem_comp->set_sk_path(
        "sensors." + hostname->get() + ".freemem");
  });
  freemem_dev->attach([freemem_dev, freemem_comp](){
    freemem_comp->set_input(freemem_dev->get());
  });
  devices.push_back(freemem_dev);
  computations.push_back(freemem_comp);

  // connect uptime (exaggerate the value!)

  Uptime* uptime_dev = new Uptime();
  Linear* uptime_comp = new Linear(
    "sensors." + hostname_str + ".uptime", 1.2, 3600.,
    "/comp/uptime");
  // update the SK path if hostname changes
  hostname->attach(
    [hostname, uptime_comp](){
      uptime_comp->set_sk_path(
        "sensors." + hostname->get() + ".uptime");
  });
  uptime_dev->attach([uptime_dev, uptime_comp](){
    uptime_comp->set_input(uptime_dev->get());
  });
  devices.push_back(uptime_dev);
  computations.push_back(uptime_comp);

  // connect analog input

  AnalogInput* analog_in_dev = new AnalogInput();
  Passthrough<float>* analog_comp = new Passthrough<float>(
    "sensors.indoor.illumination");
  analog_in_dev->attach([analog_in_dev, analog_comp](){
    analog_comp->set_input(analog_in_dev->get());
  });
  devices.push_back(analog_in_dev);
  computations.push_back(analog_comp);

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
