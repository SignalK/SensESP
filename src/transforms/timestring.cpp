#include "timestring.h"


TimeString::TimeString(String path, String id, String schema) :
    ValueConsumer<time_t>(),
    StringTransform{ path, id, schema } {
  load_configuration();
}

void TimeString::set_input(time_t input, uint8_t inputChannel) {
  char buf[sizeof "2011-10-08T07:07:09Z"];
  strftime(buf, sizeof buf, "%FT%TZ", gmtime(&input));
  output = String(buf);
  notify();
}

String TimeString::as_json() {
  DynamicJsonBuffer jsonBuffer;
  String json;
  JsonObject& root = jsonBuffer.createObject();
  root.set("path", this->sk_path);
  root.set("value", output);
  root.printTo(json);
  return json;
}

JsonObject& TimeString::get_configuration(JsonBuffer& buf) {
  JsonObject& root = buf.createObject();
  root["sk_path"] = sk_path;
  root["value"] = output;
  return root;
}

bool TimeString::set_configuration(const JsonObject& config) {
  String expected[] = {"sk_path"};
  for (auto str : expected) {
    if (!config.containsKey(str)) {
      return false;
    }
  }
  sk_path = config["sk_path"].as<String>();
  return true;
}

