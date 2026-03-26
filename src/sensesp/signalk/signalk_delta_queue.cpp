#include "sensesp.h"

#include "signalk_delta_queue.h"

#include "Arduino.h"
#include "ArduinoJson.h"
#include "sensesp_app.h"
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

void SKDeltaQueue::get_deltas(std::vector<String>& output) {
  // Drain the buffer under the semaphore
  std::list<String> items;
  take_semaphore();
  items.swap(buffer);
  release_semaphore();

  // Fast path: if no item contains a context key, skip the grouping logic.
  // This avoids deserialize/reserialize overhead for the common case.
  bool has_contextual = false;
  for (const auto& item : items) {
    if (item.indexOf("\"context\"") >= 0) {
      has_contextual = true;
      break;
    }
  }

  if (!has_contextual) {
    // All items are self-context: build a single delta (original behavior)
    JsonDocument json_doc;
    JsonArray updates = json_doc["updates"].to<JsonArray>();

    if (!meta_sent_) {
      this->add_metadata(updates);
    }

    if (!items.empty()) {
      JsonObject current = updates.add<JsonObject>();
      JsonObject source = current["source"].to<JsonObject>();
      source["label"] = SensESPBaseApp::get_hostname();
      JsonArray values = current["values"].to<JsonArray>();

      // Reverse iteration: buffer is push_front/pop_back (LIFO),
      // so rbegin gives oldest-first ordering.
      for (auto it = items.rbegin(); it != items.rend(); ++it) {
        values.add(serialized(*it));
      }
    }

    String delta;
    serializeJson(json_doc, delta);
    output.push_back(std::move(delta));
    ESP_LOGD(__FILENAME__, "delta: %s", output.back().c_str());
    return;
  }

  // Slow path: separate items by context
  std::list<String> self_items;
  std::map<String, std::list<String>> contextual_items;

  // Reverse iteration: buffer is push_front/pop_back (LIFO),
  // so rbegin gives oldest-first ordering.
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
      contextual_items[context].push_back(value_json);
    } else {
      self_items.push_back(*it);
    }
  }

  // Default (self) delta
  if (!self_items.empty() || !meta_sent_) {
    JsonDocument json_doc;
    JsonArray updates = json_doc["updates"].to<JsonArray>();

    if (!meta_sent_) {
      this->add_metadata(updates);
    }

    if (!self_items.empty()) {
      JsonObject current = updates.add<JsonObject>();
      JsonObject source = current["source"].to<JsonObject>();
      source["label"] = SensESPBaseApp::get_hostname();
      JsonArray values = current["values"].to<JsonArray>();

      for (const auto& item : self_items) {
        values.add(serialized(item));
      }
    }

    String delta;
    serializeJson(json_doc, delta);
    ESP_LOGD(__FILENAME__, "delta: %s", delta.c_str());
    output.push_back(std::move(delta));
  }

  // Contextual deltas — one per context
  for (const auto& [context, ctx_items] : contextual_items) {
    JsonDocument json_doc;
    json_doc["context"] = context;
    JsonArray updates = json_doc["updates"].to<JsonArray>();
    JsonObject current = updates.add<JsonObject>();
    JsonObject source = current["source"].to<JsonObject>();
    source["label"] = SensESPBaseApp::get_hostname();
    JsonArray values = current["values"].to<JsonArray>();

    for (const auto& item : ctx_items) {
      values.add(serialized(item));
    }

    String delta;
    serializeJson(json_doc, delta);
    ESP_LOGD(__FILENAME__, "delta: %s", delta.c_str());
    output.push_back(std::move(delta));
  }
}

void SKDeltaQueue::add_metadata(JsonArray updates) {
  JsonObject new_entry = updates.add<JsonObject>();
  JsonArray meta = new_entry["meta"].to<JsonArray>();
  for (auto const& sk_source : SKEmitter::get_sources()) {
    sk_source->add_metadata(meta);
  }
  meta_sent_ = true;
}

}  // namespace sensesp
