#include "sensesp.h"

#include "system_info.h"

#include <WiFi.h>

#include "Arduino.h"
#include "sensesp_app.h"

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

void IPAddrDev::update() {
  // Query the active network provisioner so this sensor works for both
  // WiFi and (future) non-WiFi transports. Defensively null-check the
  // app singleton: IPAddrDev::update() can run on any event-loop tick,
  // including before setup() has finished wiring the provisioner.
  auto app = SensESPApp::get();
  if (!app) {
    this->emit(String("0.0.0.0"));
    return;
  }
  auto provisioner = app->get_network_provisioner();
  if (provisioner) {
    this->emit(provisioner->local_ip().toString());
  } else {
    this->emit(String("0.0.0.0"));
  }
}

// WiFiSignal is intentionally still WiFi-specific by name and behaviour.
// Builder code that calls enable_wifi_signal_sensor() on a non-WiFi
// deployment will still get this sensor, but it will report 0 since
// there is no WiFi association. The user is responsible for not enabling
// it on non-WiFi-only devices.
void WiFiSignal::update() { this->emit(WiFi.RSSI()); }

}  // namespace sensesp
