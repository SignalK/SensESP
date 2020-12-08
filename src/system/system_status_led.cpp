#include "system_status_led.h"

// These patterns indicate how many milliseconds the led should be on and off,
// repeating until PATTERN_END is reached

// FIXME: These patterns eat up memory even if the LED
// would be disabled

// PATTERN: *___________________
int no_ap_pattern[] = {50, 950, PATTERN_END};

// PATTERN: ******______________
int wifi_disconnected_pattern[] = {300, 700, PATTERN_END};

// PATTERN: **************______
int wifi_connected_pattern[] = {700, 300, PATTERN_END};

// PATTERN: *************_*_*_*_
int ws_connecting_pattern[] = {650, 50, 50, 50, 50, 50, 50, 50, PATTERN_END};

// PATTERN: ********************
int wifimanager_pattern[] = {1000, 0, PATTERN_END};

// PATTERN: ******************__
int ws_connected_pattern[] = {900, 100, PATTERN_END};

// PATTERN: *_*_*_*_____________
int ws_disconnected_pattern[] = {50, 50, 50, 50, 50, 50, 50, 650, PATTERN_END};

// PATTERN: ****____
int ws_authorizing_pattern[] = {200, 200, PATTERN_END};

SystemStatusLed::SystemStatusLed(int pin) {
  pinMode(pin, OUTPUT);
  blinker = new PatternBlinker(pin, no_ap_pattern);
}

void SystemStatusLed::set_wifi_no_ap() { blinker->set_pattern(no_ap_pattern); }
void SystemStatusLed::set_wifi_disconnected() {
  blinker->set_pattern(wifi_disconnected_pattern);
}
void SystemStatusLed::set_wifi_connected() {
  blinker->set_pattern(wifi_connected_pattern);
}
void SystemStatusLed::set_wifimanager_activated() {
  blinker->set_pattern(wifimanager_pattern);
}
void SystemStatusLed::set_ws_disconnected() {
  blinker->set_pattern(ws_disconnected_pattern);
}
void SystemStatusLed::set_ws_authorizing() {
  blinker->set_pattern(ws_authorizing_pattern);
}
void SystemStatusLed::set_ws_connecting() {
  blinker->set_pattern(ws_connecting_pattern);
}
void SystemStatusLed::set_ws_connected() {
  blinker->set_pattern(ws_connected_pattern);
}

void SystemStatusLed::set_input(WifiState new_value, uint8_t input_channel) {
  // FIXME: If pointers to member functions would be held in an array,
  // this would be a simple array dereferencing
  switch (new_value) {
    case kWifiNoAP:
      this->set_wifi_no_ap();
      break;
    case kWifiDisconnected:
      this->set_wifi_disconnected();
      break;
    case kWifiConnectedToAP:
      this->set_wifi_connected();
      break;
    case kExecutingWifiManager:
      this->set_wifimanager_activated();
      break;
    default:
      this->set_wifi_disconnected();
      break;
  }
}

void SystemStatusLed::set_input(WSConnectionState new_value,
                              uint8_t input_channel) {
  switch (new_value) {
    case kWSDisconnected:
      this->set_ws_disconnected();
      break;
    case kWSConnecting:
      this->set_ws_connecting();
      break;
    case kWSAuthorizing:
      this->set_ws_authorizing();
      break;
    case kWSConnected:
      this->set_ws_connected();
      break;
    default:
      this->set_ws_disconnected();
      break;
  }
}

void SystemStatusLed::set_input(int new_value, uint8_t input_channel) {
  blinker->blip();
}
