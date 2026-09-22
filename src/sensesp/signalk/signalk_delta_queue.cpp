#include "sensesp.h"

#include "signalk_delta_queue.h"

#include "Arduino.h"
#include "ArduinoJson.h"
#include "sensesp_app.h"
#include "signalk_delta_packing.h"
#include "signalk_emitter.h"

namespace sensesp {

SKDeltaQueue::SKDeltaQueue(unsigned int max_buffer_size)
    : max_buffer_size{max_buffer_size}, meta_sent_{false} {
  semaphore_ = xSemaphoreCreateRecursiveMutexStatic(&semaphore_buffer_);

  event_loop()->onDelay(
      0, [this]() { this->connect_emitters(); });
}

bool SKDeltaQueue::take_semaphore(uint64_t timeout_ms) {
  if (timeout_ms == 0) {
    return xSemaphoreTakeRecursive(semaphore_, portMAX_DELAY) == pdTRUE;
  } else {
    return xSemaphoreTakeRecursive(semaphore_, timeout_ms) == pdTRUE;
  }
}

void SKDeltaQueue::release_semaphore() { xSemaphoreGiveRecursive(semaphore_); }

void SKDeltaQueue::append(const String& val) {
  take_semaphore();
  if (get_buffer_size() >= max_buffer_size) {
    buffer.pop_back();
  }
  buffer.push_front(val);
  release_semaphore();
}

void SKDeltaQueue::connect_emitters() {
  for (auto const& sk_source : SKEmitter::get_sources()) {
    if (sk_source->get_sk_path() != "") {
      sk_source->attach([sk_source, this]() {
        String output;
        JsonDocument doc;
        sk_source->as_signalk_json(doc);
        serializeJson(doc, output);
        this->append(output);
      });
    }
  }
}

bool SKDeltaQueue::data_available() {
  take_semaphore();
  bool available = buffer.size() > 0;
  release_semaphore();
  return available;
}

void SKDeltaQueue::get_delta(String& output) {
  std::vector<String> deltas;
  get_deltas(deltas);
  output = "";
  for (size_t i = 0; i < deltas.size(); ++i) {
    if (i > 0) {
      output += "\n";
    }
    output += deltas[i];
  }
}

namespace {

// The serialized length of a delta whose values array is empty. The queued
// items are copied into that array verbatim, so the finished delta is this
// length plus every item plus one comma between consecutive items -- known
// without serializing anything.
size_t values_envelope_length(const String& context, const String& label) {
  JsonDocument doc;
  if (context.length() > 0) {
    doc["context"] = context;
  }
  JsonArray updates = doc["updates"].to<JsonArray>();
  JsonObject current = updates.add<JsonObject>();
  JsonObject source = current["source"].to<JsonObject>();
  source["label"] = label;
  current["values"].to<JsonArray>();
  return measureJson(doc);
}

String build_values_delta(const String& context, const String& label,
                          const std::vector<const String*>& items, size_t begin,
                          size_t end) {
  JsonDocument doc;
  if (context.length() > 0) {
    doc["context"] = context;
  }
  JsonArray updates = doc["updates"].to<JsonArray>();
  JsonObject current = updates.add<JsonObject>();
  JsonObject source = current["source"].to<JsonObject>();
  source["label"] = label;
  JsonArray values = current["values"].to<JsonArray>();
  for (size_t i = begin; i < end; i++) {
    values.add(serialized(*items[i]));
  }
  String delta;
  serializeJson(doc, delta);
  return delta;
}

size_t meta_envelope_length() {
  JsonDocument doc;
  JsonArray updates = doc["updates"].to<JsonArray>();
  JsonObject current = updates.add<JsonObject>();
  current["meta"].to<JsonArray>();
  return measureJson(doc);
}

String build_meta_delta(const std::vector<String>& entries, size_t begin,
                        size_t end) {
  JsonDocument doc;
  JsonArray updates = doc["updates"].to<JsonArray>();
  JsonObject current = updates.add<JsonObject>();
  JsonArray meta = current["meta"].to<JsonArray>();
  for (size_t i = begin; i < end; i++) {
    meta.add(serialized(entries[i]));
  }
  String delta;
  serializeJson(doc, delta);
  return delta;
}

}  // namespace

void SKDeltaQueue::emit_deltas(const String& context,
                               const std::vector<const String*>& items,
                               size_t max_delta_size,
                               std::vector<String>& output) {
  if (items.empty()) {
    return;
  }
  const String label = SensESPBaseApp::get_hostname();
  SKDeltaPacker packer(values_envelope_length(context, label), max_delta_size);
  size_t chunk_begin = 0;
  for (size_t i = 0; i < items.size(); i++) {
    const size_t item_length = items[i]->length();
    // An item longer than the budget fits nowhere. It goes out alone and
    // send_delta() decides what to do with it; never loop on it here.
    if (!packer.is_empty() && !packer.fits(item_length)) {
      output.push_back(build_values_delta(context, label, items, chunk_begin, i));
      ESP_LOGV(__FILENAME__, "delta: %s", output.back().c_str());
      packer.reset();
      chunk_begin = i;
    }
    packer.add(item_length);
  }
  output.push_back(
      build_values_delta(context, label, items, chunk_begin, items.size()));
  ESP_LOGV(__FILENAME__, "delta: %s", output.back().c_str());
}

void SKDeltaQueue::emit_metadata_deltas(size_t max_delta_size,
                                        std::vector<String>& output) {
  // One serialized entry per emitter, so metadata splits the same way values
  // do. Before this, all of it went into the first delta, which is the delta
  // most likely to be over budget -- and it was marked sent either way, so a
  // device in that state ran without metadata until the next reconnect.
  std::vector<String> entries;
  for (auto const& sk_source : SKEmitter::get_sources()) {
    JsonDocument doc;
    JsonArray meta = doc.to<JsonArray>();
    sk_source->add_metadata(meta);
    for (JsonVariantConst entry : meta) {
      String serialized_entry;
      serializeJson(entry, serialized_entry);
      entries.push_back(serialized_entry);
    }
  }
  meta_sent_ = true;
  if (entries.empty()) {
    return;
  }

  SKDeltaPacker packer(meta_envelope_length(), max_delta_size);
  size_t chunk_begin = 0;
  for (size_t i = 0; i < entries.size(); i++) {
    const size_t entry_length = entries[i].length();
    if (!packer.is_empty() && !packer.fits(entry_length)) {
      output.push_back(build_meta_delta(entries, chunk_begin, i));
      packer.reset();
      chunk_begin = i;
    }
    packer.add(entry_length);
  }
  output.push_back(build_meta_delta(entries, chunk_begin, entries.size()));
}

void SKDeltaQueue::get_deltas(std::vector<String>& output,
                              size_t max_delta_size) {
  // Drain the buffer under the semaphore
  std::list<String> items;
  take_semaphore();
  items.swap(buffer);
  release_semaphore();

  if (!meta_sent_) {
    emit_metadata_deltas(max_delta_size, output);
  }

  // Fast path: if no item contains a context key, skip the grouping logic.
  // This avoids deserialize/reserialize overhead for the common case.
  bool has_contextual = false;
  for (const auto& item : items) {
    if (item.indexOf("\"context\"") >= 0) {
      has_contextual = true;
      break;
    }
  }

  // Reverse iteration: buffer is push_front/pop_back (LIFO),
  // so rbegin gives oldest-first ordering.
  if (!has_contextual) {
    std::vector<const String*> self_items;
    for (auto it = items.rbegin(); it != items.rend(); ++it) {
      self_items.push_back(&(*it));
    }
    emit_deltas(String(""), self_items, max_delta_size, output);
    return;
  }

  // Slow path: separate items by context
  std::list<String> self_storage;
  std::map<String, std::list<String>> contextual_storage;

  for (auto it = items.rbegin(); it != items.rend(); ++it) {
    JsonDocument item_doc;
    DeserializationError err = deserializeJson(item_doc, *it);
    if (err) {
      ESP_LOGE(__FILENAME__, "Failed to parse buffered item: %s", err.c_str());
      continue;
    }
    if (item_doc["context"].is<const char*>()) {
      String context = item_doc["context"].as<String>();
      // Re-serialize without the context key for the values array
      item_doc.remove("context");
      String value_json;
      serializeJson(item_doc, value_json);
      contextual_storage[context].push_back(value_json);
    } else {
      self_storage.push_back(*it);
    }
  }

  std::vector<const String*> self_items;
  for (const auto& item : self_storage) {
    self_items.push_back(&item);
  }
  emit_deltas(String(""), self_items, max_delta_size, output);

  for (const auto& [context, ctx_items] : contextual_storage) {
    std::vector<const String*> pointers;
    for (const auto& item : ctx_items) {
      pointers.push_back(&item);
    }
    emit_deltas(context, pointers, max_delta_size, output);
  }
}


}  // namespace sensesp
