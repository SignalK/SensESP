#include "component.h"

#include "ArduinoJson.h"

// Linear

Linear::Linear(String path, float k, float c, String id, String schema)
  : Component{ path, id, schema },
    k{ k },
    c{ c } {}

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
