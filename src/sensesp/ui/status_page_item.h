#ifndef SENSESP_UI_UI_OUTPUT_H
#define SENSESP_UI_UI_OUTPUT_H

#include <ArduinoJson.h>
#include <functional>
#include <map>

#include "Arduino.h"
#include "sensesp/system/observablevalue.h"
#include "sensesp/system/valueconsumer.h"
#include "sensesp/system/valueproducer.h"

namespace sensesp {

constexpr char kUIOutputDefaultGroup[] = "Default";
constexpr int kUIOutputDefaultOrder = 1000;

class StatusPageItemBase {
 public:
  StatusPageItemBase(String name, String group, int order)
      : name_(name), group_(group), order_(order) {}

  String& get_name() { return name_; }

  virtual JsonDocument as_json() = 0;

  static const std::map<String, StatusPageItemBase*>* get_status_page_items() {
    return &status_page_items_;
  }

 protected:
  String name_;
  String group_ = kUIOutputDefaultGroup;
  int order_ = kUIOutputDefaultOrder;
  static std::map<String, StatusPageItemBase*> status_page_items_;
};

template <typename T>
class StatusPageItem : public StatusPageItemBase, public ObservableValue<T> {
 public:
  StatusPageItem(String name, const T& value, String group, int order)
      : StatusPageItemBase(name, group, order), ObservableValue<T>(value) {
          status_page_items_[name] = this;
  }

  protected:
    virtual JsonDocument as_json() override{
    JsonDocument obj;
    obj["name"] = name_;
    obj["value"] = this->get();
    obj["group"] = group_;
    obj["order"] = order_;
    return obj;
  }
};

}  // namespace sensesp

#endif
