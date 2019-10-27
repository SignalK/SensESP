#include "signalk_delta.h"

#include "Arduino.h"
#include "ArduinoJson.h"
#include "sensesp.h"

SKDelta::SKDelta(const String& hostname, unsigned int max_buffer_size)
: hostname{hostname},
  max_buffer_size{max_buffer_size} {}

void SKDelta::append(const String val) {
  if (buffer.size() >= max_buffer_size) {
    buffer.pop_back();
  }
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

  debugD("SKDelta::get_delta: %s", output.c_str());
}

