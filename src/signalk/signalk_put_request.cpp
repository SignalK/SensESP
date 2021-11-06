#include "signalk/signalk_put_request.h"

#include "sensesp_app.h"
#include "system/uuid.h"

namespace sensesp {

extern ReactESP app;

std::map<String, SKRequest::PendingRequest*> SKRequest::request_map;

String SKRequest::send_request(
    DynamicJsonDocument& request,
    std::function<void(DynamicJsonDocument&)> callback, uint32_t timeout) {
  // Create a new PendingRequest object to track this request...
  PendingRequest* pending_request = new PendingRequest();

  // Generate a uuid for the request...
  pending_request->request_id = generate_uuid4();

  // Save the callback for future processing...
  pending_request->callback = callback;

  // After 10 seconds, if we haven't already handled a response,
  // assume its not coming.
  pending_request->timeout_cleanup = app.onDelay(timeout, [pending_request]() {
    // Mark the delay reaction null as it will be cleaned up by the ReactESP
    // framework if this executes...
    debugW("No response from server for request Id %s",
           pending_request->request_id.c_str());
    pending_request->timeout_cleanup = nullptr;
    SKRequest::remove_request(pending_request->request_id);
  });

  request_map[pending_request->request_id] = pending_request;

  // Now, send the actual request to the server...
  request["requestId"] = pending_request->request_id;

  String request_txt;
  serializeJson(request, request_txt);
  debugD("Sending websocket request to server: %s", request_txt.c_str());

  SensESPApp::get()->get_ws_client()->sendTXT(request_txt);

  return pending_request->request_id;
}

SKRequest::PendingRequest* SKRequest::get_request(String request_id) {
  auto it = request_map.find(request_id);
  if (it != request_map.end()) {
    return (it->second);
  } else {
    return nullptr;
  }
}

void SKRequest::handle_response(DynamicJsonDocument& response) {
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
    debugW("Received request response for an untracked request: %s",
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
      pending_request->timeout_cleanup->remove();
    }

    // Now, remove the request from the map...
    request_map.erase(request_id);

    // Finally, discard the request tracker...
    delete pending_request;
  }
}

SKPutRequestBase::SKPutRequestBase(String sk_path, String config_path,
                                   uint32_t timeout)
    : Configurable(config_path), sk_path{sk_path}, timeout{timeout} {
  load_configuration();
}

void SKPutRequestBase::send_put_request() {
  DynamicJsonDocument doc(1024);
  JsonObject root = doc.to<JsonObject>();
  JsonObject put_data = root.createNestedObject("put");
  put_data["path"] = sk_path;
  set_put_value(put_data);
  this->pending_request_id_ = SKRequest::send_request(
      doc,
      [this](DynamicJsonDocument& response) { this->on_response(response); },
      timeout);
}

bool SKPutRequestBase::request_pending() {
  return (get_request(this->pending_request_id_) != nullptr);
}

void SKPutRequestBase::on_response(DynamicJsonDocument& response) {
  String request_id = response["requestId"];
  String state = response["state"];
  debugD("Response %s received for PUT request: %s", state.c_str(),
         request_id.c_str());
}

void SKPutRequestBase::get_configuration(JsonObject& root) {
  root["sk_path"] = sk_path;
}

static const char SCHEMA[] PROGMEM = R"###({
    "type": "object",
      "properties": {
          "sk_path": { "title": "Signal K Path", "type": "string" }
      }
  })###";

String SKPutRequestBase::get_config_schema() { return FPSTR(SCHEMA); }

bool SKPutRequestBase::set_configuration(const JsonObject& config) {
  String expected[] = {"sk_path"};
  for (auto str : expected) {
    if (!config.containsKey(str)) {
      return false;
    }
  }
  this->sk_path = config["sk_path"].as<String>();
  return true;
}

}  // namespace sensesp
