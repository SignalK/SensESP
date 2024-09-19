#ifndef SENSESP_SRC_SENSESP_TYPES_NULLABLE_H_
#define SENSESP_SRC_SENSESP_TYPES_NULLABLE_H_

#include "ArduinoJson.h"

namespace sensesp {

/**
 * @brief Template class that supports a special invalid magic value for a type.
 *
 * The invalid value is always provided by the type's default constructor.
 *
 */
template <typename T>
class Nullable {
  public:
    Nullable() : value_{} {}
    Nullable(T value) : value_{value} {}
    Nullable<T>& operator=(T& value) {
      value_ = value;
      return *this;
    }
    Nullable<T>& operator=(const Nullable<T>& other) {
      value_ = other.value_;
      return *this;
    }
    operator T() const {
      return value_;
    }

    bool is_valid() const {
      return value_ != invalid_value_;
    }

    T* ptr() {
      return &value_;
    }

    static T invalid() {
      return invalid_value_;
    }

    T value() const {
      return value_;
    }

  private:
    T value_;
    static T invalid_value_;
};

typedef Nullable<int> NullableInt;
typedef Nullable<float> NullableFloat;
typedef Nullable<double> NullableDouble;

template <typename T>
void convertFromJson(JsonVariantConst src, Nullable<T> &dst) {
  if (src.isNull()) {
    dst = NullableInt::invalid();
  } else {
    dst = src.as<T>();
  }
}

template <typename T>
void convertToJson(const Nullable<T> &src, JsonVariant dst) {
  if (src.is_valid()) {
    dst.set(src.value());
  } else {
    dst.clear();
  }
}

}  // namespace sensesp

#endif  // SENSESP_SRC_SENSESP_TYPES_NULLABLE_H_
