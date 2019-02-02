#ifndef _systeminfo_H_
#define _systeminfo_H_

#include "device.h"

class SystemHz : public Device {
 public:
  void enable() override final;
  float get();
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
 private:
  uint32_t free_mem;
  void update();
};

class Uptime : public Device {
 public:
  void enable() override final;
  float get();
 private:
  float uptime;
  void update();
};

#endif
