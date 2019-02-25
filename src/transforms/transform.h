#ifndef _transform_H_
#define _transform_H_

#include "Arduino.h"
#include <ArduinoJson.h>

#include "system/configurable.h"
#include "system/observable.h"
#include "sensesp.h"

// TODO: Split into multiple files

///////////////////
// Transforms transform raw device readouts into useful sensor values.

// TODO: transforms should register themselves

class Transform : public Observable, public Configurable {
 public:
  Transform(String sk_path, String id="", String schema="")
    : Configurable{id, schema}, sk_path{sk_path} {}
  virtual String as_json() = 0;
  void set_sk_path(const String& path) {
    sk_path = path;
  }
 protected:
  String sk_path;
};

template <class T>
class Passthrough : public Transform {
 public:
  Passthrough() : Transform{"", "", ""} {}
  Passthrough(String sk_path, String id="", String schema="")
    : Transform{sk_path, id, schema} {}
  void set_input(T input) {
    output = input;
    notify();
  }
  String as_json() override final {
    DynamicJsonBuffer jsonBuffer;
    String json;
    JsonObject& root = jsonBuffer.createObject();
    root.set("path", sk_path);
    root.set("value", output);
    root.printTo(json);
    return json;
  }
  virtual JsonObject& get_configuration(JsonBuffer& buf) override final {
    JsonObject& root = buf.createObject();
    root["sk_path"] = sk_path;
    root["value"] = output;
    return root;
  }
  virtual bool set_configuration(const JsonObject& config) override final {
    if (!config.containsKey("sk_path")) {
      return false;
    }
    sk_path = config["sk_path"].as<String>();
  }
 private:
  T output;
};

// y = k * x + c
class Linear : public Transform {
 public:
  Linear(String sk_path, float k, float c, String id="", String schema="");
  void set_input(float input);
  String as_json() override final;
  virtual JsonObject& get_configuration(JsonBuffer& buf) override final;
  virtual bool set_configuration(const JsonObject& config) override final;
 private:
  float k;
  float c;
  float output;
};

// TODO: frequency should have a scaling factor (for flow rate meters etc)

template <class T>
class Frequency : public Transform {
 public:
  Frequency(String sk_path, String id="", String schema="")
      : Transform{sk_path, id, schema} {
    last_update = millis();
    app.onRepeat(1000, std::bind(&Frequency::repeat_cb, this));
  }
  void set_input(T input) {
    ++ticks;
    notify();
  }
  String as_json() override final {
    DynamicJsonBuffer jsonBuffer;
    String json;
    JsonObject& root = jsonBuffer.createObject();
    root.set("path", this->sk_path);
    root.set("value", output);
    root.printTo(json);
    return json;
  }
 private:
  int ticks = 0;
  int last_update = 0;
  void repeat_cb() {
    unsigned long cur_millis = millis();
    unsigned long elapsed_millis = cur_millis - last_update;
    output = ticks / (elapsed_millis/1000.);
    last_update = cur_millis;
    ticks = 0;
    notify();
  }
  float output;
};

// TODO: implement a difference transform

#endif
