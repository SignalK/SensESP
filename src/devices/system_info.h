#ifndef _systeminfo_H_
#define _systeminfo_H_

#include "device.h"


class SystemHz : public Device {
    uint32_t tick_count = 0;
    uint32_t prev_millis;
    float system_hz;
    void tick();
    void update();
  public:
    void enable() final;
    float get();
};


class FreeMem : public Device {
    uint32_t free_mem;
    void update();
  public:
    void enable() final;
    uint32_t get();
};


class Uptime : public Device {
    float uptime;
    void update();
  public:
    void enable() final;
    float get();
};


#endif
