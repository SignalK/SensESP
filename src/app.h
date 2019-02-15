#ifndef _app_H_
#define _app_H_

#include <set>

#include "config.h"
#include "transforms/transform.h"
#include "devices/device.h"
#include "net/http.h"
#include "net/networking.h"
#include "net/ws_client.h"
#include "sensesp.h"
#include "system/led_blinker.h"
#include "system/signal_k.h"

class SensESPApp {
 public:
  SensESPApp();
  void enable();
  void reset();
 private:
  std::set<Device*> devices;
  std::set<Transform*> transforms;

  void setup_standard_devices(ObservableValue<String>* hostname);
  void setup_custom_devices();

  template<typename T, typename U>
  void connect_1to1(T* device, U* transform) {
    device->attach([device, transform](){
      transform->set_input(device->get());
    });
    devices.insert(device);
    transforms.insert(transform);
  }

  template<typename T, typename U>
  void connect_1to1_h(T* device, U* transform,
                      ObservableValue<String>* hostname) {
    String hostname_str = hostname->get();
    String value_name = device->get_value_name();
    String sk_path = "sensors." + hostname_str + "." + value_name;
    auto comp_set_sk_path = [hostname, transform, value_name](){
        transform->set_sk_path(
          "sensors." + hostname->get() + "." + value_name);
    };
    comp_set_sk_path();
    device->attach([device, transform](){
      transform->set_input(device->get());
    });
    hostname->attach(comp_set_sk_path);
    devices.insert(device);
    transforms.insert(transform);
  }

  HTTPServer* http_server;
  LedBlinker led_blinker;
  Networking* networking;
  SKDelta* sk_delta;
  WSClient* ws_client;

};

extern SensESPApp* sensesp_app;

#endif
