
#include "../config.h"
#include "led_blinker.h"
#include "sensesp.h"

LedBlinker::LedBlinker() {
  pinMode(LED_PIN, OUTPUT);
  this->set_state(0);
}

void LedBlinker::remove_blinker() {
  if (this->blinker != nullptr) {
    this->blinker->remove();
  }
}

void LedBlinker::set_state(int new_state) {
  this->current_state = new_state;
  #ifdef LED_ACTIVE_LOW
  digitalWrite(LED_PIN, !new_state);
  #else
  digitalWrite(LED_PIN, new_state);
  #endif
}

void LedBlinker::flip() {
  this->set_state(!this->current_state);
}

void LedBlinker::set_wifi_disconnected() {
  this->remove_blinker();
  this->blinker = app.onRepeat(1000, [this] () {
    this->set_state(1);
    app.onDelay(100, [this] () {
      this->set_state(0);
    });
  });
}

void LedBlinker::set_wifi_connected() {
  this->remove_blinker();
  this->blinker = app.onRepeat(1000, [this] () {
    this->flip();
  });
}

void LedBlinker::set_server_connected() {
  this->remove_blinker();
  this->blinker = app.onRepeat(200, [this] () {
    this->flip();
  });
}
