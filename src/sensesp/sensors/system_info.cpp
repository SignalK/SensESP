#include "sensesp.h"

#include "system_info.h"

#include <WiFi.h>

#include "Arduino.h"

namespace sensesp {

void SystemHz::update() {
  // getting sporadic divide by 0 exceptions, no harm in skipping a loop.
  if (elapsed_millis_ == 0) {
    return;
  }

  uint32_t current_tick_count_ = event_loop()->getTickCount();
  output_ =
      (current_tick_count_ - last_tick_count_) / (elapsed_millis_ / 1000.);

  last_tick_count_ = current_tick_count_;
  elapsed_millis_ = 0;

  this->notify();
}

void FreeMem::update() { this->emit(ESP.getFreeHeap()); }

void Uptime::update() { this->emit(static_cast<double>(millis()) / 1000.); }

void IPAddrDev::update() { this->emit(WiFi.localIP().toString()); }

void WiFiSignal::update() { this->emit(WiFi.RSSI()); }

}  // namespace sensesp
