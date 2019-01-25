#include "signal_k.h"

#include "Arduino.h"
#include "ArduinoJson.h"

SKDelta::SKDelta(String hostname) {
  this->hostname = hostname;
}

void SKDelta::append(const String val) {
  buffer.push_front(val);
}

bool SKDelta::data_available() {
  return buffer.size() > 0;
}

void SKDelta::get_delta(String& output) {
  DynamicJsonBuffer jsonBuffer;

  JsonObject& delta = jsonBuffer.createObject();
  JsonArray& updates = delta.createNestedArray("updates");

  JsonObject& current = updates.createNestedObject();
  JsonObject& source = current.createNestedObject("source");
  source["label"] = hostname;
  JsonArray& values = current.createNestedArray("values");

  while (!buffer.empty()) {
    values.add(RawJson(buffer.back()));
    buffer.pop_back();
  }

  delta.printTo(output);
}

