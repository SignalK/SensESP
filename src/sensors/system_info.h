#ifndef _systeminfo_H_
#define _systeminfo_H_

#include "sensor.h"
#include <elapsedMillis.h>

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
class FreeMem : public Sensor, public ValueProducer<uint32_t> {
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
class WifiSignal : public FloatSensor {
 public:
  WifiSignal() {}
  void start() override final;
  String get_value_name() { return "wifisignal"; }

 private:
  void update();
};

#endif
