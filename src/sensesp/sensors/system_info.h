#ifndef SENSESP_SENSORS_SYSTEM_INFO_H_
#define SENSESP_SENSORS_SYSTEM_INFO_H_

#include <Arduino.h>
#include <elapsedMillis.h>
#include <memory>

#include "sensesp/signalk/signalk_output.h"
#include "sensesp_base_app.h"
#include "sensor.h"

namespace sensesp {

/**
 * @brief Connect a system information sensor to SKOutput
 **/
template <typename T>
void connect_system_info_sensor(std::shared_ptr<ValueProducer<T>>& sensor,
                                const String& prefix, const String& name) {
  auto hostname_obs = SensESPBaseApp::get()->get_hostname_observable();
  String hostname = hostname_obs->get();
  String path = prefix + hostname + "." + name;

  auto sk_output = std::make_shared<SKOutput<T>>(path);

  // connect an observer to hostname to change the output path
  // if the hostname is changed
  auto update_output_sk_path = [hostname_obs, sk_output, prefix, name]() {
    String path = prefix + hostname_obs->get() + "." + name;
    sk_output->set_sk_path(path);
  };
  update_output_sk_path();

  hostname_obs->attach(update_output_sk_path);

  // connect the sensor to the output
  sensor->connect_to(sk_output);
}

/**
 * @brief Reports the current clock speed of the ESP.
 *
 * Nothing about it is configurable, except whether or not it
 * appears in your project's output. That is configured with
 * SensESPAppBuilder.
 **/
class SystemHz : public ValueProducer<float> {
 public:
  SystemHz() {
    elapsed_millis_ = 0;

    event_loop()->onTick([this]() { this->tick(); });
    event_loop()->onRepeat(1000, [this]() { this->update(); });
  }
  String get_value_name() { return "systemhz"; }

 protected:
  uint32_t tick_count_ = 0;
  elapsedMillis elapsed_millis_;
  void tick();
  void update();
};

/**
 * @brief Reports the current amount of unused memory of the ESP.
 *
 * Nothing about it is configurable, except whether or not it
 * appears in your project's output. That is configured with
 * SensESPAppBuilder.
 **/
class FreeMem : public ValueProducer<uint32_t> {
 public:
  FreeMem() {
    event_loop()->onRepeat(1000, [this]() { this->update(); });
  }
  String get_value_name() { return "freemem"; }

 protected:
  void update();
};

/**
 * @brief Reports the number of seconds since the last restart of
 * the ESP.
 *
 * Nothing about it is configurable, except whether or not it
 * appears in your project's output. That is configured with
 * SensESPAppBuilder.
 **/
class Uptime : public ValueProducer<float> {
 public:
  Uptime() {
    event_loop()->onRepeat(1000, [this]() { this->update(); });
  }
  String get_value_name() { return "uptime"; }

 protected:
  void update();
};

/**
 * @brief Reports the IP address of the ESP once it's connected
 * to wifi.
 *
 * Nothing about it is configurable, except whether or not it
 * appears in your project's output. That is configured with
 * SensESPAppBuilder.
 **/
class IPAddrDev : public ValueProducer<String> {
 public:
  IPAddrDev() {
    event_loop()->onRepeat(10000, [this]() { this->update(); });
  }
  String get_value_name() { return "ipaddr"; }

 protected:
  void update();
};

/**
 * @brief Reports the current strength of the wifi signal that
 * the ESP is connected to.
 *
 * Nothing about it is configurable, except whether or not it
 * appears in your project's output. That is configured with
 * SensESPAppBuilder.
 **/
class WiFiSignal : public ValueProducer<int> {
 public:
  WiFiSignal() {
    event_loop()->onRepeat(3000, [this]() { this->update(); });
  }
  String get_value_name() { return "wifisignal"; }

 protected:
  void update();
};

}  // namespace sensesp

#endif
