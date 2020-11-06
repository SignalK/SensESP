
#include "led_blinker.h"

#include "net/networking.h"
#include "sensesp.h"
#include "sensesp_app.h"

#define max(a, b) ((a) > (b) ? (a) : (b))

BaseBlinker::BaseBlinker(int pin) : pin{pin} {}

/**
 * Turn the LED on or off.
 */
void BaseBlinker::set_state(bool state) {
  this->state = state;
  digitalWrite(pin, state);
  update_counter++;
}

/**
 * Invert the current LED state.
 */
void BaseBlinker::flip_state() { this->set_state(!this->state); }

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

  bool orig_state = this->state;
  this->set_state(false);
  int current_counter = this->update_counter;
  app.onDelay(duration, [this, duration, orig_state, current_counter]() {
    // only update if no-one has touched the LED in the meanwhile
    if (this->update_counter == current_counter) {
      this->set_state(true);
      int new_counter = this->update_counter;
      app.onDelay(duration, [this, orig_state, new_counter]() {
        // again, only update if no-one has touched the LED
        if (this->update_counter == new_counter) {
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
  bool was_enabled = this->enabled;
  this->enabled = state;
  if (this->enabled) {
    this->tick();
  } else {
    this->set_state(false);
    if (was_enabled) {
      reaction->remove();
    }
  }
}

void BaseBlinker::enable() { this->tick(); }

PeriodicBlinker::PeriodicBlinker(int pin, unsigned int period)
    : BaseBlinker(pin), period{period} {}

EvenBlinker::EvenBlinker(int pin, unsigned int period)
    : PeriodicBlinker(pin, period) {}

void EvenBlinker::tick() {
  if (!enabled) {
    return;
  }
  this->flip_state();
  reaction = app.onDelay(period, [this]() { this->tick(); });
}

RatioBlinker::RatioBlinker(int pin, unsigned int period, float ratio)
    : PeriodicBlinker(pin, period), ratio{ratio} {}

void RatioBlinker::tick() {
  if (!enabled) {
    return;
  }
  this->flip_state();
  int on_duration = ratio * period;
  int off_duration = max(0, period - on_duration);
  unsigned int ref_duration = state == false ? off_duration : on_duration;
  reaction = app.onDelay(ref_duration, [this]() { this->tick(); });
}

PatternBlinker::PatternBlinker(int pin, int pattern[])
    : BaseBlinker(pin), pattern{pattern} {}

/**
 * Set a new blink pattern. Patterns are arrays of ints, with
 * PATTERN_END as the last value. Each number in the pattern indicates
 * the length of that segment, in milliseconds. The first number indicates
 * an ON duration, the second an OFF duration, and so on.
 */
void PatternBlinker::set_pattern(int pattern[]) {
  this->pattern = pattern;
  this->restart();
}

void PatternBlinker::tick() {
  if (!enabled) {
    return;
  }
  // When pattern[pattern_ptr] == PATTERN_END, that's the end of the pattern,
  // so start over at the beginning.
  if (pattern[pattern_ptr] == PATTERN_END) {
    pattern_ptr = 0;
  }
  // odd indices indicate times when LED should be OFF, even when ON
  bool new_state = (pattern_ptr % 2) == 0;
  this->set_state(new_state);
  reaction = app.onDelay(pattern[pattern_ptr++], [this]() { this->tick(); });
}

void PatternBlinker::restart() {
  state = false;
  pattern_ptr = 0;
  if (reaction != NULL) {
    reaction->remove();
    reaction = NULL;
    this->tick();
  }
}
