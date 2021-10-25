#include "ws_client.h"

#include <ArduinoJson.h>

#include "Arduino.h"
#ifdef ESP8266
#include <ESP8266HTTPClient.h>
#include <ESP8266mDNS.h>  // Include the mDNS library
#elif defined(ESP32)
#include <ESPmDNS.h>
#include <HTTPClient.h>
#endif

#include <WiFiClient.h>

#include "sensesp_app.h"
#include "signalk/signalk_listener.h"
#include "signalk/signalk_put_request.h"
#include "signalk/signalk_put_request_listener.h"
#include "system/uuid.h"

WSClient* ws_client;

static const char* kRequestPermission = "readwrite";

bool WSClient::test_auth_on_each_connect_ = true;

void webSocketClientEvent(WStype_t type, uint8_t* payload, size_t length) {
  switch (type) {
    case WStype_DISCONNECTED:
      ws_client->on_disconnected();
      break;
    case WStype_ERROR:
      ws_client->on_error();
      break;
    case WStype_CONNECTED:
      ws_client->on_connected(payload);
      break;
    case WStype_TEXT:
      ws_client->on_receive_delta(payload);
      break;
    default:
      // Do nothing for other types
      break;
  }
}

WSClient::WSClient(String config_path, SKDeltaQueue* sk_delta_queue,
                   String server_address, uint16_t server_port)
    : Configurable{config_path} {
  this->sk_delta_queue_ = sk_delta_queue;

  preset_server_address_ = server_address;
  preset_server_port_ = server_port;
  this->server_address_ = server_address;
  this->server_port_ = server_port;

  // a WSClient object observes its own connection_state_ member
  // and simply passes through any notification it emits. As a result,
  // whenever the value of connection_state_ is updated, observers of the
  // WSClient object get automatically notified.
  this->connection_state_.attach(
      [this]() { this->emit(this->connection_state_.get()); });

  // set the singleton object pointer
  ws_client = this;

  load_configuration();
}

void WSClient::enable() {
  app.onDelay(0, [this]() { this->connect(); });
  app.onRepeat(20, [this]() { this->loop(); });
  app.onRepeat(5, [this]() { this->send_delta(); });
  app.onRepeat(10000, [this]() { this->connect_loop(); });
}

void WSClient::connect_loop() {
  if (this->connection_state_ == WSConnectionState::kWSDisconnected) {
    this->connect();
  }
}

void WSClient::on_disconnected() {
  if (this->connection_state_ == WSConnectionState::kWSConnecting &&
      server_detected_ && !token_test_success_) {
    // Going from connecting directly to disconnect when we
    // know we have found and talked to the server usually means
    // the authentication token is bad.
    debugW("Bad access token detected. Setting token to null.");
    auth_token_ = NULL_AUTH_TOKEN;
    save_configuration();
  }
  this->connection_state_ = WSConnectionState::kWSDisconnected;
  server_detected_ = false;
}

void WSClient::on_error() {
  this->connection_state_ = WSConnectionState::kWSDisconnected;
  debugW("Websocket client error.");
}

void WSClient::on_connected(uint8_t* payload) {
  this->connection_state_ = WSConnectionState::kWSConnected;
  this->sk_delta_queue_->reset_meta_send();
  debugI("Websocket client connected to URL: %s\n", payload);
  debugI("Subscribing to Signal K listeners...");
  this->subscribe_listeners();
}

void WSClient::subscribe_listeners() {
  const std::vector<SKListener*>& listeners = SKListener::get_listeners();

  if (listeners.size() > 0) {
    DynamicJsonDocument subscription(1024);
    subscription["context"] = "vessels.self";
    JsonArray subscribe = subscription.createNestedArray("subscribe");

    for (size_t i = 0; i < listeners.size(); i++) {
      auto* listener = listeners.at(i);
      String sk_path = listener->get_sk_path();
      int listen_delay = listener->get_listen_delay();

      JsonObject subscribePath = subscribe.createNestedObject();

      subscribePath["path"] = sk_path;
      subscribePath["period"] = listen_delay;
      debugI("Adding %s subscription with listen_delay %d\n", sk_path.c_str(),
             listen_delay);
    }

    String messageJson;

    serializeJson(subscription, messageJson);
    debugI("Subscription JSON message:\n %s", messageJson.c_str());
    this->client_.sendTXT(messageJson);
  }
}

void WSClient::on_receive_delta(uint8_t* payload) {
#ifdef SIGNALK_PRINT_RCV_DELTA
  debugD("Websocket payload received: %s", (char*)payload);
#endif

  DynamicJsonDocument message(1024);
  // JsonObject message = jsonDoc.as<JsonObject>();
  auto error = deserializeJson(message, payload);

  if (!error) {
    if (message.containsKey("updates")) {
      on_receive_updates(message);
    }

    if (message.containsKey("put")) {
      on_receive_put(message);
    }

    if (message.containsKey("requestId")) {
      SKRequest::handle_response(message);
    }
  } else {
    debugE("deserializeJson error: %s", error.c_str());
  }
}

void WSClient::on_receive_updates(DynamicJsonDocument& message) {
  // Process updates from subscriptions...
  JsonArray updates = message["updates"];

  for (size_t i = 0; i < updates.size(); i++) {
    JsonObject update = updates[i];

    JsonArray values = update["values"];

    for (size_t vi = 0; vi < values.size(); vi++) {
      JsonObject value = values[vi];

      const char* path = value["path"];
      debugD("Got update of value %s\n", path);

      const std::vector<SKListener*>& listeners = SKListener::get_listeners();

      for (size_t i = 0; i < listeners.size(); i++) {
        SKListener* listener = listeners[i];
        if (listener->get_sk_path().equals(path)) {
          listener->parse_value(value);
        }
      }
    }
  }
}

void WSClient::on_receive_put(DynamicJsonDocument& message) {
  // Process PUT requests...
  JsonArray puts = message["put"];
  size_t responseCount = 0;
  for (size_t i = 0; i < puts.size(); i++) {
    JsonObject put = puts[i];
    const char* path = put["path"];
    String strVal = put["value"].as<String>();
    debugD("Received PUT request for path %s (value %s)\n", path,
           strVal.c_str());
    const std::vector<SKPutListener*>& listeners =
        SKPutListener::get_listeners();
    for (size_t i = 0; i < listeners.size(); i++) {
      SKPutListener* listener = listeners[i];
      if (listener->get_sk_path().equals(path)) {
        listener->parse_value(put);
        responseCount++;
      }
    }

    // Send back a request reasponse...
    DynamicJsonDocument putResponse(512);
    putResponse["requestId"] = message["requestId"];
    if (responseCount == puts.size()) {
      // We found a response for every PUT request
      putResponse["state"] = "COMPLETED";
      putResponse["statusCode"] = 200;
    } else {
      // One or more requests did not have a matching path
      putResponse["state"] = "FAILED";
      putResponse["statusCode"] = 405;
    }
    String responseTxt;
    serializeJson(putResponse, responseTxt);
    debugD("Replying to PUT request with %s", responseTxt.c_str());
    this->client_.sendTXT(responseTxt);
  }
}

void WSClient::sendTXT(String& payload) {
  if (connection_state_ == WSConnectionState::kWSConnected) {
    this->client_.sendTXT(payload);
  }
}

bool WSClient::get_mdns_service(String& server_address, uint16_t& server_port) {
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

void WSClient::connect() {
  debugI("WSClient websocket connect attempt (state=%d)",
         connection_state_.get());

  if (connection_state_ != WSConnectionState::kWSDisconnected) {
    return;
  }

  if (!WiFi.isConnected()) {
    debugI(
        "WiFi is disconnected. SignalK client connection will connect when "
        "WiFi is connected.");
    return;
  }

  debugD("Initiating websocket connection with server...");

  connection_state_ = WSConnectionState::kWSAuthorizing;

  String server_address = this->server_address_;
  uint16_t server_port = this->server_port_;

  if (this->server_address_.isEmpty()) {
    if (!get_mdns_service(server_address, server_port)) {
      debugE("No Signal K server found in network when using mDNS service!");
    } else {
      debugI("Signal K server has been found at address %s:%d by mDNS.",
             server_address.c_str(), server_port);
    }
  }

  if (!server_address.isEmpty() && server_port > 0) {
    debugD("Websocket is connecting to Signal K server on address %s:%d",
           server_address.c_str(), server_port);
  } else {
    // host and port not defined - wait for mDNS
    connection_state_ = WSConnectionState::kWSDisconnected;
    return;
  }

  if (this->polling_href_ != "") {
    // existing pending request
    this->poll_access_request(server_address, server_port, this->polling_href_);
    return;
  }

  if (this->auth_token_ == NULL_AUTH_TOKEN) {
    // initiate HTTP authentication
    debugD("No prior authorization token present.");
    this->send_access_request(server_address, server_port);
    return;
  }

  if (test_auth_on_each_connect_ || !token_test_success_) {
    // Test the validity of the authorization token for the first time...
    this->test_token(server_address, server_port);
  } else {
    // The token has already been validated once,
    // so we must be trying a subsequent reconnect.
    // Jump directly to opening up the websocket...
    server_detected_ = true;
    this->connect_ws(server_address, server_port);
  }
}

void WSClient::test_token(const String server_address,
                          const uint16_t server_port) {
  // FIXME: implement async HTTP client!
  HTTPClient http;

  String url =
      String("http://") + server_address + ":" + server_port + "/signalk/";
  debugD("Testing token with url %s", url.c_str());
  http.begin(wifi_client_, url);
  String full_token = String("JWT ") + auth_token_;
  http.addHeader("Authorization", full_token.c_str());
  int httpCode = http.GET();
  if (httpCode > 0) {
    String payload = http.getString();
    http.end();
    debugD("Testing resulted in http status %d", httpCode);
    if (payload.length() > 0) {
      debugD("Returned payload (length %d) is: ", payload.length());
      debugD("%s", payload.c_str());
      debugD("End of payload output");
    } else {
      debugD("Returned payload is empty");
    }
    if (httpCode == 200) {
      // our token is valid, go ahead and connect
      debugD("Attempting to connect to Signal K Websocket...");
      server_detected_ = true;
      token_test_success_ = true;
      this->connect_ws(server_address, server_port);
    } else if (httpCode == 401) {
      this->client_id_ = "";
      this->send_access_request(server_address, server_port);
    } else {
      connection_state_ = WSConnectionState::kWSDisconnected;
    }
  } else {
    debugE("GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
    connection_state_ = WSConnectionState::kWSDisconnected;
  }
}

void WSClient::send_access_request(const String server_address,
                                   const uint16_t server_port) {
  debugD("Preparing a new access request");
  if (client_id_ == "") {
    // generate a client ID
    client_id_ = generate_uuid4();
    save_configuration();
  }

  // create a new access request
  DynamicJsonDocument doc(1024);
  doc["clientId"] = client_id_;
  doc["description"] = String("SensESP device: ") + sensesp_app->get_hostname();
  doc["permissions"] = kRequestPermission;
  String json_req = "";
  serializeJson(doc, json_req);

  HTTPClient http;

  String url = String("http://") + server_address + ":" + server_port +
               "/signalk/v1/access/requests";
  http.begin(wifi_client_, url);
  http.addHeader("Content-Type", "application/json");
  int httpCode = http.POST(json_req);
  String payload = http.getString();
  http.end();

  // if we get a response we can't handle, try to reconnect later
  if (httpCode != 202) {
    debugW("Can't handle response %d to access request.", httpCode);
    debugD("%s", payload.c_str());
    connection_state_ = WSConnectionState::kWSDisconnected;
    client_id_ = "";
    return;
  }

  // http status code 202

  deserializeJson(doc, payload.c_str());
  String state = doc["state"];

  if (state != "PENDING") {
    debugW("Got unknown state: %s", state.c_str());
    connection_state_ = WSConnectionState::kWSDisconnected;
    client_id_ = "";
    return;
  }

  String href = doc["href"];
  polling_href_ = href;
  save_configuration();

  debugD("Polling %s in 5 seconds", polling_href_.c_str());
  app.onDelay(5000, [this, server_address, server_port, href]() {
    this->poll_access_request(server_address, server_port, this->polling_href_);
  });
}

void WSClient::poll_access_request(const String server_address,
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
    DynamicJsonDocument doc(1024);
    auto error = deserializeJson(doc, payload.c_str());
    if (error) {
      debugW("WARNING: Could not deserialize http payload.");
      debugW("DeserializationError: %s", error.c_str());
      return;  // TODO: return at this point, or keep going?
    }
    String state = doc["state"];
    debugD("%s", state.c_str());
    if (state == "PENDING") {
      app.onDelay(5000, [this, server_address, server_port, href]() {
        this->poll_access_request(server_address, server_port, href);
      });
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
        connection_state_ = WSConnectionState::kWSDisconnected;
        return;
      } else if (permission == "APPROVED") {
        debugI("Permission granted");
        String token = access_req["token"];
        auth_token_ = token;
        save_configuration();
        app.onDelay(0, [this, server_address, server_port]() {
          this->connect_ws(server_address, server_port);
        });
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
      connection_state_ = WSConnectionState::kWSDisconnected;
      return;
    }
    // any other HTTP status code
    debugW("Can't handle response %d to pending access request.\n", httpCode);
    connection_state_ = WSConnectionState::kWSDisconnected;
    return;
  }
}

void WSClient::connect_ws(const String host, const uint16_t port) {
  String path = "/signalk/v1/stream?subscribe=none";
  this->connection_state_ = WSConnectionState::kWSConnecting;
  this->client_.begin(host, port, path);
  this->client_.onEvent(webSocketClientEvent);
  String full_token = String("JWT ") + auth_token_;
  this->client_.setAuthorization(full_token.c_str());
}

void WSClient::loop() {
  if (this->connection_state_ == WSConnectionState::kWSConnecting ||
      this->connection_state_ == WSConnectionState::kWSConnected) {
    this->client_.loop();
  }
}

bool WSClient::is_connected() {
  return connection_state_ == WSConnectionState::kWSConnected;
}

void WSClient::restart() {
  if (connection_state_ == WSConnectionState::kWSConnected) {
    this->client_.disconnect();
    connection_state_ = WSConnectionState::kWSDisconnected;
  }
}

void WSClient::send_delta() {
  String output;
  if (connection_state_ == WSConnectionState::kWSConnected) {
    if (sk_delta_queue_->data_available()) {
      sk_delta_queue_->get_delta(output);
      this->client_.sendTXT(output);
      // This automatically notifies the observers
      this->delta_count_producer_ = 1;
    }
  }
}

void WSClient::get_configuration(JsonObject& root) {
  root["sk_address"] = this->server_address_;
  root["sk_port"] = this->server_port_;

  root["token"] = this->auth_token_;
  root["client_id"] = this->client_id_;
  root["polling_href"] = this->polling_href_;
}

static const char SCHEMA[] PROGMEM = R"~({
    "type": "object",
    "properties": {
        "sk_address": { "title": "Signal K server address", "type": "string" },
        "sk_port": { "title": "Signal K server port", "type": "integer" },
        "client_id": { "title": "Client ID (readonly)", "type": "string", "readOnly": true },
        "token": { "title": "Server authorization token (readonly)", "type": "string", "readOnly": true },
        "polling_href": { "title": "Server authorization polling href (readonly)", "type": "string", "readOnly": true }
    }
  })~";

// TODO: FIXME: Don't Repeat Yourself
static const char SCHEMA_READONLY[] PROGMEM = R"~(
  {
    "type": "object",
    "properties": {
        "sk_address": { "title": "Signal K server address (readonly)", "type": "string", "readOnly": true },
        "sk_port": { "title": "Signal K server port (readonly)", "type": "integer", "readOnly": true },
        "client_id": { "title": "Client ID  (readonly)", "type": "string", "readOnly": true },
        "token": { "title": "Server authorization token (readonly)", "type": "string", "readOnly": true },
        "polling_href": { "title": "Server authorization polling href (readonly)", "type": "string", "readOnly": true }
    }
  }
  )~";

String WSClient::get_config_schema() {
  if (!preset_server_address_.isEmpty()) {
    return FPSTR(SCHEMA);
  } else {
    return FPSTR(SCHEMA_READONLY);
  }
}

bool WSClient::set_configuration(const JsonObject& config) {
  String expected[] = {"sk_address", "sk_port", "token", "client_id"};
  for (auto str : expected) {
    if (!config.containsKey(str)) {
      debugI(
          "Websocket configuration update rejected. Missing following "
          "parameter: %s",
          str.c_str());
      return false;
    }
  }

  if (!preset_server_address_.isEmpty()) {
    debugI(
        "Saved Signal K server configuration ignored due to hardcoded values.");
  } else {
    this->server_address_ = config["sk_address"].as<String>();
    this->server_port_ = config["sk_port"].as<int>();
  }

  // FIXME: setting the token should not be allowed via the REST API.
  this->auth_token_ = config["token"].as<String>();
  this->client_id_ = config["client_id"].as<String>();
  this->polling_href_ = config["polling_href"].as<String>();

  return true;
}
