#ifndef _systeminfo_H_
#define _systeminfo_H_

#include "sensor.h"

class SystemHz : public NumericSensor {
 public:
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
  void enable() override final;
  String get_value_name() { return "freemem"; }
 private:
  void update();
};

class Uptime : public NumericSensor {
 public:
  void enable() override final;
  String get_value_name() { return "uptime"; }
 private:
  void update();
};

class IPAddrDev : public StringSensor {
 public:
  void enable() override final;
  String get_value_name() { return "ipaddr"; }
 private:
  void update();
};

#endif
