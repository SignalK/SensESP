#include "system_status_led.h"

namespace sensesp {

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
  blinker_ = new PatternBlinker(pin, no_ap_pattern);
}

void SystemStatusLed::set_wifi_no_ap() { blinker_->set_pattern(no_ap_pattern); }
void SystemStatusLed::set_wifi_disconnected() {
  blinker_->set_pattern(wifi_disconnected_pattern);
}
void SystemStatusLed::set_wifimanager_activated() {
  blinker_->set_pattern(wifimanager_pattern);
}
void SystemStatusLed::set_ws_disconnected() {
  blinker_->set_pattern(ws_disconnected_pattern);
}
void SystemStatusLed::set_ws_authorizing() {
  blinker_->set_pattern(ws_authorizing_pattern);
}
void SystemStatusLed::set_ws_connecting() {
  blinker_->set_pattern(ws_connecting_pattern);
}
void SystemStatusLed::set_ws_connected() {
  blinker_->set_pattern(ws_connected_pattern);
}

void SystemStatusLed::set(SystemStatus new_value) {
  switch (new_value) {
    case SystemStatus::kWifiNoAP:
      this->set_wifi_no_ap();
      break;
    case SystemStatus::kWifiDisconnected:
      this->set_wifi_disconnected();
      break;
    case SystemStatus::kWifiManagerActivated:
      this->set_wifimanager_activated();
      break;
    case SystemStatus::kSKWSDisconnected:
      this->set_ws_disconnected();
      break;
    case SystemStatus::kSKWSConnecting:
      this->set_ws_connecting();
      break;
    case SystemStatus::kSKWSAuthorizing:
      this->set_ws_authorizing();
      break;
    case SystemStatus::kSKWSConnected:
      this->set_ws_connected();
      break;
  }
}

void SystemStatusLed::set(int new_value) { blinker_->blip(); }

}  // namespace sensesp
