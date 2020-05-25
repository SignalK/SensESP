
#include "led_blinker.h"

#include "sensesp.h"

#ifndef LED_PIN
#define LED_PIN LED_BUILTIN  // If your board doesn't have a defined LED_BUILTIN, comment this line out...
//#define LED_PIN 2          // ... and uncomment this line, and change 13 to any LED pin you have. If you have none,
                             // you can add an external LED to an available GPIO pin and use that pin number here.
#endif

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
  digitalWrite(LED_PIN, !new_state);
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
