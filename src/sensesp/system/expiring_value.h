#ifndef SENSESP_SRC_SENSESP_SYSTEM_EXPIRING_VALUE_H_
#define SENSESP_SRC_SENSESP_SYSTEM_EXPIRING_VALUE_H_

namespace sensesp {

/**
 * @brief Value container that keeps track of its expiration time.
 *
 * The value is considered expired if the time since the last update is greater
 * than the expiration duration. When expired, the value is replaced with an
 * expiration placeholder value.
 *
 * @tparam T
 */
template <typename T>
class ExpiringValue {
 public:
  ExpiringValue()
      : value_{},
        expiration_duration_{1000},
        last_update_{0},
        expired_value_{T{}} {}

  ExpiringValue(T value, unsigned long expiration_duration, T expired_value)
      : value_{value},
        expiration_duration_{expiration_duration},
        expired_value_{expired_value},
        last_update_{millis()} {}

  void update(T value) {
    value_ = value;
    last_update_ = millis();
  }

  T get() const {
    if (!is_expired()) {
      return value_;
    } else {
      return expired_value_;
    }
  }

  bool is_expired() const {
    return millis() - last_update_ > expiration_duration_;
  }

 private:
  T value_;
  T expired_value_;
  unsigned long expiration_duration_;
  unsigned long last_update_;
};

}  // namespace sensesp

#endif  // SENSESP_SRC_SENSESP_SYSTEM_EXPIRING_VALUE_H_
