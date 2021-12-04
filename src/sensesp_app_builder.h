#ifndef SENSESP_APP_BUILDER_H
#define SENSESP_APP_BUILDER_H

#include "sensesp_app.h"
#include "sensesp_base_app_builder.h"
#include "sensors/system_info.h"

const char* kDefaultSystemInfoSensorPrefix = "sensorDevice.";

namespace sensesp {

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
  SensESPAppBuilder() { app_ = SensESPApp::get(); }
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
  SensESPAppBuilder* set_hostname(String hostname) override final {
    app_->set_hostname(hostname);
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
  SensESPAppBuilder* enable_system_hz_sensor(String prefix=kDefaultSystemInfoSensorPrefix) {
    connect_system_info_sensor(new SystemHz(), prefix, "systemHz");
    return this;
  }
  SensESPAppBuilder* enable_free_mem_sensor(String prefix=kDefaultSystemInfoSensorPrefix) {
    connect_system_info_sensor(new FreeMem(), prefix, "freeMemory");
    return this;
  }
  SensESPAppBuilder* enable_uptime_sensor(String prefix=kDefaultSystemInfoSensorPrefix) {
    connect_system_info_sensor(new Uptime(), prefix, "uptime");
    return this;
  }
  SensESPAppBuilder* enable_ip_address_sensor(String prefix=kDefaultSystemInfoSensorPrefix) {
    connect_system_info_sensor(new IPAddrDev(), prefix, "ipAddress");
    return this;
  }
  SensESPAppBuilder* enable_wifi_signal_sensor(String prefix=kDefaultSystemInfoSensorPrefix) {
    connect_system_info_sensor(new WiFiSignal(), prefix, "wifiSignalLevel");
    return this;
  }

  SensESPAppBuilder* enable_system_info_sensors(String prefix=kDefaultSystemInfoSensorPrefix) {
    this->enable_system_hz_sensor(prefix);
    this->enable_free_mem_sensor(prefix);
    this->enable_uptime_sensor(prefix);
    this->enable_ip_address_sensor(prefix);
    this->enable_wifi_signal_sensor(prefix);
    return this;
  }

  /**
   * @brief Enable over-the-air updates for the device.
   *
   * @param password OTA authentication password.
   * @return SensESPAppBuilder*
   */
  SensESPAppBuilder* enable_ota(const char* password) {
    app_->enable_ota(password);
    return this;
  }
  SensESPApp* get_app() override final {
    app_->setup();
    return app_;
  }
};

}  // namespace sensesp

#endif
