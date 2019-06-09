
#include "gnss_position.h"

String GNSSPosition::as_signalK() {
    DynamicJsonBuffer jsonBuffer;
    String json;
    JsonObject& root = jsonBuffer.createObject();
    root.set("path", sk_path);
    JsonObject& value = root.createNestedObject("value");
    value.set("latitude", output.latitude);
    value.set("longitude", output.longitude);
    if (output.altitude > -10000) {
      value.set("altitude", output.altitude);
    }
    root.printTo(json);
    return json;
}

JsonObject& GNSSPosition::get_configuration(JsonBuffer& buf) {
  JsonObject& root = buf.createObject();
  root["sk_path"] = sk_path;
  return root;
}

String GNSSPosition::get_config_schema() {
   return R"({
      "type": "object",
      "properties": {
          "sk_path": { "title": "SignalK Path", "type": "string" }
      }
   })";
}



bool GNSSPosition::set_configuration(const JsonObject& config) {
  if (!config.containsKey("sk_path")) {
    return false;
  }
  sk_path = config["sk_path"].as<String>();
  return true;
}
