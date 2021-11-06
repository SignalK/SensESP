#include "system_info.h"

#include "Arduino.h"
#if defined(ESP8266)
#include <ESP8266WiFi.h>
#elif defined(ESP32)
#include <WiFi.h>
#endif
#include "sensesp.h"

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

void SystemHz::start() {
  elapsed_millis_ = 0;

  app.onTick([this]() { this->tick(); });
  app.onRepeat(1000, [this]() { this->update(); });
}

void FreeMem::update() { this->emit(ESP.getFreeHeap()); }

void FreeMem::start() {
  app.onRepeat(1000, [this]() { this->update(); });
}

void Uptime::update() { this->emit(millis() / 1000.); }

void Uptime::start() {
  app.onRepeat(1000, [this]() { this->update(); });
}

void IPAddrDev::update() { this->emit(WiFi.localIP().toString()); }

void IPAddrDev::start() {
  app.onRepeat(10000, [this]() { this->update(); });
}

void WifiSignal::start() {
  app.onRepeat(3000, [this]() { this->update(); });
}

void WifiSignal::update() { this->emit(WiFi.RSSI()); }

}  // namespace sensesp
