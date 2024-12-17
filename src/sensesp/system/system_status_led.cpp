#include "system_status_led.h"

namespace sensesp {

// PATTERN: **__________________
LEDPattern no_ap_pattern = {frag_linear_fade(100, 100, CRGB::Red),
                            frag_linear_fade(950, 200, CRGB::Black)};

// PATTERN: ******______________
LEDPattern wifi_disconnected_pattern = {
    frag_linear_fade(300, 100, CRGB::Red),
    frag_linear_fade(700, 200, CRGB::Black),
};

// PATTERN: **************______
LEDPattern wifi_connected_pattern = {
    frag_linear_fade(700, 200, CRGB::Yellow),
    frag_linear_fade(300, 100, CRGB::Black),
};

// PATTERN: *************_*_*_*_
LEDPattern ws_connecting_pattern = {
    frag_linear_fade(650, 200, CRGB::Yellow),
    frag_linear_fade(50, 50, CRGB::Black),
    frag_linear_fade(50, 50, CRGB::Yellow),
    frag_linear_fade(50, 50, CRGB::Black),
    frag_linear_fade(50, 50, CRGB::Yellow),
    frag_linear_fade(50, 50, CRGB::Black),
    frag_linear_fade(50, 50, CRGB::Yellow),
    frag_linear_fade(50, 50, CRGB::Black),
};

// PATTERN: ******************__
LEDPattern ws_connected_pattern = {
    frag_linear_fade(900, 200, CRGB::Blue),
    frag_linear_fade(100, 100, CRGB::Black),
};

// PATTERN: *_*_*_*_____________
LEDPattern ws_disconnected_pattern = {
    frag_linear_fade(50, 50, CRGB::Yellow),
    frag_linear_fade(50, 50, CRGB::Black),
    frag_linear_fade(50, 50, CRGB::Yellow),
    frag_linear_fade(50, 50, CRGB::Black),
    frag_linear_fade(50, 50, CRGB::Yellow),
    frag_linear_fade(50, 50, CRGB::Black),
    frag_linear_fade(50, 50, CRGB::Yellow),
    frag_linear_fade(650, 200, CRGB::Black),
};

// PATTERN: ****____
LEDPattern ws_authorizing_pattern = {
    frag_linear_fade(200, 200, CRGB::Yellow),
    frag_linear_fade(200, 200, CRGB::Black),
};

LEDPattern blip_pattern = {
    frag_blend(50, CRGB::Black),
    frag_blend(50, CRGB::Black, true),
};

void BaseSystemStatusLed::set_wifi_no_ap() {
  std::vector<LEDPatternFragment> no_ap_frags;
  no_ap_frags.push_back(frag_linear_fade(100, 100, CRGB::Red));
  no_ap_frags.push_back(frag_linear_fade(950, 200, CRGB::Black));
  LEDPattern no_ap_pattern(no_ap_frags);

  blinker_->set_pattern(no_ap_pattern);
  ESP_LOGD("SystemStatusLed", "pattern set to no_ap");
}

void BaseSystemStatusLed::set_wifi_disconnected() {
  blinker_->set_pattern(wifi_disconnected_pattern);
}

void BaseSystemStatusLed::set_ws_disconnected() {
  blinker_->set_pattern(ws_disconnected_pattern);
}
void BaseSystemStatusLed::set_ws_authorizing() {
  blinker_->set_pattern(ws_authorizing_pattern);
}
void BaseSystemStatusLed::set_ws_connecting() {
  blinker_->set_pattern(ws_connecting_pattern);
}
void BaseSystemStatusLed::set_ws_connected() {
  blinker_->set_pattern(ws_connected_pattern);
}

ValueConsumer<int>& BaseSystemStatusLed::get_delta_tx_count_consumer() {
  static LambdaConsumer<int> delta_tx_count_consumer_{
      [this](int) { blinker_->add_modifier(blip_pattern); }};
  return delta_tx_count_consumer_;
}

}  // namespace sensesp
