#ifndef _SYSTEM_PROPERTY_H_
#define _SYSTEM_PROPERTY_H_
#include <ArduinoJson.h>

#include <functional>
#include <map>

#include "Arduino.h"
#include "observablevalue.h"
#include "valueconsumer.h"
#include "valueproducer.h"

namespace sensesp {

class UIOutputBase : virtual public Observable {
 protected:
  String name_;

 public:
  UIOutputBase(String name);
  String& get_name() { return name_; }

  virtual void set_json(const JsonObject& obj) {}
};

template <typename T>
class UILambdaOutput : public UIOutputBase {
 private:
  std::function<T()> value_function_;

 public:
  UILambdaOutput(String name, std::function<T()> function)
      : UIOutputBase(name) {
    value_function_ = function;
  }

  T get() { return value_function_(); }

  void set_json(const JsonObject& obj) override { obj[name_] = get(); }
};

extern std::map<String, UIOutputBase*> ui_outputs;

template <typename T>
class UIOutput : public UIOutputBase,
                 public ObservableValue<T>,
                 public ValueConsumer<T> {
 public:
  UIOutput(String name) : UIOutputBase(name) {}

  UIOutput(String name, T value) : UIOutputBase(name) {
    this->ObservableValue<T>::emit(value);
  }

  void set_json(const JsonObject& obj) override {
    obj[name_] = this->ObservableValue<T>::get();
  }

  void set_input(T new_value, uint8_t input_channel = 0) override {
    this->ValueProducer<T>::emit(new_value);
  }
};
}  // namespace sensesp

#endif