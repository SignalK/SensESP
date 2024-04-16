#include "signalk_ws_client.h"

#include <ArduinoJson.h>
#include <ESPmDNS.h>
#include <HTTPClient.h>
#include <WiFiClient.h>

#include "Arduino.h"
#include "elapsedMillis.h"
#include "sensesp.h"
#include "sensesp/signalk/signalk_listener.h"
#include "sensesp/signalk/signalk_put_request.h"
#include "sensesp/signalk/signalk_put_request_listener.h"
#include "sensesp/system/uuid.h"
#include "sensesp_app.h"

namespace sensesp {

constexpr int ws_client_task_stack_size = 8192;

SKWSClient* ws_client;

static const char* kRequestPermission = "readwrite";

void ExecuteWebSocketTask(void* parameter) {
  elapsedMillis connect_loop_elapsed = 0;
  elapsedMillis delta_loop_elapsed = 0;
  elapsedMillis ws_client_loop_elapsed = 0;

  ws_client->connect();

  while (true) {
    if (connect_loop_elapsed > 2000) {
      connect_loop_elapsed = 0;
      ws_client->connect();
    }
    if (delta_loop_elapsed > 5) {
      delta_loop_elapsed = 0;
      ws_client->send_delta();
    }
    delay(1);
  }
}

/**
 * @brief Websocket event handler.
 *
 * @param handler_args
 * @param base
 * @param event_id
 * @param event_data
 */
static void websocket_event_handler(void* handler_args, esp_event_base_t base,
                                    int32_t event_id, void* event_data) {
  esp_websocket_event_data_t* data = (esp_websocket_event_data_t*)event_data;
  switch (event_id) {
    case WEBSOCKET_EVENT_CONNECTED:
      debugD("WEBSOCKET_EVENT_CONNECTED");
      ws_client->on_connected();
      break;
    case WEBSOCKET_EVENT_DISCONNECTED:
      debugD("WEBSOCKET_EVENT_DISCONNECTED");
      ws_client->on_disconnected();
      break;
    case WEBSOCKET_EVENT_DATA:
      // check if the payload is text)
      if (data->op_code == 0x01) {
        ws_client->on_receive_delta((uint8_t*)data->data_ptr, data->data_len);
      }
      break;
    case WEBSOCKET_EVENT_ERROR:
      ws_client->on_error();
      break;
  }
}

SKWSClient::SKWSClient(String config_path, SKDeltaQueue* sk_delta_queue,
                   String server_address, uint16_t server_port, bool use_mdns)
    : Configurable{config_path, "/System/Signal K Settings", 200},
      sk_delta_queue_{sk_delta_queue},
      conf_server_address_{server_address},
      conf_server_port_{server_port},
      use_mdns_{use_mdns} {
  // a SKWSClient object observes its own connection_state_ member
  // and simply passes through any notification it emits. As a result,
  // whenever the value of connection_state_ is updated, observers of the
  // SKWSClient object get automatically notified.
  this->connection_state_.attach(
      [this]() { this->emit(this->connection_state_.get()); });

  // process any received updates in the main task
  ReactESP::app->onRepeat(1, [this]() { this->process_received_updates(); });

  // set the singleton object pointer
  ws_client = this;

  load_configuration();

  // Connect the counters
  delta_tx_tick_producer_.connect_to(&delta_tx_count_producer_);

  ReactESP::app->onDelay(0, [this]() {
    debugD("Starting SKWSClient");
    xTaskCreate(ExecuteWebSocketTask, "SKWSClient", ws_client_task_stack_size,
                this, 1, NULL);
    MDNS.addService("signalk-sensesp", "tcp", 80);
  });
}

void SKWSClient::connect_loop() {
  if (this->get_connection_state() == SKWSConnectionState::kSKWSDisconnected) {
    this->connect();
  }
}

/**
 * @brief Called when the websocket connection is disconnected.
 *
 * This method is called in the websocket task context.
 *
 */
void SKWSClient::on_disconnected() {
  if (this->get_connection_state() == SKWSConnectionState::kSKWSConnecting &&
      server_detected_ && !token_test_success_) {
    // Going from connecting directly to disconnect when we
    // know we have found and talked to the server usually means
    // the authentication token is bad.
    debugW("Bad access token detected. Setting token to null.");
    auth_token_ = NULL_AUTH_TOKEN;
    save_configuration();
  }
  this->set_connection_state(SKWSConnectionState::kSKWSDisconnected);
  server_detected_ = false;
}

/**
 * @brief Called when the websocket connection encounters an error.
 *
 * Called in the websocket task context.
 *
 */
void SKWSClient::on_error() {
  this->set_connection_state(SKWSConnectionState::kSKWSDisconnected);
  debugW("Websocket client error.");
}

/**
 * @brief Called when the websocket connection is established.
 *
 * Called in the websocket task context.
 */
void SKWSClient::on_connected() {
  this->set_connection_state(SKWSConnectionState::kSKWSConnected);
  this->sk_delta_queue_->reset_meta_send();
  debugI("Subscribing to Signal K listeners...");
  this->subscribe_listeners();
}

/**
 * @brief Subscribes the SK delta paths to the websocket.
 *
 * Called in the websocket task context.
 *
 */
void SKWSClient::subscribe_listeners() {
  bool output_available = false;
  JsonDocument subscription;
  subscription["context"] = "vessels.self";

  SKListener::take_semaphore();
  const std::vector<SKListener*>& listeners = SKListener::get_listeners();

  if (listeners.size() > 0) {
    output_available = true;
    JsonArray subscribe = subscription["subscribe"].to<JsonArray>();

    for (size_t i = 0; i < listeners.size(); i++) {
      auto* listener = listeners.at(i);
      String sk_path = listener->get_sk_path();
      int listen_delay = listener->get_listen_delay();

      JsonObject subscribePath = subscribe.add<JsonObject>();

      subscribePath["path"] = sk_path;
      subscribePath["period"] = listen_delay;
      debugI("Adding %s subscription with listen_delay %d\n", sk_path.c_str(),
             listen_delay);
    }
  }
  SKListener::release_semaphore();

  if (output_available) {
    String messageJson;

    serializeJson(subscription, messageJson);
    debugI("Subscription JSON message:\n %s", messageJson.c_str());
    esp_websocket_client_send_text(this->client_, messageJson.c_str(),
                                   messageJson.length(), portMAX_DELAY);
  }
}

/**
 * @brief Called when the websocket receives a delta.
 *
 * Called in the websocket task context.
 *
 * @param payload
 */
void SKWSClient::on_receive_delta(uint8_t* payload, size_t length) {
  // Need to work on null-terminated strings
  char buf[length + 1];
  memcpy(buf, payload, length);
  buf[length] = 0;

#ifdef SIGNALK_PRINT_RCV_DELTA
  debugD("Websocket payload received: %s", (char*)buf);
#endif

  JsonDocument message;
  // JsonObject message = jsonDoc.as<JsonObject>();
  auto error = deserializeJson(message, buf);

  if (!error) {
    if (message.containsKey("updates")) {
      on_receive_updates(message);
    }

    if (message.containsKey("put")) {
      on_receive_put(message);
    }

    // Putrequest contains also requestId Key  GA
    if (message.containsKey("requestId") && !message.containsKey("put")) {
      SKRequest::handle_response(message);
    }
  } else {
    debugE("deserializeJson error: %s", error.c_str());
  }
}

/**
 * @brief Called when a delta update is received.
 *
 * Called in the websocket task context.
 *
 * @param message
 */
void SKWSClient::on_receive_updates(JsonDocument& message) {
  // Process updates from subscriptions...
  JsonArray updates = message["updates"];

  take_received_updates_semaphore();
  for (size_t i = 0; i < updates.size(); i++) {
    JsonObject update = updates[i];

    JsonArray values = update["values"];

    for (size_t vi = 0; vi < values.size(); vi++) {
      JsonDocument value_doc = JsonDocument((JsonObject)values[vi]);

      // push all values into a separate list for processing
      // in the main task
      received_updates_.push_back(value_doc);
    }
  }
  release_received_updates_semaphore();
}

/**
 * @brief Loop through the received updates and process them.
 *
 * This method is called in the main task context.
 *
 */
void SKWSClient::process_received_updates() {
  SKListener::take_semaphore();

  const std::vector<SKListener*>& listeners = SKListener::get_listeners();
  const std::vector<SKPutListener*>& put_listeners =
      SKPutListener::get_listeners();

  take_received_updates_semaphore();
  int num_updates = received_updates_.size();
  while (!received_updates_.empty()) {
    JsonDocument& doc = received_updates_.front();

    const char* path = doc["path"];
    JsonObject value = doc.as<JsonObject>();

    for (size_t i = 0; i < listeners.size(); i++) {
      SKListener* listener = listeners[i];
      if (listener->get_sk_path().equals(path)) {
        listener->parse_value(value);
      }
    }
    //   to be able to parse values of Put Listeners    GA
    for (size_t i = 0; i < put_listeners.size(); i++) {
      SKPutListener* listener = put_listeners[i];
      if (listener->get_sk_path().equals(path)) {
        listener->parse_value(value);
      }
    }
    received_updates_.pop_front();
  }
  release_received_updates_semaphore();
  delta_rx_count_producer_.set(num_updates);

  SKListener::release_semaphore();
}

/**
 * @brief Called when a PUT event is received.
 *
 * Called in the websocket task context.
 *
 * @param message
 */
void SKWSClient::on_receive_put(JsonDocument& message) {
  // Process PUT requests...
  JsonArray puts = message["put"];
  size_t response_count = 0;
  for (size_t i = 0; i < puts.size(); i++) {
    JsonObject value = puts[i];
    const char* path = value["path"];
    String strVal = value["value"].as<String>();

    SKListener::take_semaphore();
    const std::vector<SKPutListener*>& listeners =
        SKPutListener::get_listeners();
    for (size_t i = 0; i < listeners.size(); i++) {
      SKPutListener* listener = listeners[i];
      if (listener->get_sk_path().equals(path)) {
        take_received_updates_semaphore();
        received_updates_.push_back(value);
        release_received_updates_semaphore();
        response_count++;
      }
    }
    SKListener::release_semaphore();

    // Send back a request response...
    JsonDocument put_response;
    put_response["requestId"] = message["requestId"];
    if (response_count == puts.size()) {
      // We found a response for every PUT request
      put_response["state"] = "COMPLETED";
      put_response["statusCode"] = 200;
    } else {
      // One or more requests did not have a matching path
      put_response["state"] = "FAILED";
      put_response["statusCode"] = 405;
    }
    String response_text;
    serializeJson(put_response, response_text);
    esp_websocket_client_send_text(this->client_, response_text.c_str(),
                                   response_text.length(), portMAX_DELAY);
  }
}

/**
 * @brief Send some processed data to the websocket.
 *
 * Called in the websocket task context.
 *
 * @param payload
 */
void SKWSClient::sendTXT(String& payload) {
  if (get_connection_state() == SKWSConnectionState::kSKWSConnected) {
    esp_websocket_client_send_text(this->client_, payload.c_str(),
                                   payload.length(), portMAX_DELAY);
  }
}

bool SKWSClient::get_mdns_service(String& server_address, uint16_t& server_port) {
  // get IP address using an mDNS query
  int n = MDNS.queryService("signalk-ws", "tcp");
  if (n == 0) {
    // no service found
    return false;
  } else {
    server_address = MDNS.IP(0).toString();
    server_port = MDNS.port(0);
    debugI("Found server %s (port %d)", server_address.c_str(), server_port);
    return true;
  }
}

void SKWSClient::connect() {
  if (get_connection_state() != SKWSConnectionState::kSKWSDisconnected) {
    return;
  }

  if (!WiFi.isConnected() && WiFi.getMode() != WIFI_MODE_AP) {
    debugI(
        "WiFi is disconnected. SignalK client connection will be initiated "
        "when WiFi is connected.");
    return;
  }

  debugI("Initiating websocket connection with server...");

  set_connection_state(SKWSConnectionState::kSKWSAuthorizing);
  if (use_mdns_) {
    if (!get_mdns_service(this->server_address_, this->server_port_)) {
      debugE("No Signal K server found in network when using mDNS service!");
    } else {
      debugI("Signal K server has been found at address %s:%d by mDNS.",
             this->server_address_.c_str(), this->server_port_);
    }
  } else {
    this->server_address_ = this->conf_server_address_;
    this->server_port_ = this->conf_server_port_;
  }

  if (!this->server_address_.isEmpty() && this->server_port_ > 0) {
    debugD("Websocket is connecting to Signal K server on address %s:%d",
           this->server_address_.c_str(), this->server_port_);
  } else {
    // host and port not defined - don't try to connect
    debugD(
        "Websocket is not connecting to Signal K server because host and "
        "port are not defined.");
    set_connection_state(SKWSConnectionState::kSKWSDisconnected);
    return;
  }

  if (this->polling_href_ != "") {
    // existing pending request
    this->poll_access_request(this->server_address_, this->server_port_,
                              this->polling_href_);
    return;
  }

  if (this->auth_token_ == NULL_AUTH_TOKEN) {
    // initiate HTTP authentication
    debugD("No prior authorization token present.");
    this->send_access_request(this->server_address_, this->server_port_);
    return;
  }

  // Test the validity of the authorization token
  this->test_token(this->server_address_, this->server_port_);
}

void SKWSClient::test_token(const String server_address,
                          const uint16_t server_port) {
  // FIXME: implement async HTTP client!
  HTTPClient http;

  String url = String("http://") + server_address + ":" + server_port +
               "/signalk/v1/stream";
  debugD("Testing token with url %s", url.c_str());
  http.begin(wifi_client_, url);
  String full_token = String("Bearer ") + auth_token_;
  debugD("Authorization: %s", full_token.c_str());
  http.addHeader("Authorization", full_token.c_str());
  int http_code = http.GET();
  if (http_code > 0) {
    String payload = http.getString();
    http.end();
    debugD("Testing resulted in http status %d", http_code);
    if (payload.length() > 0) {
      debugD("Returned payload (length %d) is: ", payload.length());
      debugD("%s", payload.c_str());
    } else {
      debugD("Returned payload is empty");
    }
    if (http_code == 426) {
      // HTTP status 426 is "Upgrade Required", which is the expected
      // response for a websocket connection.
      debugD("Attempting to connect to Signal K Websocket...");
      server_detected_ = true;
      token_test_success_ = true;
      this->connect_ws(server_address, server_port);
    } else if (http_code == 401) {
      this->client_id_ = "";
      this->send_access_request(server_address, server_port);
    } else {
      set_connection_state(SKWSConnectionState::kSKWSDisconnected);
    }
  } else {
    debugE("GET... failed, error: %s\n", http.errorToString(http_code).c_str());
    set_connection_state(SKWSConnectionState::kSKWSDisconnected);
  }
}

void SKWSClient::send_access_request(const String server_address,
                                   const uint16_t server_port) {
  debugD("Preparing a new access request");
  if (client_id_ == "") {
    // generate a client ID
    client_id_ = generate_uuid4();
    save_configuration();
  }

  // create a new access request
  JsonDocument doc;
  doc["clientId"] = client_id_;
  doc["description"] =
      String("SensESP device: ") + SensESPBaseApp::get_hostname();
  doc["permissions"] = kRequestPermission;
  String json_req = "";
  serializeJson(doc, json_req);

  debugD("Access request: %s", json_req.c_str());

  HTTPClient http;

  String url = String("http://") + server_address + ":" + server_port +
               "/signalk/v1/access/requests";
  debugD("Access request url: %s", url.c_str());
  http.begin(wifi_client_, url);
  http.addHeader("Content-Type", "application/json");
  int httpCode = http.POST(json_req);
  String payload = http.getString();
  http.end();

  // if we get a response we can't handle, try to reconnect later
  if (httpCode != 202) {
    debugW("Can't handle response %d to access request.", httpCode);
    debugD("%s", payload.c_str());
    set_connection_state(SKWSConnectionState::kSKWSDisconnected);
    client_id_ = "";
    return;
  }

  // http status code 202

  deserializeJson(doc, payload.c_str());
  String state = doc["state"];

  if (state != "PENDING") {
    debugW("Got unknown state: %s", state.c_str());
    set_connection_state(SKWSConnectionState::kSKWSDisconnected);
    client_id_ = "";
    return;
  }

  String href = doc["href"];
  polling_href_ = href;
  save_configuration();

  delay(5000);
  this->poll_access_request(server_address, server_port, this->polling_href_);
}

void SKWSClient::poll_access_request(const String server_address,
                                   const uint16_t server_port,
                                   const String href) {
  debugD("Polling SK Server for authentication token");

  HTTPClient http;

  String url = String("http://") + server_address + ":" + server_port + href;
  http.begin(wifi_client_, url);
  int httpCode = http.GET();
  if (httpCode == 200 or httpCode == 202) {
    String payload = http.getString();
    http.end();
    JsonDocument doc;
    auto error = deserializeJson(doc, payload.c_str());
    if (error) {
      debugW("WARNING: Could not deserialize http payload.");
      debugW("DeserializationError: %s", error.c_str());
      return;  // TODO: return at this point, or keep going?
    }
    String state = doc["state"];
    debugD("%s", state.c_str());
    if (state == "PENDING") {
      set_connection_state(SKWSConnectionState::kSKWSDisconnected);
      delay(5000);
      return;
    } else if (state == "COMPLETED") {
      JsonObject access_req = doc["accessRequest"];
      String permission =
          access_req["permission"];  // TODO: like this in ArdJson 6? String
                                     // permission =
                                     // resp["accessRequest"]["permission"];
      polling_href_ = "";
      save_configuration();

      if (permission == "DENIED") {
        debugW("Permission denied");
        set_connection_state(SKWSConnectionState::kSKWSDisconnected);
        return;
      } else if (permission == "APPROVED") {
        debugI("Permission granted");
        String token = access_req["token"];
        auth_token_ = token;
        save_configuration();
        this->connect_ws(server_address, server_port);
        return;
      }
    }
  } else {
    http.end();
    if (httpCode == 500) {
      // this is probably the server barfing due to
      // us polling a non-existing request. Just
      // delete the polling href.
      debugD("Got 500, probably a non-existing request.");
      polling_href_ = "";
      save_configuration();
      set_connection_state(SKWSConnectionState::kSKWSDisconnected);
      return;
    }
    // any other HTTP status code
    debugW("Can't handle response %d to pending access request.\n", httpCode);
    set_connection_state(SKWSConnectionState::kSKWSDisconnected);
    return;
  }
}

void SKWSClient::connect_ws(const String host, const uint16_t port) {
  String path = "/signalk/v1/stream?subscribe=none";
  set_connection_state(SKWSConnectionState::kSKWSConnecting);

  esp_err_t error;

  String url = String("ws://") + host + ":" + port + path;

  esp_websocket_client_config_t websocket_cfg = {};
  websocket_cfg.uri = url.c_str();

  String full_auth_header =
      String("Authorization: Bearer ") + auth_token_ + "\r\n";

  websocket_cfg.headers = full_auth_header.c_str();

  debugD("Websocket config: %s", websocket_cfg.uri);
  debugD("Initializing websocket client...");
  this->client_ = esp_websocket_client_init(&websocket_cfg);
  debugD("Registering websocket event handler...");
  error = esp_websocket_register_events(this->client_, WEBSOCKET_EVENT_ANY,
                                        websocket_event_handler,
                                        (void*)this->client_);
  if (error != ESP_OK) {
    debugE("Error registering websocket event handler: %d", error);
  }
  debugD("Starting websocket client...");
  error = esp_websocket_client_start(this->client_);
  if (error != ESP_OK) {
    debugE("Error starting websocket client: %d", error);
  }
  debugD("Websocket client started.");
}

bool SKWSClient::is_connected() {
  return get_connection_state() == SKWSConnectionState::kSKWSConnected;
}

void SKWSClient::restart() {
  if (get_connection_state() == SKWSConnectionState::kSKWSConnected) {
    esp_websocket_client_close(this->client_, portMAX_DELAY);
    set_connection_state(SKWSConnectionState::kSKWSDisconnected);
  }
}

void SKWSClient::send_delta() {
  String output;
  if (get_connection_state() == SKWSConnectionState::kSKWSConnected) {
    if (sk_delta_queue_->data_available()) {
      sk_delta_queue_->get_delta(output);
      esp_websocket_client_send_text(this->client_, output.c_str(),
                                     output.length(), portMAX_DELAY);
      // This automatically notifies the observers
      this->delta_tx_tick_producer_.set(1);
    }
  }
}

void SKWSClient::get_configuration(JsonObject& root) {
  root["sk_address"] = this->conf_server_address_;
  root["sk_port"] = this->conf_server_port_;
  root["use_mdns"] = this->use_mdns_;

  root["token"] = this->auth_token_;
  root["client_id"] = this->client_id_;
  root["polling_href"] = this->polling_href_;
}

bool SKWSClient::set_configuration(const JsonObject& config) {
  if (config.containsKey("sk_address")) {
    this->conf_server_address_ = config["sk_address"].as<String>();
  }
  if (config.containsKey("sk_port")) {
    this->conf_server_port_ = config["sk_port"].as<int>();
  }
  if (config.containsKey("use_mdns")) {
    this->use_mdns_ = config["use_mdns"].as<bool>();
  }
  if (config.containsKey("token")) {
    this->auth_token_ = config["token"].as<String>();
  }
  if (config.containsKey("client_id")) {
    this->client_id_ = config["client_id"].as<String>();
  }
  if (config.containsKey("polling_href")) {
    this->polling_href_ = config["polling_href"].as<String>();
  }

  return true;
}

/**
 * @brief Get a String representation of the current connection state
 *
 * @return String
 */
String SKWSClient::get_connection_status() {
  auto state = get_connection_state();
  switch (state) {
    case SKWSConnectionState::kSKWSAuthorizing:
      return "Authorizing with SignalK";
    case SKWSConnectionState::kSKWSConnected:
      return "Connected";
    case SKWSConnectionState::kSKWSConnecting:
      return "Connecting";
    case SKWSConnectionState::kSKWSDisconnected:
      return "Disconnected";
  }

  return "Unknown";
}

}  // namespace sensesp
