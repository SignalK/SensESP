#include "signalk_put_request.h"

#include "sensesp/system/uuid.h"
#include "sensesp_app.h"

namespace sensesp {

std::map<String, SKRequest::PendingRequest*> SKRequest::request_map_;

String SKRequest::send_request(JsonDocument& request,
                               std::function<void(JsonDocument&)> callback,
                               uint32_t timeout) {
  // Create a new PendingRequest object to track this request...
  auto* pending_request = new PendingRequest();

  // Generate a uuid for the request...
  pending_request->request_id = generate_uuid4();

  // Save the callback for future processing...
  pending_request->callback = callback;

  // After 10 seconds, if we haven't already handled a response,
  // assume its not coming.
  pending_request->timeout_cleanup =
      event_loop()->onDelay(timeout, [pending_request]() {
        // Mark the delay eventll as it will be cleaned up by the ReactESP
        // framework if this executes...
        ESP_LOGW(__FILENAME__, "No response from server for request Id %s",
                 pending_request->request_id.c_str());
        pending_request->timeout_cleanup = nullptr;
        SKRequest::remove_request(pending_request->request_id);
      });

  request_map_[pending_request->request_id] = pending_request;

  // Now, send the actual request to the server...
  request["requestId"] = pending_request->request_id;

  String request_txt;
  serializeJson(request, request_txt);
  ESP_LOGD(__FILENAME__, "Sending websocket request to server: %s",
           request_txt.c_str());

  SensESPApp::get()->get_ws_client()->sendTXT(request_txt);

  return pending_request->request_id;
}

SKRequest::PendingRequest* SKRequest::get_request(String request_id) {
  auto iterator = request_map_.find(request_id);
  if (iterator != request_map_.end()) {
    return (iterator->second);
  }
  return nullptr;
}

void SKRequest::handle_response(JsonDocument& response) {
  String request_id = response["requestId"];
  PendingRequest* pending_request = get_request(request_id);
  if (pending_request != nullptr) {
    pending_request->callback(response);

    // Now, are we done?
    String state = response["state"];
    if (!state.equalsIgnoreCase("PENDING")) {
      remove_request(request_id);
    }
  } else {
    ESP_LOGW(__FILENAME__,
             "Received request response for an untracked request: %s",
             request_id.c_str());
  }
}

void SKRequest::remove_request(String request_id) {
  PendingRequest* pending_request = SKRequest::get_request(request_id);
  if (pending_request != nullptr) {
    // First, stop any pending timeout handlers...
    if (pending_request->timeout_cleanup != nullptr) {
      // The timeout code was not called, so just
      // remove it from the ReactESP execution queue...
      pending_request->timeout_cleanup->remove(
          event_loop());
    }

    // Now, remove the request from the map...
    request_map_.erase(request_id);

    // Finally, discard the request tracker...
    delete pending_request;
  }
}

SKPutRequestBase::SKPutRequestBase(const String& sk_path,
                                   const String& config_path, uint32_t timeout)
    : FileSystemSaveable(config_path), sk_path{sk_path}, timeout{timeout} {
  load();
}

void SKPutRequestBase::send_put_request() {
  JsonDocument doc;
  JsonObject root = doc.to<JsonObject>();
  JsonObject put_data = root["put"].to<JsonObject>();
  put_data["path"] = sk_path;
  set_put_value(put_data);
  this->pending_request_id_ = SKRequest::send_request(
      doc, [this](JsonDocument& response) { this->on_response(response); },
      timeout);
}

bool SKPutRequestBase::request_pending() {
  return (get_request(this->pending_request_id_) != nullptr);
}

void SKPutRequestBase::on_response(JsonDocument& response) {
  String request_id = response["requestId"];
  String state = response["state"];
  ESP_LOGD(__FILENAME__, "Response %s received for PUT request: %s",
           state.c_str(), request_id.c_str());
}

bool SKPutRequestBase::to_json(JsonObject& root) {
  root["sk_path"] = sk_path;
  return true;
}

bool SKPutRequestBase::from_json(const JsonObject& config) {
  const String expected[] = {"sk_path"};
  for (auto str : expected) {
    if (!config[str].is<JsonVariant>()) {
      return false;
    }
  }
  this->sk_path = config["sk_path"].as<String>();
  return true;
}

}  // namespace sensesp
