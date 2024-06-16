#ifndef _observablevalue_H
#define _observablevalue_H

#include "configurable.h"
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
 * @brief A value container that notifies its observers if it gets changed.
 */
template <class T>
class ObservableValue : public ValueConsumer<T>, public ValueProducer<T> {
 public:
  ObservableValue() : ValueConsumer<T>(), ValueProducer<T>() {}

  ObservableValue(const T& value)
      : ValueConsumer<T>(), ValueProducer<T>(value) {}

  void set(T value) override { this->ValueProducer<T>::emit(value); }

  const T& operator=(const T& value) {
    set(value);
    return value;
  }

 protected:
  template <class U>
  friend bool operator==(ObservableValue<U> const& lhs, U const& rhs);
  template <class U>
  friend bool operator!=(ObservableValue<U> const& lhs, U const& rhs);
};

/**
 * @brief An ObservableValue that saves its value to the configuration.
 *
 * Only use this class for values that change infrequently. Frequent writes
 * to the configuration can wear out the flash memory.
 *
 * @tparam T
 */
template <class T>
class PersistingObservableValue : public ObservableValue<T>,
                                  public Configurable {
 public:
  PersistingObservableValue() : Configurable() {}

  PersistingObservableValue(const T& value, String config_path = "")
      : ObservableValue<T>(value), Configurable(config_path) {
    load_configuration();
  }

  virtual void set(T value) override {
    ObservableValue<T>::set(value);
    this->save_configuration();
  }

 protected:
  virtual void get_configuration(JsonObject& doc) override {
    doc["value"] = this->output;
  }

  virtual bool set_configuration(const JsonObject& config) override {
    if (!config.containsKey("value")) {
      return false;
    }
    ObservableValue<T>::set(config["value"]);
    return true;
  }
};

}  // namespace sensesp

#endif
