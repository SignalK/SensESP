#include "system_info.h"

#include "Arduino.h"

#include "sensesp.h"

void SystemHz::tick() {
  tick_count++;
}

void SystemHz::update() {
  uint32_t cur_millis = millis();
  uint32_t elapsed = cur_millis - prev_millis;

  // getting sporadic divide by 0 exceptions, no harm in skipping a loop.
  if (elapsed == 0) {
    return;
  }

  system_hz = (tick_count*1000) / elapsed;

  tick_count = 0;
  prev_millis = cur_millis;

  this->notify();
}

void SystemHz::enable() {
  prev_millis = millis();

  app.onTick([this](){ this->tick(); });
  app.onRepeat(1000, [this](){ this->update(); });
}

float SystemHz::get() {
  return this->system_hz;
}

void FreeMem::update() {
  free_mem = ESP.getFreeHeap();
  this->notify();
}

void FreeMem::enable() {
  app.onRepeat(1000, [this](){ this->update(); });
}

uint32_t FreeMem::get() {
  return this->free_mem;
}

void Uptime::update() {
  uptime = millis() / 1000.;
  this->notify();
}

void Uptime::enable() {
  app.onRepeat(1000, [this](){ this->update(); });
}

float Uptime::get() {
  return this->uptime;
}

