#ifndef SENSESP_SYSTEM_OBSERVABLEVALUE_H
#define SENSESP_SYSTEM_OBSERVABLEVALUE_H

#include "saveable.h"
#include "observable.h"
#include "sensesp/ui/config_item.h"
#include "valueproducer.h"

namespace sensesp {

// forward declaration for the operator overloading functions
template <class T>
class ObservableValue;

template <class T>
bool operator==(ObservableValue<T> const& lhs, T const& rhs) {
  return lhs.output_ == rhs;
}

template <class T>
bool operator!=(ObservableValue<T> const& lhs, T const& rhs) {
  return lhs.output_ != rhs;
}

/**
 * @brief A value container that notifies its observers if it gets changed.
 */
template <class T>
class ObservableValue : public ValueConsumer<T>, public ValueProducer<T> {
 public:
  ObservableValue() = default;

  ObservableValue(const T& value)
      : ValueConsumer<T>(), ValueProducer<T>(value) {}

  // Delete copy constructor and assignment operator
  ObservableValue(const ObservableValue&) = delete;
  ObservableValue& operator=(const ObservableValue&) = delete;

  void set(const T& value) override { this->ValueProducer<T>::emit(value); }

  const T& operator=(const T& value) {
    set(value);
    return value;
  }

  T& operator++() {
    set(this->output_ + 1);
    return this->output_;
  }

  T& operator--() {
    set(this->output_ - 1);
    return this->output_;
  }

  T operator++(int) {
    T old = this->output_;
    set(this->output_ + 1);
    return old;
  }

  T operator--(int) {
    T old = this->output_;
    set(this->output_ - 1);
    return old;
  }

  const T& operator+=(const T& value) {
    set(this->output_ + value);
    return this->output_;
  }

  const T& operator-=(const T& value) {
    set(this->output_ - value);
    return this->output_;
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
                                  public FileSystemSaveable {
 public:
  PersistingObservableValue(const T& value, String config_path = "")
      : ObservableValue<T>(value), FileSystemSaveable(config_path) {
    load();

    // Emit the current state as soon as the event loop starts
    event_loop()->onDelay(0, [this]() { this->emit(this->output_); });
  }

  virtual void set(const T& value) override {
    ObservableValue<T>::set(value);
    this->save();
  }

  virtual bool to_json(JsonObject& doc) override {
    doc["value"] = this->output_;
    return true;
  }

  virtual bool from_json(const JsonObject& config) override {
    if (!config["value"].is<T>()) {
      return false;
    }
    this->output_ = config["value"].as<T>();
    return true;
  }

 protected:
};

template <class T>
const String ConfigSchema(const PersistingObservableValue<T>& obj) {
  String schema = R"({"type":"object","properties":{"value":{"title":"Value","type":"{{type_string}}"}}})";
  const T value = obj.get();
  schema.replace("{{type_string}}", get_schema_type_string(value));
  return schema;
}

}  // namespace sensesp

#endif
