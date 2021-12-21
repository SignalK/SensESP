#ifndef _system_info_H_
#define _system_info_H_

#include <Arduino.h>
#include <elapsedMillis.h>

#include "sensesp/signalk/signalk_output.h"
#include "sensesp_base_app.h"
#include "sensor.h"

namespace sensesp {

/**
 * @brief Connect a system information sensor to SKOutput
 **/
template <typename T>
void connect_system_info_sensor(SensorT<T>* sensor, String prefix,
                                String name) {
  auto hostname_obs = SensESPBaseApp::get()->get_hostname_observable();
  String hostname = hostname_obs->get();
  String path = prefix + hostname + "." + name;

  auto* sk_output = new SKOutput<T>(path);

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
class SystemHz : public FloatSensor {
 public:
  SystemHz() {}
  void start() override final;
  String get_value_name() { return "systemhz"; }

 private:
  uint32_t tick_count_ = 0;
  elapsedMillis elapsed_millis_;
  float system_hz_;
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
class FreeMem : public IntSensor {
 public:
  FreeMem() {}
  void start() override final;
  String get_value_name() { return "freemem"; }

 private:
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
class Uptime : public FloatSensor {
 public:
  Uptime() {}
  void start() override final;
  String get_value_name() { return "uptime"; }

 private:
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
class IPAddrDev : public StringSensor {
 public:
  IPAddrDev() {}
  void start() override final;
  String get_value_name() { return "ipaddr"; }

 private:
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
class WiFiSignal : public FloatSensor {
 public:
  WiFiSignal() {}
  void start() override final;
  String get_value_name() { return "wifisignal"; }

 private:
  void update();
};

}  // namespace sensesp

#endif
