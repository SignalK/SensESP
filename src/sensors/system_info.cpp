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

  output = (tick_count*1000) / elapsed;

  tick_count = 0;
  prev_millis = cur_millis;

  this->notify();
}

void SystemHz::enable() {
  prev_millis = millis();

  app.onTick([this](){ this->tick(); });
  app.onRepeat(1000, [this](){ this->update(); });
}


void FreeMem::update() {
  output = ESP.getFreeHeap();
  this->notify();
}

void FreeMem::enable() {
  app.onRepeat(1000, [this](){ this->update(); });
}


void Uptime::update() {
  output = millis() / 1000.;
  this->notify();
}

void Uptime::enable() {
  app.onRepeat(1000, [this](){ this->update(); });
}


void IPAddrDev::update() {
  output = WiFi.localIP().toString();
  this->notify();
}

void IPAddrDev::enable() {
  app.onRepeat(10000, [this](){ this->update(); });
}
