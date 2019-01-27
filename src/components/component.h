#ifndef _component_H_
#define _component_H_

#include "Arduino.h"
#include <ArduinoJson.h>

#include "system/configurable.h"
#include "system/observable.h"

///////////////////
// Components transform raw device readouts into useful sensor values.

class Component : public Observable, public Configurable {
 public:
  Component(String sk_path, String id="", String schema="")
    : Configurable{id, schema}, sk_path{sk_path} {}
  virtual String as_json() = 0;
 protected:
  String sk_path;
};

template <class T>
class Passthrough : public Component {
 public:
  Passthrough(String sk_path, String id="", String schema="")
    : Component{sk_path, id, schema} {}
  void set_input(T input) {
    output = input;
    notify();
  }
  String as_json() override {
    DynamicJsonBuffer jsonBuffer;
    String json;
    JsonObject& root = jsonBuffer.createObject();
    root.set("path", sk_path);
    root.set("value", output);
    root.printTo(json);
    return json;
  }
 private:
  T output;
};

// y = k * x + c
class Linear : public Component {
  float k;
  float c;
  float output;
  public:
    Linear(String sk_path, float k, float c, String id="", String schema="");
    void set_input(float input);
    //void set_configuration(JsonObject& config) override;
    String as_json() override;
    //JsonObject& get_configuration() override;
    //String get_config_description() override;
};

#endif
