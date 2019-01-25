#ifndef _component_H_
#define _component_H_

#include "../observable.h"

#include "Arduino.h"
#include <ArduinoJson.h>

///////////////////
// Components transform raw device readouts into useful sensor values.

class Component : public Observable {
  protected:
    String path;
  public:
    Component(String path) : path(path) {}
    //virtual void set_configuration(JsonObject& config) = 0;
    virtual String as_json() = 0;
    //virtual JsonObject& get_configuration() = 0;
    //virtual String get_config_description() = 0;
};

template <class T>
class Passthrough : public Component {
  T output;
  public:
    Passthrough(String path) : Component(path) {}
    void set_input(T input) {
      output = input;
      notify();
    }
    //void set_configuration(JsonObject& config) override;
    String as_json() override {
      DynamicJsonBuffer jsonBuffer;
      String json;
      JsonObject& root = jsonBuffer.createObject();
      root.set("path", path);
      root.set("value", output);
      root.printTo(json);
      return json;
    }
    //JsonObject& get_configuration() override;
    //String get_config_description() override;
};

// y = k * x + c
class Linear : public Component {
  float k;
  float c;
  float output;
  public:
    Linear(String path, float k, float c);
    void set_input(float input);
    //void set_configuration(JsonObject& config) override;
    String as_json() override;
    //JsonObject& get_configuration() override;
    //String get_config_description() override;
};

#endif
