#include "sensesp.h"

#include "system_info.h"

#include <WiFi.h>

#include "Arduino.h"

namespace sensesp {

void SystemHz::tick() { tick_count_++; }

void SystemHz::update() {
  // getting sporadic divide by 0 exceptions, no harm in skipping a loop.
  if (elapsed_millis_ == 0) {
    return;
  }

  output = (tick_count_ * 1000) / elapsed_millis_;

  tick_count_ = 0;
  elapsed_millis_ = 0;

  this->notify();
}

void FreeMem::update() { this->emit(ESP.getFreeHeap()); }

void Uptime::update() { this->emit(static_cast<double>(millis()) / 1000.); }

void IPAddrDev::update() { this->emit(WiFi.localIP().toString()); }

void WiFiSignal::update() { this->emit(WiFi.RSSI()); }

}  // namespace sensesp
