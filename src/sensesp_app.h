#ifndef _app_H_
#define _app_H_

// Required for RemoteDebug
#define USE_LIB_WEBSOCKET true

#include "sensesp_app_options.h"
#include "sensors/sensor.h"
#include "net/http.h"
#include "net/networking.h"
#include "net/ws_client.h"
#include "sensesp.h"
#include "system/led_blinker.h"
#include "signalk/signalk_delta.h"
#include "system/valueproducer.h"
#include "system/valueconsumer.h"
#include "system/observablevalue.h"
#include "sensesp_app_options.h"

class SensESPApp {
 public:
  SensESPApp(SensESPAppOptions* appOptions);
  void enable();
  void reset();
  String get_hostname();
  SensESPAppOptions* getOptions() { return options; }


  template<typename T>
  void connect(ValueProducer<T>* pProducer, ValueConsumer<T>* pConsumer, uint8_t inputChannel = 0) {
      pProducer->connectTo(pConsumer, inputChannel);
  }

  template<typename T, typename U>
  void connect_1to1_h(T* sensor, U* transform,
                      ObservableValue<String>* hostname) {
    String hostname_str = hostname->get();
    String value_name = sensor->get_value_name();
    String sk_path = hostname_str + "." + value_name;
    auto comp_set_sk_path = [hostname, transform, value_name](){
        transform->set_sk_path(
          hostname->get() + "." + value_name);
    };
    comp_set_sk_path();
    sensor->attach([sensor, transform](){
      transform->set_input(sensor->get());
    });
    hostname->attach(comp_set_sk_path);
  }

 /**
  * Returns true if the host system is connected to Wifi
  */
  bool isWifiConnected() {
     return WiFi.status() == WL_CONNECTED;
  }


  /**
   * Returns true if the host system is connected to a SignalK server
   */
  bool isSignalKConnected() {
    return ws_client->is_connected();
  }


 private:
  SensESPAppOptions* options;
  void setup_standard_sensors(ObservableValue<String>* hostname);

  HTTPServer* http_server;
  LedBlinker led_blinker;
  Networking* networking;
  SKDelta* sk_delta;
  WSClient* ws_client;

};

extern SensESPApp* sensesp_app;

#endif
