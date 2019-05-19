#ifndef _passthrough_H_
#define _passthrough_H_

#include "transform.h"


// Passthrough is a "null" transform, just passing the value to output
template <typename T>
class Passthrough : public OneToOneTransform<T> {
 public:
  Passthrough() : Passthrough("") {}

  Passthrough(String sk_path, String id="", String schema="", uint8_t valueIdx = 0)
    : OneToOneTransform<T>{sk_path, id, schema, valueIdx} {

    }


  virtual void set_input(T newValue, uint8_t idx = 0) override final {
    ValueProducer<T>::output = newValue;
    this->notify();
  }


  String as_json() override final {
    DynamicJsonBuffer jsonBuffer;
    String json;
    JsonObject& root = jsonBuffer.createObject();
    root.set("path", TransformBase::sk_path);
    root.set("value", ValueProducer<T>::output);
    root.printTo(json);
    return json;
  }

  virtual JsonObject& get_configuration(JsonBuffer& buf) override final {
    JsonObject& root = buf.createObject();
    root["sk_path"] = TransformBase::sk_path;
    root["value"] = ValueProducer<T>::output;
    return root;
  }

  virtual bool set_configuration(const JsonObject& config) override final {
    if (!config.containsKey("sk_path")) {
      return false;
    }
    TransformBase::sk_path = config["sk_path"].as<String>();
    return true;
  }
  
};

#endif