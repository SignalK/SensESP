#include "sensesp.h"

#include "signalk_delta_queue.h"

#include "Arduino.h"
#include "ArduinoJson.h"
#include "sensesp_app.h"
#include "signalk_emitter.h"

namespace sensesp {

SKDeltaQueue::SKDeltaQueue(unsigned int max_buffer_size)
    : max_buffer_size{max_buffer_size}, meta_sent_{false} {
  semaphore_ = xSemaphoreCreateRecursiveMutex();

  reactesp::EventLoop::app->onDelay(0, [this]() { this->connect_emitters(); });
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
      sk_source->attach(
          [sk_source, this]() { this->append(sk_source->as_signalk()); });
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
  // estimate the size of the serialized json string

  JsonDocument json_doc;

  // JsonObject delta = jsonDoc.as<JsonObject>();
  JsonArray updates = json_doc["updates"].to<JsonArray>();

  if (!meta_sent_) {
    this->add_metadata(updates);
  }

  JsonObject current = updates.add<JsonObject>();
  JsonObject source = current["source"].to<JsonObject>();
  source["label"] = SensESPBaseApp::get_hostname();
  JsonArray values = current["values"].to<JsonArray>();

  take_semaphore();
  while (!buffer.empty()) {
    values.add(serialized(buffer.back()));
    buffer.pop_back();
  }
  release_semaphore();

  serializeJson(json_doc, output);

  ESP_LOGD(__FILENAME__, "delta: %s", output.c_str());
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
