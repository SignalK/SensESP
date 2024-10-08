#ifndef SENSESP_APP_BUILDER_H
#define SENSESP_APP_BUILDER_H

#include <memory>

#include "Arduino.h"
#include "sensesp/sensors/system_info.h"
#include "sensesp/system/valueproducer.h"
#include "sensesp/transforms/debounce.h"
#include "sensesp_app.h"
#include "sensesp_base_app_builder.h"

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
  std::shared_ptr<SensESPApp> app_;

 public:
  /**
   * @brief Construct a new SensESPApp Builder object.
   *
   * SensESPAppBuilder is used to instantiate a SensESPApp
   * object with non-trivial configuration.
   */
  SensESPAppBuilder() {
    app_ = std::shared_ptr<SensESPApp>(new SensESPApp());
    app_->set_instance(app_);
  }
  /**
   * @brief Set the Wi-Fi network SSID and password.
   *
   * If not set, WiFiManager is used to create an access point for configuring
   * the settings.
   *
   * @param ssid
   * @param password
   * @return SensESPAppBuilder*
   */
  SensESPAppBuilder* set_wifi_client(String ssid, String password) {
    app_->set_ssid(ssid);
    app_->set_wifi_password(password);
    return this;
  }

  [[deprecated("Use set_wifi_client instead")]]
  SensESPAppBuilder* set_wifi(String ssid, String password) {
    return set_wifi_client(ssid, password);
  }

  /**
   * @brief Set the wifi access point object SSID and password.
   *
   * To disable the SSID, set both to empty strings.
   *
   * @param ssid
   * @param password
   * @return SensESPAppBuilder*
   */
  SensESPAppBuilder* set_wifi_access_point(const String& ssid,
                                           const String& password) {
    app_->set_ap_ssid(ssid);
    app_->set_ap_password(password);
    return this;
  }

  /**
   * @brief Set the Signal K server address and port.
   *
   * If not set, mDNS is used to discover the Signal K server.
   *
   * @param address
   * @param port
   * @return SensESPAppBuilder*
   */
  SensESPAppBuilder* set_sk_server(String address, uint16_t port) {
    app_->set_sk_server_address(address);
    app_->set_sk_server_port(port);
    return this;
  }
  /**
   * @brief Set the device hostname.
   *
   * If not set, the device hostname is set to "SensESP".
   *
   * @param hostname
   * @return SensESPAppBuilder*
   */
  SensESPAppBuilder* set_hostname(String hostname) override final {
    app_->set_hostname(hostname);
    return this;
  }
  /**
   * @brief Set admin username and password for the web interface.
   *
   */
  SensESPAppBuilder* set_admin_user(const char* username,
                                    const char* password) {
    app_->set_admin_user(username, password);
    return this;
  }

  /**
   * @brief Set the system status led object.
   *
   * This allows custom status LED patterns to be used.
   *
   * @param system_status_led
   * @return SensESPAppBuilder*
   */
  SensESPAppBuilder* set_system_status_led(
      std::shared_ptr<SystemStatusLed> system_status_led) {
    app_->set_system_status_led(system_status_led);
    return this;
  }
  /**
   * @brief Enable the System Hz sensor.
   *
   * The System Hz sensor is a built-in sensor that measures how many
   * times the system loop is executed per second.
   *
   * @param prefix
   * @return SensESPAppBuilder*
   */
  SensESPAppBuilder* enable_system_hz_sensor(
      String prefix = kDefaultSystemInfoSensorPrefix) {
    auto sensor = std::make_shared<SystemHz>();
    auto vproducer = std::static_pointer_cast<ValueProducer<float>>(sensor);
    // We need to store the sensor in the app so it doesn't get garbage
    // collected.
    app_->system_hz_sensor_ = vproducer;
    connect_system_info_sensor(vproducer, prefix, "systemHz");
    return this;
  }
  /**
   * @brief Enable the free memory sensor.
   *
   * @param prefix
   * @return SensESPAppBuilder*
   */
  SensESPAppBuilder* enable_free_mem_sensor(
      String prefix = kDefaultSystemInfoSensorPrefix) {
    auto sensor = std::make_shared<FreeMem>();
    auto vproducer = std::static_pointer_cast<ValueProducer<uint32_t>>(sensor);
    // We need to store the sensor in the app so it doesn't get garbage
    // collected.
    app_->free_mem_sensor_ = vproducer;
    connect_system_info_sensor(vproducer, prefix, "freeMemory");
    return this;
  }
  /**
   * @brief Report the system uptime in seconds since the last reboot.
   *
   * @param prefix
   * @return SensESPAppBuilder*
   */
  SensESPAppBuilder* enable_uptime_sensor(
      String prefix = kDefaultSystemInfoSensorPrefix) {
    auto sensor = std::make_shared<Uptime>();
    auto vproducer = std::static_pointer_cast<ValueProducer<float>>(sensor);
    // We need to store the sensor in the app so it doesn't get garbage
    // collected.
    app_->uptime_sensor_ = vproducer;
    connect_system_info_sensor(vproducer, prefix, "uptime");
    return this;
  }
  /**
   * @brief Report the IP address of the device.
   *
   * @param prefix
   * @return SensESPAppBuilder*
   */
  SensESPAppBuilder* enable_ip_address_sensor(
      String prefix = kDefaultSystemInfoSensorPrefix) {
    auto sensor = std::make_shared<IPAddrDev>();
    auto vproducer = std::static_pointer_cast<ValueProducer<String>>(sensor);
    // We need to store the sensor in the app so it doesn't get garbage
    // collected.
    app_->ip_address_sensor_ = vproducer;
    connect_system_info_sensor(vproducer, prefix, "ipAddress");
    return this;
  }
  /**
   * @brief Report the Wi-Fi signal strength.
   *
   * @param prefix
   * @return SensESPAppBuilder*
   */
  SensESPAppBuilder* enable_wifi_signal_sensor(
      String prefix = kDefaultSystemInfoSensorPrefix) {
    auto sensor = std::make_shared<WiFiSignal>();
    auto vproducer = std::static_pointer_cast<ValueProducer<int>>(sensor);
    // We need to store the sensor in the app so it doesn't get garbage
    // collected.
    app_->wifi_signal_sensor_ = vproducer;
    connect_system_info_sensor(vproducer, prefix, "wifiSignalLevel");
    return this;
  }

  /**
   * @brief Set the button GPIO pin.
   *
   * @return SensESPAppBuilder*
   */
  SensESPAppBuilder* set_button_pin(int pin) {
    app_->set_button_pin(pin);
    return this;
  }

  /**
   * @brief Enable all built-in system info sensors.
   *
   * @param prefix
   * @return SensESPAppBuilder*
   */
  SensESPAppBuilder* enable_system_info_sensors(
      String prefix = kDefaultSystemInfoSensorPrefix) {
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

  /**
   * @brief Set the wifi manager password.
   *
   * Set the password for the WiFi configuration access point
   * that is enabled after device reset if no wifi configuration
   * is provided in the application code.
   *
   * @param password
   * @return SensESPAppBuilder*
   */
  [[deprecated("Use set_wifi_access_point instead")]]
  SensESPAppBuilder* set_wifi_manager_password(const char* password) {
    app_->set_ap_password(password);
    return this;
  }

  const SensESPAppBuilder* enable_wifi_watchdog() {
    // create the wifi disconnect watchdog
    app_->system_status_controller_
        ->connect_to(new Debounce<SystemStatus>(
            3 * 60 * 1000  // 180 s = 180000 ms = 3 minutes
            ))
        ->connect_to(new LambdaConsumer<SystemStatus>([](SystemStatus input) {
          ESP_LOGD(__FILENAME__, "Got system status: %d", (int)input);
          if (input == SystemStatus::kWifiDisconnected ||
              input == SystemStatus::kWifiNoAP) {
            ESP_LOGW(__FILENAME__,
                     "Unable to connect to wifi for too long; restarting.");
            event_loop()->onDelay(1000, []() { ESP.restart(); });
          }
        }));

    return this;
  }

  /**
   * @brief Get the SensESPApp object.
   *
   * Return a SensESPApp object that has been setup.
   *
   * @return SensESPApp*
   */
  std::shared_ptr<SensESPApp> get_app() {
    app_ = SensESPApp::get();
    app_->setup();
    return app_;
  }
};

}  // namespace sensesp

#endif
