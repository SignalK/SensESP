#include "signalk_delta_queue.h"

#include "Arduino.h"
#include "ArduinoJson.h"
#include "sensesp.h"
#include "signalk/signalk_emitter.h"

SKDeltaQueue::SKDeltaQueue(const String& hostname, unsigned int max_buffer_size)
    : Startable{0}, hostname{hostname}, max_buffer_size{max_buffer_size}, meta_sent_{false} {}

void SKDeltaQueue::start() {
  this->connect_emitters();
}

void SKDeltaQueue::append(const String val) {
  if (buffer.size() >= max_buffer_size) {
    buffer.pop_back();
  }
  buffer.push_front(val);
}

void SKDeltaQueue::connect_emitters() {
  for (auto const& sk_source : SKEmitter::get_sources()) {
    if (sk_source->get_sk_path() != "") {
      sk_source->attach([sk_source, this]() {
        this->append(sk_source->as_signalk());
      });
    }
  }
}

bool SKDeltaQueue::data_available() { return buffer.size() > 0; }

unsigned int SKDeltaQueue::get_doc_size_estimate() {
  int buf_size = buffer.size();
  int estimate =
      2 * JSON_OBJECT_SIZE(1) +         // source and one update
      JSON_ARRAY_SIZE(1) +              // one update
      JSON_ARRAY_SIZE(buf_size) +       // buf_size values in the update
      buf_size * JSON_OBJECT_SIZE(2) +  // two key-value pairs in each object
      200;  // some extra headroom to hide embarrassing bugs

  for (auto item : buffer) {
    // also reserve space for the pre-rendered strings
    estimate += item.length() + 1;
  }
  return estimate;  
}

unsigned int SKDeltaQueue::get_metadata_size_estimate() {
  int num_metadata = SKEmitter::get_sources().size();
  int estimate = JSON_ARRAY_SIZE(num_metadata);

  int num_fields;
  auto update_estimate = [&](String& field) {
    if (!field.isEmpty()) {
      num_fields++;
      estimate += field.length() + 1;
    }
  };

  for (auto const& source : SKEmitter::get_sources()) {
    num_fields = 0;
    auto metadata = source->get_metadata();
    if (metadata == NULL) {
      continue;
    }
    update_estimate(metadata->units_);
    update_estimate(metadata->display_name_);
    update_estimate(metadata->description_);
    update_estimate(metadata->short_name_);
    
    estimate += JSON_OBJECT_SIZE(num_fields);
  }
  return estimate;
}

void SKDeltaQueue::get_delta(String& output) {
  // estimate the size of the serialized json string

  unsigned int doc_size_estimate = get_doc_size_estimate();

  if (!meta_sent_) {
    doc_size_estimate += JSON_OBJECT_SIZE(1) + get_metadata_size_estimate();
  }

  DynamicJsonDocument jsonDoc(doc_size_estimate);

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

  debugD("delta: %s", output.c_str());
}

void SKDeltaQueue::add_metadata(JsonArray updates) {
  JsonObject new_entry = updates.createNestedObject();
  JsonArray meta = new_entry.createNestedArray("meta");
  for (auto const& sk_source : SKEmitter::get_sources()) {
    sk_source->add_metadata(meta);
  }
  meta_sent_ = true;
}
