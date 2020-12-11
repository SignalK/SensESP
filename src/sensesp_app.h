#ifndef _sensesp_app_H_
#define _sensesp_app_H_

// Required for RemoteDebug
#define USE_LIB_WEBSOCKET true

#ifdef LED_BUILTIN
#define LED_PIN LED_BUILTIN
#define ENABLE_LED true
#else
#define LED_PIN 0
#define ENABLE_LED false
#endif

#include <forward_list>

#include "controllers/system_status_controller.h"
#include "net/http.h"
#include "net/networking.h"
#include "net/ws_client.h"
#include "sensesp.h"
#include "sensors/sensor.h"
#include "signalk/signalk_delta.h"
#include "system/observablevalue.h"
#include "system/system_status_led.h"
#include "system/valueconsumer.h"
#include "system/valueproducer.h"

enum StandardSensors {
  NONE,
  UPTIME = 0x01,
  FREQUENCY = 0x02,
  FREE_MEMORY = 0x04,
  IP_ADDRESS = 0x08,
  WIFI_SIGNAL = 0x10,
  ALL = 0x1F
};

enum SKPermissions { READONLY, READWRITE, ADMIN };

void SetupSerialDebug(uint32_t baudrate);

class SensESPApp {
 public:
  SensESPApp(bool defer_setup);
  SensESPApp(String hostname = "SensESP", String ssid = "",
             String wifi_password = "", String sk_server_address = "",
             uint16_t sk_server_port = 0);
  void setup();
  void enable();
  void reset();
  String get_hostname();

  template <typename T>
  void connect(ValueProducer<T>* producer, ValueConsumer<T>* consumer,
               uint8_t inputChannel = 0) {
    producer->connect_to(consumer, inputChannel);
  }

  template <typename T, typename U>
  void connect_1to1_h(T* sensor, U* transform,
                      ObservableValue<String>* hostname) {
    String hostname_str = hostname->get();
    String value_name = sensor->get_value_name();
    String sk_path = hostname_str + "." + value_name;
    auto comp_set_sk_path = [hostname, transform, value_name]() {
      transform->set_sk_path(hostname->get() + "." + value_name);
    };
    comp_set_sk_path();
    sensor->attach(
        [sensor, transform]() { transform->set_input(sensor->get()); });
    hostname->attach(comp_set_sk_path);
  }

  /**
   * Sends the specified payload text directly to the signalk server
   * over the connected websocket. If no websocket is connect, the
   * call is ignored.
   */
  void send_to_server(String& payload) { this->ws_client_->sendTXT(payload); }

  /**
   * Returns true if the host system is connected to Wifi
   */
  bool isWifiConnected() { return WiFi.status() == WL_CONNECTED; }

  /**
   * Returns true if the host system is connected to a Signal K server
   */
  bool isSignalKConnected() { return ws_client_->is_connected(); }

  // getters for internal members
  SKDelta* get_sk_delta() { return this->sk_delta_; }
  SystemStatusController* get_system_status_controller() {
    return &(this->system_status_controller_);
  }
  Networking* get_networking() { return this->networking_; }
  WSClient* get_ws_client() { return this->ws_client_; }

 protected:
  // setters for all constructor arguments

  const SensESPApp* set_preset_hostname(String preset_hostname) {
    this->preset_hostname_ = preset_hostname;
    return this;
  }
  const SensESPApp* set_ssid(String ssid) {
    this->ssid_ = ssid;
    return this;
  }
  const SensESPApp* set_wifi_password(String wifi_password) {
    this->wifi_password_ = wifi_password;
    return this;
  }
  const SensESPApp* set_sk_server_address(String sk_server_address) {
    this->sk_server_address_ = sk_server_address;
    return this;
  }
  const SensESPApp* set_sk_server_port(uint16_t sk_server_port) {
    this->sk_server_port_ = sk_server_port;
    return this;
  }
  const SensESPApp* set_system_status_led(SystemStatusLed* system_status_led) {
    this->system_status_led_ = system_status_led;
    return this;
  }
  const SensESPApp* set_sensors(StandardSensors sensors) {
    this->sensors_ = sensors;
    return this;
  }
  const SensESPApp* set_requested_permissions(SKPermissions permissions) {
    this->requested_permissions_ = permissions;
    return this;
  }

 private:
  String preset_hostname_ = "SensESP";
  String ssid_ = "";
  String wifi_password_ = "";
  String sk_server_address_ = "";
  uint16_t sk_server_port_ = 0;
  StandardSensors sensors_ = ALL;
  SKPermissions requested_permissions_ = READWRITE;

  void initialize();
  void setup_standard_sensors(ObservableValue<String>* hostname,
                              StandardSensors enabled_sensors = ALL);

  HTTPServer* http_server_;
  SystemStatusLed* system_status_led_ = NULL;
  SystemStatusController system_status_controller_;
  Networking* networking_;
  SKDelta* sk_delta_;
  WSClient* ws_client_;
  String get_permission_string(SKPermissions permission);

  void set_wifi(String ssid, String password);

  friend class HTTPServer;
  friend class SensESPAppBuilder;
};

extern SensESPApp* sensesp_app;

#endif
