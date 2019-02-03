#ifndef _computation_H_
#define _computation_H_

#include "Arduino.h"
#include <ArduinoJson.h>

#include "system/configurable.h"
#include "system/observable.h"
#include "sensesp.h"

///////////////////
// Computations transform raw device readouts into useful sensor values.

class Computation : public Observable, public Configurable {
 public:
  Computation(String sk_path, String id="", String schema="")
    : Configurable{id, schema}, sk_path{sk_path} {}
  virtual String as_json() = 0;
  void set_sk_path(const String& path) {
    sk_path = path;
  }
 protected:
  String sk_path;
};

template <class T>
class Passthrough : public Computation {
 public:
  Passthrough() : Computation{"", "", ""} {}
  Passthrough(String sk_path, String id="", String schema="")
    : Computation{sk_path, id, schema} {}
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
 private:
  T output;
};

// y = k * x + c
class Linear : public Computation {
 public:
  Linear(String sk_path, float k, float c, String id="", String schema="");
  void set_input(float input);
  String as_json() override final;
  virtual JsonObject& get_configuration(JsonBuffer& buf) override final;
  virtual void set_configuration(const JsonObject& config) override final;
 private:
  float k;
  float c;
  float output;
};

template <class T>
class Frequency : public Computation {
 public:
  Frequency(String sk_path, String id="", String schema="")
      : Computation{sk_path, id, schema} {
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

#endif
