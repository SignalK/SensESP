#ifndef _systeminfo_H_
#define _systeminfo_H_

#include "device.h"

class SystemHz : public Device {
 public:
  void enable() override final;
  float get();
  String get_value_name() { return "systemhz"; }
 private:
  uint32_t tick_count = 0;
  uint32_t prev_millis;
  float system_hz;
  void tick();
  void update();
};

class FreeMem : public Device {
 public:
  void enable() override final;
  uint32_t get();
  String get_value_name() { return "freemem"; }
 private:
  uint32_t free_mem;
  void update();
};

class Uptime : public Device {
 public:
  void enable() override final;
  float get();
  String get_value_name() { return "uptime"; }
 private:
  float uptime;
  void update();
};

#endif
