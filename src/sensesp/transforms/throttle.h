#ifndef SENSESP_SRC_SENSESP_TRANSFORMS_THROTTLE_H_
#define SENSESP_SRC_SENSESP_TRANSFORMS_THROTTLE_H_

#include <climits>
#include <elapsedMillis.h>

#include "transform.h"

namespace sensesp {

/**
 * @brief Throttle the input rate.
 *
 * Make sure that a frequently changing value is only
 * reported at a specified minimum interval. If, for example,
 * the input value changes every 100ms, but you only want to process it
 * every 1000ms, you can use this transform. It does not average or buffer
 * any values, it just throws away values that are too frequent.
 *
 * In many cases, you might want to consider the `moving_average` transform
 * instead of this one. The `moving_average` transform will smooth out
 * the input values, while this one will only emit the last value.
 *
 * @param min_interval Minimum time, in ms, before a new value
 * is emitted again.
 *
 */
template <typename T>
class Throttle : public SymmetricTransform<T> {
 public:
  Throttle(long min_interval)
      : SymmetricTransform<T>(), min_interval_{min_interval} {}

  void set(T input) override {
    if (age_ < min_interval_) {
      return;
    }
    age_ = 0;
    this->emit(input);
  }

 protected:
  long min_interval_;
  elapsedMillis age_ = LONG_MAX;
};

}  // namespace sensesp

#endif  // SENSESP_SRC_SENSESP_TRANSFORMS_THROTTLE_H_
