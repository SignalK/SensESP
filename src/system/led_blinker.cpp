
#include "led_blinker.h"

#include "net/networking.h"
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

void LedBlinker::set_input(WifiState new_value, uint8_t input_channel) {
  switch (new_value) {
    case kWifiNoAP:
      this->set_wifi_disconnected();
      break;
    case kWifiDisconnected:
      this->set_wifi_disconnected();
      break;
    case kWifiConnectedToAP:
      this->set_wifi_connected();
      break;
    case kExecutingWifiManager:
      this->set_wifi_disconnected();
      break;
    default:
      this->set_wifi_disconnected();
      break;
  }
}

void LedBlinker::set_input(WSConnectionState new_value, uint8_t input_channel) {
  switch (new_value) {
    case kWSDisconnected:
      this->set_server_disconnected();
      break;
    case kWSConnecting:
      this->set_server_disconnected();
      break;
    case kWSAuthorizing:
      this->set_server_disconnected();
      break;
    case kWSConnected:
      this->set_server_connected();
      break;
    default:
      this->set_server_disconnected();
      break;
  }
}

void LedBlinker::set_input(int new_value, uint8_t input_channel) {
  this->flip();
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
