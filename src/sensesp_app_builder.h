#ifndef SENSESP_APP_BUILDER_H
#define SENSESP_APP_BUILDER_H

#include "sensesp_app.h"

class SensESPAppBuilder {
 private:
  String hostname_ = "SensESP";
  String ssid_ = "";
  String password_ = "";
  String sk_server_address_ = "";
  uint16_t sk_server_port_ = 0;
  StandardSensors sensors_ = ALL;
  SKPermissions sk_server_permissions_ = READWRITE;

  SensESPApp* app;

 public:
  SensESPAppBuilder() { app = new SensESPApp(true); }
  SensESPAppBuilder* set_wifi(String ssid, String password) {
    app->set_ssid(ssid);
    app->set_wifi_password(password);
    return this;
  }
  SensESPAppBuilder* set_sk_server(String address, uint16_t port,
                                   SKPermissions permissions = READWRITE) {
    app->set_sk_server_address(address);
    app->set_sk_server_port(port);
    app->set_requested_permissions(permissions);
    return this;
  }
  SensESPAppBuilder* set_standard_sensors(StandardSensors sensors = ALL) {
    app->set_sensors(sensors);
    return this;
  }
  SensESPAppBuilder* set_hostname(String hostname) {
    app->set_preset_hostname(hostname);
    return this;
  }
  SensESPAppBuilder* set_system_status_led(SystemStatusLed* system_status_led) {
    app->set_system_status_led(system_status_led);
    return this;
  }
  SensESPAppBuilder* set_test_auth_on_each_connect(bool val) {
    WSClient::test_auth_on_each_connect_ = val;
    return this;
  }
  SensESPApp* get_app() {
    app->setup();
    return app;
  }
};

#endif