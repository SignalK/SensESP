#include "component.h"

#include "ArduinoJson.h"


// Linear

Linear::Linear(String path, float k, float c)
  : Component{ path },
    k{ k },
    c{ c } {}

void Linear::set_input(float input) {
  output = k * input + c;
  notify();
}

// void Linear::set_configuration(JsonObject& config) {
//   Serial.println("Linear::set_configuration");
// }

String Linear::as_json() {
  DynamicJsonBuffer jsonBuffer;
  String json;
  JsonObject& root = jsonBuffer.createObject();
  root.set("path", this->path);
  root.set("value", output);
  root.printTo(json);
  return json;
}

// JsonObject& Linear::get_configuration() {
//   Serial.println("Linear::as_json");
// }

// String Linear::get_config_description() {
//   return "{}";
// }
