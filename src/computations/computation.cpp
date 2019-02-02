#include "computation.h"

#include "ArduinoJson.h"

#include "sensesp.h"

// Linear

Linear::Linear(String path, float k, float c, String id, String schema)
    : Computation{ path, id, schema },
      k{ k },
      c{ c } {
  load_configuration();
}

void Linear::set_input(float input) {
  output = k * input + c;
  notify();
}

String Linear::as_json() {
  DynamicJsonBuffer jsonBuffer;
  String json;
  JsonObject& root = jsonBuffer.createObject();
  root.set("path", this->sk_path);
  root.set("value", output);
  root.printTo(json);
  return json;
}

JsonObject& Linear::get_configuration(JsonBuffer& buf) {
  JsonObject& root = buf.createObject();
  root["k"] = k;
  root["c"] = c;
  root["sk_path"] = sk_path;
  root["value"] = output;
  return root;
}

void Linear::set_configuration(const JsonObject& config) {
  k = config["k"];
  c = config["c"];
  sk_path = config["sk_path"].as<String>();
}

Frequency::Frequency(String sk_path, String id, String schema)
    : Computation{sk_path, id, schema} {
  last_update = millis();
  app.onRepeat(1000, std::bind(&Frequency::repeat_cb, this));
}

void Frequency::tick() {
  ++ticks;
}

void Frequency::repeat_cb() {
  unsigned long cur_millis = millis();
  unsigned long elapsed_millis = cur_millis - last_update;
  output = ticks / (elapsed_millis/1000.);
  last_update = cur_millis;
  ticks = 0;
  notify();
}

String Frequency::as_json() {
  DynamicJsonBuffer jsonBuffer;
  String json;
  JsonObject& root = jsonBuffer.createObject();
  root.set("path", this->sk_path);
  root.set("value", output);
  root.printTo(json);
  return json;
}
