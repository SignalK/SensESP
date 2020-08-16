
#include "led_blinker.h"

#include "sensesp.h"
#include "sensesp_app.h"
#include "sensesp_app_options.h"

LedBlinker::LedBlinker(int pin, bool enabled, LedIntervals_T intervals) {
  this->pin = pin;
  this->enabled = enabled;
  this->intervals = intervals;

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

  if (intervals.offlineInterval > 0) {
    this->blinker = app.onRepeat(intervals.offlineInterval, [this]() {
      this->set_state(1);
      app.onDelay(100, [this]() { this->set_state(0); });
    });
  }
}

void LedBlinker::set_wifi_connected() {
  this->remove_blinker();
  this->blinker =
      app.onRepeat(intervals.wifiConnected, [this]() { this->flip(); });
}

void LedBlinker::set_server_connected() {
  this->remove_blinker();
  this->blinker =
      app.onRepeat(intervals.websocketConnected, [this]() { this->flip(); });
}
