#ifndef SENSESP_SRC_SENSESP_TRANSFORMS_REPEAT_H_
#define SENSESP_SRC_SENSESP_TRANSFORMS_REPEAT_H_

#include <elapsedMillis.h>

#include "sensesp/types/nullable.h"
#include "transform.h"

namespace sensesp {

/**
 * @brief Repeat the input at specific intervals.
 *
 * Ensures that values that do not change frequently are still
 * reported at a specified interval. If the value has not
 * changed in interval milliseconds, the current value is emmitted
 * again.
 *
 * The repetition only occurs if the value has changed.
 *
 * @param interval Maximum time, in ms, before the previous value
 * is emitted again.
 *
 * @param config_path Path to configure this transform in the Config UI.
 */
template <typename T>
class Repeat : public SymmetricTransform<T> {
 public:
  Repeat(long interval) : SymmetricTransform<T>(), interval_{interval} {}

  void set(T input) override {
    this->emit(input);
    if (repeat_event_ != nullptr) {
      // Delete the old repeat event
      repeat_event_->remove();
    }
    repeat_event_ = SensESPBaseApp::get_event_loop()->onRepeat(
        interval_, [this]() { this->notify(); });
  }

 protected:
  long interval_;
  reactesp::RepeatEvent* repeat_event_ = nullptr;
};

// For compatibility with the old RepeatReport class
template <typename T>
class RepeatReport : public Repeat<T> {};

/**
 * @brief Repeat transform that stops emitting if the value age exceeds
 * max_age.
 *
 * @tparam T
 */
template <typename T>
class RepeatStopping : public Repeat<T> {
 public:
  RepeatStopping(long interval, long max_age)
      : Repeat<T>(interval), max_age_{max_age} {
    age_ = max_age;

    if (this->repeat_event_ != nullptr) {
      // Delete the old repeat event
      this->repeat_event_->remove();
    }
    this->repeat_event_ = SensESPBaseApp::get_event_loop()->onRepeat(
        this->interval_, [this]() { this->repeat_function(); });
  }

  virtual void set(const T& input) override {
    this->emit(input);
    age_ = 0;
    if (this->repeat_event_ != nullptr) {
      // Delete the old repeat event
      this->repeat_event_->remove();
    }
    this->repeat_event_ = SensESPBaseApp::get_event_loop()->onRepeat(
        this->interval_, [this]() { this->repeat_function(); });
  }

 protected:
  elapsedMillis age_;
  long max_age_;

 protected:
  void repeat_function() {
    if (age_ < max_age_) {
      this->notify();
    } else {
      if (this->repeat_event_ != nullptr) {
        // Delete the old repeat event
        this->repeat_event_->remove();
        this->repeat_event_ = nullptr;
      }
    }
  };
};

/**
 * @brief Repeat transform that emits an expired value if the value age exceeds
 * max_age.
 *
 * @tparam T
 */
template <typename T>
class RepeatExpiring : public Repeat<Nullable<T>> {
 public:
  RepeatExpiring(long interval, long max_age)
      : Repeat<T>(interval), max_age_{max_age} {
    age_ = max_age;

    if (this->repeat_event_ != nullptr) {
      // Delete the old repeat event
      this->repeat_event_->remove();
    }
    this->repeat_event_ = SensESPBaseApp::get_event_loop()->onRepeat(
        this->interval_, [this]() { this->repeat_function(); });
  }

  virtual void set(const T& input) override {
    this->emit(input);
    age_ = 0;
    if (this->repeat_event_ != nullptr) {
      // Delete the old repeat event
      this->repeat_event_->remove();
    }
    this->repeat_event_ = SensESPBaseApp::get_event_loop()->onRepeat(
        this->interval_, [this]() { this->repeat_function(); });
  }

 protected:
  elapsedMillis age_;
  long max_age_;

 protected:
  void repeat_function() {
    // Note: This could've been implemented with the new ExpiringValue class,
    // but this is more in line with the other Repeat classes.
    if (age_ < max_age_) {
      this->notify();
    } else {
      this->emit(Nullable::invalid());
    }
  };
};

/**
 * @brief Repeat transform that emits the last value at a constant interval.
 *
 * The last value is emitted at a constant interval, regardless of whether the
 * value has changed or not. If the value age exceeds max_age, expired_value is
 * emitted.
 *
 * This is particularly useful for outputs that expect a value at a constant
 * rate such as NMEA 2000.
 *
 */
template <typename T>
class RepeatConstantRate : public RepeatExpiring<T> {
 public:
  RepeatConstantRate(long interval, long max_age)
      : RepeatExpiring<T>(interval, max_age) {
    if (this->repeat_event_ != nullptr) {
      // Delete the old repeat event
      this->repeat_event_->remove();
    }

    this->repeat_event_ = SensESPBaseApp::get_event_loop()->onRepeat(
        interval, [this]() { this->repeat_function(); });
  }

  void set(T input) override {
    this->output = input;
    this->age_ = 0;
  }
};

}  // namespace sensesp

#endif  // SENSESP_SRC_SENSESP_TRANSFORMS_REPEAT_H_
