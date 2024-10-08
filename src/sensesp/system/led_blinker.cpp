
#include "sensesp.h"

#include "led_blinker.h"

#include "sensesp_app.h"

namespace sensesp {

#define max(a, b) ((a) > (b) ? (a) : (b))

BaseBlinker::BaseBlinker(int pin) : pin_{pin} {
  pinMode(pin, OUTPUT);
  event_loop()->onDelay(1, [this]() { this->tick(); });
}

/**
 * Turn the LED on or off.
 */
void BaseBlinker::set_state(bool state) {
  this->state_ = state;
  digitalWrite(pin_, state);
  update_counter_++;
}

/**
 * Invert the current LED state.
 */
void BaseBlinker::flip_state() { this->set_state(!this->state_); }

/**
 * Flip the LED off and on for `duration` milliseconds.
 */
void BaseBlinker::blip(int duration) {
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
          event_loop()->onDelay(
              duration, [this, orig_state, new_counter]() {
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
void BaseBlinker::set_enabled(bool state) {
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

PeriodicBlinker::PeriodicBlinker(int pin, unsigned int period)
    : BaseBlinker(pin), period_{period} {}

EvenBlinker::EvenBlinker(int pin, unsigned int period)
    : PeriodicBlinker(pin, period) {}

void EvenBlinker::tick() {
  if (!enabled_) {
    return;
  }
  this->flip_state();
  event_ = event_loop()->onDelay(
      period_, [this]() { this->tick(); });
}

RatioBlinker::RatioBlinker(int pin, unsigned int period, float ratio)
    : PeriodicBlinker(pin, period), ratio_{ratio} {}

void RatioBlinker::tick() {
  if (!enabled_) {
    return;
  }
  this->flip_state();
  int const on_duration = ratio_ * period_;
  int const off_duration = max(0, period_ - on_duration);
  unsigned int const ref_duration =
      state_ == false ? off_duration : on_duration;
  event_ = event_loop()->onDelay(
      ref_duration, [this]() { this->tick(); });
}

PatternBlinker::PatternBlinker(int pin, int pattern[])
    : BaseBlinker(pin), pattern_{pattern} {}

/**
 * Set a new blink pattern. Patterns are arrays of ints, with
 * PATTERN_END as the last value. Each number in the pattern indicates
 * the length of that segment, in milliseconds. The first number indicates
 * an ON duration, the second an OFF duration, and so on.
 */
void PatternBlinker::set_pattern(int pattern[]) {
  this->pattern_ = pattern;
  this->restart();
}

void PatternBlinker::tick() {
  if (!enabled_) {
    return;
  }
  // When pattern[pattern_ptr] == PATTERN_END, that's the end of the pattern,
  // so start over at the beginning.
  if (pattern_[pattern_ptr_] == PATTERN_END) {
    pattern_ptr_ = 0;
  }
  // odd indices indicate times when LED should be OFF, even when ON
  bool const new_state = (pattern_ptr_ % 2) == 0;
  this->set_state(new_state);
  event_ = event_loop()->onDelay(
      pattern_[pattern_ptr_++], [this]() { this->tick(); });
}

void PatternBlinker::restart() {
  state_ = false;
  pattern_ptr_ = 0;
  if (event_ != NULL) {
    event_->remove(event_loop());
    event_ = NULL;
    this->tick();
  }
}

}  // namespace sensesp
