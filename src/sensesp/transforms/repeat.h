#ifndef SENSESP_SRC_SENSESP_TRANSFORMS_REPEAT_H_
#define SENSESP_SRC_SENSESP_TRANSFORMS_REPEAT_H_

#include <elapsedMillis.h>

#include "sensesp/types/nullable.h"
#include "sensesp_base_app.h"
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
template <typename FROM, typename TO>
class Repeat : public Transform<FROM, TO> {
 public:
  Repeat(unsigned long interval) : Transform<FROM, TO>(), interval_{interval} {
    if (interval_ == 0) {
      ESP_LOGW("Repeat", "Interval is 0. This will cause a busy loop.");
    }
  }

  virtual void set(const FROM& input) override {
    this->emit(input);
    if (repeat_event_ != nullptr) {
      // Delete the old repeat event
      repeat_event_->remove(event_loop());
    }
    repeat_event_ =
        event_loop()->onRepeat(interval_, [this]() { this->notify(); });
  }

 protected:
  unsigned long interval_;
  reactesp::RepeatEvent* repeat_event_ = nullptr;
};

// For compatibility with the old RepeatReport class
template <typename T>
class RepeatReport : public Repeat<T, T> {};

/**
 * @brief Repeat transform that stops emitting if the value age exceeds
 * max_age.
 *
 * @tparam T
 */
template <typename T>
class RepeatStopping : public Repeat<T, T> {
 public:
  RepeatStopping(unsigned long interval, unsigned long max_age)
      : Repeat<T, T>(interval), max_age_{max_age} {
    age_ = max_age;

    if (this->repeat_event_ != nullptr) {
      // Delete the old repeat event
      this->repeat_event_->remove();
    }
    this->repeat_event_ = event_loop()->onRepeat(
        this->interval_, [this]() { this->repeat_function(); });
  }

  virtual void set(const T& input) override {
    this->emit(input);
    age_ = 0;
    if (this->repeat_event_ != nullptr) {
      // Delete the old repeat event
      this->repeat_event_->remove();
    }
    this->repeat_event_ = event_loop()->onRepeat(
        this->interval_, [this]() { this->repeat_function(); });
  }

 protected:
  elapsedMillis age_;
  unsigned long max_age_;

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
class RepeatExpiring : public Repeat<T, Nullable<T>> {
 public:
  RepeatExpiring(unsigned long interval, unsigned long max_age)
      : Repeat<T, Nullable<T>>(interval), max_age_{max_age} {
    ESP_LOGD("RepeatExpiring", "interval: %lu, max_age: %lu", interval,
             max_age);

    age_ = max_age;

    if (this->repeat_event_ != nullptr) {
      // Delete the old repeat event
      this->repeat_event_->remove(event_loop());
    }
    this->repeat_event_ = event_loop()->onRepeat(
        this->interval_, [this]() { this->repeat_function(); });
  }

  virtual void set(const T& input) override {
    this->emit(input);
    age_ = 0;
    if (this->repeat_event_ != nullptr) {
      // Delete the old repeat event
      this->repeat_event_->remove(event_loop());
    }
    this->repeat_event_ = event_loop()->onRepeat(
        this->interval_, [this]() { this->repeat_function(); });
  }

 protected:
  elapsedMillis age_;
  unsigned long max_age_;

 protected:
  void repeat_function() {
    // Note: This could've been implemented with the new ExpiringValue class,
    // but this is more in line with the other Repeat classes.
    if (age_ < max_age_) {
      this->notify();
    } else {
      this->emit(this->get().invalid());
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
  RepeatConstantRate(unsigned long interval, unsigned long max_age)
      : RepeatExpiring<T>(interval, max_age) {
    if (this->repeat_event_ != nullptr) {
      // Delete the old repeat event
      this->repeat_event_->remove();
    }

    this->repeat_event_ =
        event_loop()->onRepeat(interval, [this]() { this->repeat_function(); });
  }

  void set(T input) override {
    this->output_ = input;
    this->age_ = 0;
  }
};

}  // namespace sensesp

#endif  // SENSESP_SRC_SENSESP_TRANSFORMS_REPEAT_H_
