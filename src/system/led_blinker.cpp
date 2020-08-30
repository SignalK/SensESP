
#include "led_blinker.h"

#include "sensesp.h"
#include "sensesp_app.h"

LedBlinker::LedBlinker(int pin, bool enabled, int ws_connected_interval,
                       int wifi_connected_interval, int offline_interval) {
  this->pin = pin;
  this->enabled = enabled;
  this->ws_connected_interval = ws_connected_interval;
  this->wifi_connected_interval = wifi_connected_interval;
  this->offline_interval = offline_interval;

  if (enabled) {
    pinMode(pin, OUTPUT);
    this->set_state(0);
  }
}

void LedBlinker::remove_blinker() {
  if (this->blinker != nullptr) {
    this->blinker->remove();
  }
}

void LedBlinker::set_state(int new_state) {
  if (enabled) {
    this->current_state = new_state;
    digitalWrite(pin, !new_state);
  }
}

void LedBlinker::flip() {
  if (enabled) {
    this->set_state(!this->current_state);
  }
}

void LedBlinker::set_wifi_disconnected() {
  this->remove_blinker();

  if (offline_interval > 0) {
    this->blinker = app.onRepeat(offline_interval, [this]() {
      this->set_state(1);
      app.onDelay(100, [this]() { this->set_state(0); });
    });
  }
}

void LedBlinker::set_wifi_connected() {
  this->remove_blinker();
  this->blinker =
      app.onRepeat(wifi_connected_interval, [this]() { this->flip(); });
}

void LedBlinker::set_server_connected() {
  this->remove_blinker();
  this->blinker =
      app.onRepeat(ws_connected_interval, [this]() { this->flip(); });
}
