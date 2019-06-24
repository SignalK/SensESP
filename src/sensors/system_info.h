#ifndef _systeminfo_H_
#define _systeminfo_H_

#include "sensor.h"

class SystemHz : public NumericSensor {
 public:
  SystemHz() { className = "SystemHz"; }
  void enable() override final;
  String get_value_name() { return "systemhz"; }
 private:
  uint32_t tick_count = 0;
  uint32_t prev_millis;
  float system_hz;
  void tick();
  void update();
};

class FreeMem : public Sensor, public ValueProducer<uint32_t> {
 public:
  FreeMem() { className = "FreeMem"; }
  void enable() override final;
  String get_value_name() { return "freemem"; }
 private:
  void update();
};

class Uptime : public NumericSensor {
 public:
  Uptime() { className = "Uptime"; }
  void enable() override final;
  String get_value_name() { return "uptime"; }
 private:
  void update();
};

class IPAddrDev : public StringSensor {
 public:
  IPAddrDev() { className = "IPAddrDev"; }
  void enable() override final;
  String get_value_name() { return "ipaddr"; }
 private:
  void update();
};

#endif
