#include "signalk_delta.h"

#include "Arduino.h"
#include "ArduinoJson.h"
#include "sensesp.h"

#include "signalk/signalk_emitter.h"

SKDelta::SKDelta(const String& hostname, unsigned int max_buffer_size)
    : hostname{hostname}, max_buffer_size{max_buffer_size}, meta_sent_{false} {}

void SKDelta::append(const String val) {
  if (buffer.size() >= max_buffer_size) {
    buffer.pop_back();
  }
  buffer.push_front(val);
}

bool SKDelta::data_available() { return buffer.size() > 0; }

void SKDelta::get_delta(String& output) {
  DynamicJsonDocument jsonDoc(1024);

  if (!meta_sent_) {
    this->add_metadata(jsonDoc);
  }

  // JsonObject delta = jsonDoc.as<JsonObject>();
  JsonArray updates = jsonDoc.createNestedArray("updates");

  if (!meta_sent_) {
    this->add_metadata(updates);
  }

  JsonObject current = updates.createNestedObject();
  JsonObject source = current.createNestedObject("source");
  source["label"] = hostname;
  JsonArray values = current.createNestedArray("values");

  while (!buffer.empty()) {
    values.add(serialized(buffer.back()));
    buffer.pop_back();
  }

  serializeJson(jsonDoc, output);

  debugD("SKDelta::get_delta: %s", output.c_str());
}

void SKDelta::add_metadata(JsonArray updates) {

    JsonObject new_entry = updates.createNestedObject();
    JsonArray meta = new_entry.createNestedArray("meta");;
    for (auto const& sk_source : SKEmitter::get_sources()) {
       sk_source->add_metadata(meta);
    } // for
    meta_sent_ = true;
}
