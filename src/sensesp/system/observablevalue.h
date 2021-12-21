#ifndef _observablevalue_H
#define _observablevalue_H

#include "observable.h"
#include "valueproducer.h"

namespace sensesp {

// forward declaration for the operator overloading functions
template <class T>
class ObservableValue;

template <class T>
bool operator==(ObservableValue<T> const& lhs, T const& rhs) {
  return lhs.output == rhs;
}

template <class T>
bool operator!=(ObservableValue<T> const& lhs, T const& rhs) {
  return lhs.output != rhs;
}

/**
 * @brief A value that notifies its observers if it gets changed.
 */
template <class T>
class ObservableValue : public ValueProducer<T> {
 public:
  ObservableValue() {}

  ObservableValue(const T& value) { ValueProducer<T>::output = value; }

  void set(const T& value) { this->ValueProducer<T>::emit(value); }

  const T& operator=(const T& value) {
    set(value);
    return value;
  }

  template <class U>
  friend bool operator==(ObservableValue<U> const& lhs, U const& rhs);
  template <class U>
  friend bool operator!=(ObservableValue<U> const& lhs, U const& rhs);
};

}  // namespace sensesp

#endif
