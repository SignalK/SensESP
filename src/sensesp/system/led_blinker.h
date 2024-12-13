#ifndef SENSESP_SYSTEM_LED_BLINKER_H_
#define SENSESP_SYSTEM_LED_BLINKER_H_

#include <Arduino.h>
#include <FastLED.h>
#include <ReactESP.h>

#include "sensesp/signalk/signalk_ws_client.h"

namespace sensesp {

#define max(a, b) ((a) > (b) ? (a) : (b))
#define PATTERN_END (-1)

/**
 * @brief A base class for LED blinker classes.
 */
class BaseBlinker {
 public:
  BaseBlinker(int pin) : pin_{pin} {
    pinMode(pin, OUTPUT);
    event_loop()->onDelay(1, [this]() { this->tick(); });
  }
  /**
   * Turn the LED on or off.
   */
  void set_state(bool state) {
    this->state_ = state;
    digitalWrite(pin_, state);
    update_counter_++;
  }

  /**
   * Invert the current LED state.
   */
  void flip_state() { this->set_state(!this->state_); }

  /**
   * Flip the LED off and on for `duration` milliseconds.
   */
  void blip(int duration = 20) {
    // indicator for a blip being in progress
    static bool blipping = false;

    // only allow one blip at a time
    if (blipping) {
      return;
    }
    blipping = true;

    bool const orig_state = this->state_;
    this->set_state(false);
    int const current_counter = this->update_counter_;
    event_loop()->onDelay(
        duration, [this, duration, orig_state, current_counter]() {
          // only update if no-one has touched the LED in the meanwhile
          if (this->update_counter_ == current_counter) {
            this->set_state(true);
            int const new_counter = this->update_counter_;
            event_loop()->onDelay(duration, [this, orig_state, new_counter]() {
              // again, only update if no-one has touched the LED
              if (this->update_counter_ == new_counter) {
                this->set_state(orig_state);
              }
              blipping = false;
            });
          } else {
            blipping = false;
          }
        });
  }

  /**
   * Enable or disable the blinker.
   */
  void set_enabled(bool state) {
    bool const was_enabled = this->enabled_;
    this->enabled_ = state;
    if (this->enabled_) {
      this->tick();
    } else {
      this->set_state(false);
      if (was_enabled) {
        event_->remove(event_loop());
      }
    }
  }
  /**
   * Tick is called whenever the blinker is enabled or when it's time to
   * change the LED state.
   */
  virtual void tick() = 0;

 protected:
  int pin_;
  bool enabled_ = true;
  bool state_ = false;
  int update_counter_ = 0;
  reactesp::Event* event_ = NULL;
};

class LEDPattern;
using FragmentCallback = std::function<void(uint32_t, CRGB&)>;

class LEDPatternFragment {
 public:
  LEDPatternFragment(uint32_t duration_ms, FragmentCallback callback)
      : duration_ms_(duration_ms), callback_(callback) {}

  // Copy constructor
  LEDPatternFragment(const LEDPatternFragment& other)
      : duration_ms_(other.duration_ms_), callback_(other.callback_) {}

  // Assignment operator
  LEDPatternFragment& operator=(const LEDPatternFragment& other) {
    duration_ms_ = other.duration_ms_;
    callback_ = other.callback_;
    return *this;
  }

  uint32_t duration_ms_;
  FragmentCallback callback_;
};

class LEDPattern {
 public:
  LEDPattern() {}
  LEDPattern(const std::vector<LEDPatternFragment>& fragments)
      : fragments_(fragments) {}
  LEDPattern(const std::initializer_list<LEDPatternFragment>& fragments)
      : fragments_(fragments), current_fragment_idx_(0), fragment_begin_ms_(0) {}

  // Assignment operator
  LEDPattern& operator=(const LEDPattern& other) {
    fragments_ = other.fragments_;
    return *this;
  }

  bool apply(CRGB& crgb, bool oneshot = false) {
    // Initialize fragment begin time if it's the first time
    if (fragment_begin_ms_ == 0) {
      fragment_begin_ms_ = millis();
    }
    unsigned long current_fragment_duration_ms =
        fragments_[current_fragment_idx_].duration_ms_;
    while (millis() - fragment_begin_ms_ >= current_fragment_duration_ms) {
      current_fragment_idx_++;
      if (current_fragment_idx_ >= fragments_.size()) {
        current_fragment_idx_ = 0;
        if (oneshot) {
          return false;
        }
      }
      fragment_begin_ms_ += current_fragment_duration_ms;
      current_fragment_duration_ms =
          fragments_[current_fragment_idx_].duration_ms_;
    }
    // Call the callback function for the current fragment, passing the elapsed
    // time and a reference to the CRGB object
    fragments_[current_fragment_idx_].callback_(millis() - fragment_begin_ms_,
                                                crgb);
    return true;
  }

 protected:
  std::vector<LEDPatternFragment> fragments_;
  uint32_t current_fragment_idx_ = 0;
  unsigned long fragment_begin_ms_ = 0;
};

/**
 * @brief Blink the LED with a pattern specified by a vector of callback
 * functions.
 *
 * LED blinker class that blinks the LED with a pattern specified by a vector of
 * callback functions. Each callback function is called in sequence, with the
 * duration of each callback function specified in the LEDPatternFragment
 * object.
 *
 * In addition to the pattern, there can be one or more transient modifiers that
 * modify the LED color during the pattern. These modifiers are applied in
 * sequence on top of the pattern. They can be used e.g. to change the color,
 * dim the LED, or apply a brief fade effect to indicate e.g. a button press or
 * received data.
 *
 */
class LEDBlinker {
 public:
  LEDBlinker(CRGB& crgb, LEDPattern pattern, std::function<void()> show_func)
      : crgb_(crgb), pattern_(pattern), show_func_(show_func) {}
  void tick() {
    // Always start with the last color
    crgb_ = last_color_;
    pattern_.apply(crgb_);
    last_color_ = crgb_;
    std::list<LEDPattern>::iterator it = modifiers_.begin();
    while (it != modifiers_.end()) {
      LEDPattern* mod = &*it;
      bool result = mod->apply(crgb_, true);
      if (!result) {
        it = modifiers_.erase(it);
      } else {
        it++;
      }
    }
    show_func_();
  }

  void set_pattern(const LEDPattern& pattern) { pattern_ = pattern; }

  void add_modifier(const LEDPattern& modifier) {
    modifiers_.push_back(modifier);
  }

 protected:
  CRGB& crgb_;
  CRGB last_color_ = CRGB::Black;  // Previous set color before modifiers
  LEDPattern pattern_;
  std::list<LEDPattern> modifiers_ = {};

  std::function<void()> show_func_;
};

LEDPatternFragment frag_solid_color(uint32_t duration_ms, const CRGB& color);
LEDPatternFragment frag_linear_fade(uint32_t duration_ms,
                                    uint32_t fade_duration_ms,
                                    const CRGB& target_color);
LEDPatternFragment frag_linear_invert(uint32_t duration_ms,
                                      bool reverse = false);
LEDPatternFragment frag_blend(uint32_t duration_ms, const CRGB& target_color,
                              bool reverse = false);
LEDPatternFragment frag_nop(uint32_t duration_ms);
}  // namespace sensesp

#endif
