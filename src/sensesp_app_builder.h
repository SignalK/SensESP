#ifndef SENSESP_APP_BUILDER_H
#define SENSESP_APP_BUILDER_H

#include "sensesp_app.h"

class SensESPAppBuilder {
 private:
  String hostname = "SensESP";
  String ssid = "";
  String password = "";
  String sk_server_address = "";
  uint16_t sk_server_port = 0;
  StandardSensors sensors = ALL;
  int led_pin = LED_PIN;
  bool enable_led = ENABLE_LED;
  int led_ws_connected = 200;
  int led_wifi_connected = 1000;
  int led_offline = 5000;
  SKPermissions sk_server_permissions = READWRITE;

  SensESPApp* app;

 public:
  SensESPAppBuilder() {
    app = new SensESPApp(true);
  }
  SensESPAppBuilder* set_wifi(String ssid, String password) {
    app->set_ssid(ssid);
    app->set_wifi_password(password);
    return this;
  }
  SensESPAppBuilder* set_sk_server(String address, uint16_t port, SKPermissions permissions = READWRITE) {
    app->set_sk_server_address(address);
    app->set_sk_server_port(port);
    app->set_requested_permissions(permissions);
    return this;
  }
  SensESPAppBuilder* set_standard_sensors(StandardSensors sensors = ALL) {
    app->set_sensors(sensors);
    return this;
  }
  SensESPAppBuilder* set_led_pin(int led_pin) {
    app->set_led_pin(led_pin);
    return this;
  }
  SensESPAppBuilder* set_led_blinker(bool enabled, int websocket_connected,
                                     int wifi_connected, int offline) {
    app->set_enable_led(enabled);
    app->set_led_ws_connected(websocket_connected);
    app->set_led_wifi_connected(wifi_connected);
    app->set_led_offline(offline);
    return this;
  }
  SensESPAppBuilder* set_hostname(String hostname) {
    app->set_preset_hostname(hostname);
    return this;
  }

  SensESPApp* get_app() {
    app->setup();
    return app;
  }
};

#endif