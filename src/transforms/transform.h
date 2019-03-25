#ifndef _transform_H_
#define _transform_H_

#include <set>

#include "Arduino.h"
#include <ArduinoJson.h>

#include "system/configurable.h"
#include "system/observable.h"
#include "sensesp.h"

// TODO: Split into multiple files

///////////////////
// Transforms transform raw device readouts into useful sensor values.

class Transform : public Observable, public Configurable {
 public:
  Transform(String sk_path, String id="", String schema="");
  virtual String as_json() = 0;
  String& get_sk_path() {
    return sk_path;
  }
  void set_sk_path(const String& path) {
    sk_path = path;
  }
  virtual void enable() {}
  static const std::set<Transform*>& get_transforms() {
    return transforms;
  }
 protected:
  String sk_path;
 private:
  static std::set<Transform*> transforms;
};

// Passthrough is a "null" transform, just passing the value to output

template <class T>
class Passthrough : public Transform {
 public:
  Passthrough() : Transform{"", "", ""} {}
  Passthrough(String sk_path, String id="", String schema="")
    : Transform{sk_path, id, schema} {}
  T get() { return output; }
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
    return true;
  }
 private:
  T output;
};

// TODO: implement a difference transform

#endif
