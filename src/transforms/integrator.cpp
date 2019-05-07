#include "integrator.h"

#include "sensesp_app.h"

// Integrator

Integrator::Integrator(String path, float k, float value, String id, String schema)
    : Transform{ path, id, schema },
      k{ k },
      output{ value } {
  load_configuration();
}

void Integrator::enable() {
  // save the integrator value every 10 s
  // NOTE: Disabled for now because interrupts start throwing
  // exceptions.
  //app.onRepeat(10000, [this](){ this->save_configuration(); });
}

void Integrator::set_input(float input) {
  output += input;
  notify();
}

String Integrator::as_json() {
  DynamicJsonBuffer jsonBuffer;
  String json;
  JsonObject& root = jsonBuffer.createObject();
  root.set("path", this->sk_path);
  root.set("value", output);
  root.printTo(json);
  return json;
}

JsonObject& Integrator::get_configuration(JsonBuffer& buf) {
  JsonObject& root = buf.createObject();
  root["k"] = k;
  root["sk_path"] = sk_path;
  root["value"] = output;
  return root;
}

bool Integrator::set_configuration(const JsonObject& config) {
  String expected[] = {"k", "value", "sk_path"};
  for (auto str : expected) {
    if (!config.containsKey(str)) {
      return false;
    }
  }
  k = config["k"];
  output = config["value"];
  sk_path = config["sk_path"].as<String>();
  return true;
}
