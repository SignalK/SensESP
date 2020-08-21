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
  StandardSensors sensors = NONE;
  int led_pin = LED_PIN;
  bool enable_led = ENABLE_LED;
  int led_ws_connected = 200;
  int led_wifi_connected = 1000;
  int led_offline = 5000;

 public:
  SensESPAppBuilder() {}
  SensESPAppBuilder* set_wifi(String ssid, String password) {
    ssid = ssid;
    password = password;
    return this;
  }
  SensESPAppBuilder* set_sk_server(String address, int port) {
    sk_server_address = address;
    sk_server_port = port;
    return this;
  }
  SensESPAppBuilder* set_standard_sensors(StandardSensors sensors = ALL) {
    sensors = sensors;
    return this;
  }
  SensESPAppBuilder* set_led_pin(int led_pin) {
    led_pin = led_pin;
    return this;
  }
  SensESPAppBuilder* set_led_blinker(bool enabled, int websocket_connected,
                                     int wifi_connected, int offline) {
    enable_led = enabled;
    led_ws_connected = websocket_connected;
    led_wifi_connected = wifi_connected;
    led_offline = offline;
    return this;
  }
  SensESPAppBuilder* set_hostname(String hostname) {
    hostname = hostname;
    return this;
  }
  SensESPApp* get_app() {
    return new SensESPApp(hostname, ssid, password, sk_server_address,
                          sk_server_port, sensors, led_pin, enable_led,
                          led_ws_connected, led_wifi_connected, led_offline);
  }
};

#endif