#ifndef _UI_OUTPUT_H
#define _UI_OUTPUT_H
#include <ArduinoJson.h>

#include <functional>
#include <map>

#include "Arduino.h"
#include "observablevalue.h"
#include "valueconsumer.h"
#include "valueproducer.h"
#define UI_OUTPUT_GROUP_GENERAL "General"
#define UI_OUTPUT_GROUP_NETWORK "Network"
#define UI_OUTPUT_GROUP_SIGNALK "Signal K"
#define UI_OUTPUT_ORDER_DEFAULT 100

namespace sensesp {

class UIOutputBase : virtual public Observable {
 protected:
  String name_;
  String group_ = UI_OUTPUT_GROUP_GENERAL;
  int order_ = UI_OUTPUT_ORDER_DEFAULT;

 public:
  UIOutputBase(String name, String group, int order);
  String& get_name() { return name_; }

  virtual void set_json(const JsonObject& obj) {}
};

template <typename T>
class UILambdaOutput : public UIOutputBase {
 private:
  std::function<T()> value_function_;

 public:
  UILambdaOutput(String name, std::function<T()> function, String group = UI_OUTPUT_GROUP_GENERAL, int order = UI_OUTPUT_ORDER_DEFAULT)
      : UIOutputBase(name, group, order) {
    value_function_ = function;
  }

  T get() { return value_function_(); }

  void set_json(const JsonObject& obj) override 
  {
    JsonObject outputJson = obj.createNestedObject(name_);
    outputJson["Value"] = get();
    outputJson["Group"] = group_;
    outputJson["Order"] = order_;
  }
};

extern std::map<String, UIOutputBase*> ui_outputs;

template <typename T>
class UIOutput : public UIOutputBase,
                 public ObservableValue<T>,
                 public ValueConsumer<T> {
 public:
  UIOutput(String name, String group = UI_OUTPUT_GROUP_GENERAL, int order = UI_OUTPUT_ORDER_DEFAULT) : UIOutputBase(name, group, order) {}

  UIOutput(String name, T value, String group = UI_OUTPUT_GROUP_GENERAL, int order = UI_OUTPUT_ORDER_DEFAULT) : UIOutputBase(name, group, order) {
    this->ObservableValue<T>::emit(value);
  }

  void set_json(const JsonObject& obj) override {
    JsonObject outputJson = obj.createNestedObject(name_);
    outputJson["Value"] = ObservableValue<T>::get();
    outputJson["Group"] = group_;
    outputJson["Order"] = order_;
  }

  void set_input(T new_value, uint8_t input_channel = 0) override {
    this->ValueProducer<T>::emit(new_value);
  }
};
}  // namespace sensesp

#endif