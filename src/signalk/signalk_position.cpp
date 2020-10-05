
#include "signalk_position.h"

template <>
String SKOutput<Position>::as_signalk() {
  DynamicJsonDocument jsonDoc(1024);
  String json;
  JsonObject root = jsonDoc.as<JsonObject>();
  root["path"] = this->get_sk_path();
  JsonObject value = root.createNestedObject("value");
  value["latitude"] = output.latitude;
  value["longitude"] = output.longitude;
  if (output.altitude > -10000) {
    value["altitude"] = output.altitude;
  }
  serializeJson(root, json);
  return json;
}
