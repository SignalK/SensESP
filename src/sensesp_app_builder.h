#ifndef SENSESP_APP_BUILDER_H
#define SENSESP_APP_BUILDER_H

#include "sensesp_base_app_builder.h"
#include "sensesp_app.h"

/**
 * @brief A class for quickly configuring a SensESP application object before
 * wiring up your sensors.
 */
class SensESPAppBuilder : public SensESPBaseAppBuilder {
 private:
  String hostname_ = "SensESP";
  String ssid_ = "";
  String password_ = "";
  String sk_server_address_ = "";
  uint16_t sk_server_port_ = 0;

 protected:
  SensESPApp* app_;


 public:
  SensESPAppBuilder() { app_ = new SensESPApp(true); }
  SensESPAppBuilder* set_wifi(String ssid, String password) {
    app_->set_ssid(ssid);
    app_->set_wifi_password(password);
    return this;
  }
  SensESPAppBuilder* set_sk_server(String address, uint16_t port) {
    app_->set_sk_server_address(address);
    app_->set_sk_server_port(port);
    return this;
  }
  SensESPAppBuilder* set_hostname(String hostname) {
    this->SensESPBaseAppBuilder::set_hostname(hostname);
    return this;
  }
  SensESPAppBuilder* set_system_status_led(SystemStatusLed* system_status_led) {
    app_->set_system_status_led(system_status_led);
    return this;
  }
  SensESPAppBuilder* set_test_auth_on_each_connect(bool val) {
    WSClient::test_auth_on_each_connect_ = val;
    return this;
  }
  SensESPApp* get_app() {
    app_->setup();
    return app_;
  }
};

#endif