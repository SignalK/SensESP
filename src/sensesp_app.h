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

#include "net/http.h"
#include "net/networking.h"
#include "net/ws_client.h"
#include "sensesp.h"
#include "sensors/sensor.h"
#include "signalk/signalk_delta.h"
#include "system/led_blinker.h"
#include "system/observablevalue.h"
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

enum SKPermissions
{
  READONLY,
  READWRITE,
  ADMIN
};

void SetupSerialDebug(uint32_t baudrate);

class SensESPApp {
 public:
  SensESPApp(String hostname = "SensESP", String ssid = "", String wifi_password = "",
             String sk_server_address = "", uint16_t sk_server_port = 0,
             StandardSensors sensors = ALL, int led_pin = LED_PIN,
             bool enable_led = ENABLE_LED, int led_ws_connected = 200,
             int led_wifi_connected = 1000, int led_offline = 2000, SKPermissions permissions = READWRITE);
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
   * Returns true if the host system is connected to Wifi
   */
  bool isWifiConnected() { return WiFi.status() == WL_CONNECTED; }

  /**
   * Returns true if the host system is connected to a Signal K server
   */
  bool isSignalKConnected() { return ws_client->is_connected(); }

 private:
  void initialize();
  void setup_standard_sensors(ObservableValue<String>* hostname,
                              StandardSensors enabled_sensors = ALL);

  HTTPServer* http_server;
  LedBlinker* led_blinker;
  Networking* networking;
  SKDelta* sk_delta;
  WSClient* ws_client;
  String get_permission_string(SKPermissions permission);

  void set_wifi(String ssid, String password);

  friend class HTTPServer;
  friend class SensESPAppBuilder;
};

extern SensESPApp* sensesp_app;

#endif
