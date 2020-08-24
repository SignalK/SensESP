#ifndef SENSESP_APP_BUILDER_H
#define SENSESP_APP_BUILDER_H

#include "sensesp_app.h"

class SensESPAppBuilder {
 private:
  String hostname = "";
  String ssid = "";
  String password = "";
  String sk_server_address = "";
  int sk_server_port = 0;
  StandardSensors sensors = ALL;
  int led_pin = LED_PIN;
  bool enable_led = ENABLE_LED;
  int led_ws_connected = 200;
  int led_wifi_connected = 1000;
  int led_offline = 5000;

 public:
  SensESPAppBuilder() {}
  SensESPAppBuilder* set_wifi(String ssid, String password) {
    this->ssid = ssid;
    this->password = password;
    return this;
  }
  SensESPAppBuilder* set_sk_server(String address, int port) {
    this->sk_server_address = address;
    this->sk_server_port = port;
    return this;
  }
  SensESPAppBuilder* set_standard_sensors(StandardSensors sensors = ALL) {
    this->sensors = sensors;
    return this;
  }
  SensESPAppBuilder* set_led_pin(int led_pin) {
    this->led_pin = led_pin;
    return this;
  }
  SensESPAppBuilder* set_led_blinker(bool enabled, int websocket_connected,
                                     int wifi_connected, int offline) {
    this->enable_led = enabled;
    this->led_ws_connected = websocket_connected;
    this->led_wifi_connected = wifi_connected;
    this->led_offline = offline;
    return this;
  }
  SensESPAppBuilder* set_hostname(String hostname) {
    this->hostname = hostname;
    return this;
  }
  SensESPApp* get_app() {
    return new SensESPApp(hostname, ssid, password, sk_server_address,
                          sk_server_port, sensors, led_pin, enable_led,
                          led_ws_connected, led_wifi_connected, led_offline);
  }
};

#endif