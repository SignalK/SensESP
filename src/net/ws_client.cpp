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

#include <ESPTrueRandom.h>
#include <WiFiClient.h>

#include "sensesp_app.h"
#include "signalk/signalk_listener.h"

WSClient* ws_client;

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

WSClient::WSClient(String config_path, SKDelta* sk_delta, String server_address,
                   uint16_t server_port,
                   std::function<void(bool)> connected_cb,
                   void_cb_func delta_cb, String permission)
    : Configurable{config_path} {
  this->sk_delta = sk_delta;

  preset_server_address = server_address;
  preset_server_port = server_port;
  this->server_address = server_address;
  this->server_port = server_port;

  this->connected_cb = connected_cb;
  this->delta_cb = delta_cb;

  this->sk_permission = permission;
  // set the singleton object pointer
  ws_client = this;

  load_configuration();
}

void WSClient::enable() {
  app.onDelay(0, [this]() { this->connect(); });
  app.onRepeat(20, [this]() { this->loop(); });
  app.onRepeat(100, [this]() { this->send_delta(); });
  app.onRepeat(10000, [this]() { this->connect_loop(); });
}

void WSClient::connect_loop() {
  if (this->connection_state == disconnected) {
    this->connect();
  }
}

void WSClient::on_disconnected() {
  if (this->connection_state == connecting && server_detected) {
    // Going from connecting directly to disconnect when we
    // know we have found and talked to the server usually means
    // the authentication token is bad.
    debugW("Bad access token detected. Setting token to null.");
    auth_token = NULL_AUTH_TOKEN;
    save_configuration();
  }
  this->connection_state = disconnected;
  server_detected = false;
  this->connected_cb(false);
}

void WSClient::on_error() {
  this->connection_state = disconnected;
  debugW("Websocket client error.");
  this->connected_cb(false);
}

void WSClient::on_connected(uint8_t* payload) {
  this->connection_state = connected;
  debugI("Websocket client connected to URL: %s\n", payload);
  this->connected_cb(true);
  debugI("Subscribing to Signal K listeners...");
  this->subscribe_listeners();
}

void WSClient::subscribe_listeners() {
  const std::vector<SKListener*>& listeners = SKListener::get_listeners();

  if (listeners.size() > 0) {
    DynamicJsonDocument jsonDoc(1024);
    JsonObject subscription = jsonDoc.as<JsonObject>();
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
    this->client.sendTXT(messageJson);
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
    JsonArray updates = message["updates"];

    for (size_t i = 0; i < updates.size(); i++) {
      JsonObject update = updates[i];

      JsonArray values = update["values"];

      for (size_t vi = 0; vi < values.size(); vi++) {
        JsonObject value = values[vi];

        const char* path = value["path"];
        // debugD("Got update of value %s\n", path);

        const std::vector<SKListener*>& listeners = SKListener::get_listeners();

        for (size_t i = 0; i < listeners.size(); i++) {
          SKListener* listener = listeners[i];
          if (listener->get_sk_path().equals(path)) {
            listener->parse_value(value);
          }
        }
      }
    }
  } else {
    debugE("deserializeJson error: %s", error.c_str());
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
  if (connection_state != disconnected) {
    return;
  }
  debugD("Initiating connection");

  connection_state = authorizing;

  String server_address = this->server_address;
  uint16_t server_port = this->server_port;

  if (this->server_address.isEmpty()) {
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
    connection_state = disconnected;
    return;
  }

  if (this->polling_href != "") {
    // existing pending request
    this->poll_access_request(server_address, server_port, this->polling_href);
    return;
  }

  if (this->auth_token == NULL_AUTH_TOKEN) {
    // initiate HTTP authentication
    debugD("No prior authorization token present.");
    this->send_access_request(server_address, server_port);
    return;
  }
  this->test_token(server_address, server_port);
}

void WSClient::test_token(const String server_address,
                          const uint16_t server_port) {
  // FIXME: implement async HTTP client!
  HTTPClient http;

  String url = String("http://") + server_address + ":" + server_port +
               "/signalk/v1/api/";
  debugD("Testing token with url %s", url.c_str());
  http.begin(wifi_client, url);
  String full_token = String("JWT ") + auth_token;
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
      server_detected = true;
      this->connect_ws(server_address, server_port);
    } else if (httpCode == 401) {
      this->client_id = "";
      this->send_access_request(server_address, server_port);
    } else {
      connection_state = disconnected;
    }
  } else {
    debugE("GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
    connection_state = disconnected;
  }
}

void WSClient::send_access_request(const String server_address,
                                   const uint16_t server_port) {
  debugD("Preparing a new access request");
  if (client_id == "") {
    // generate a client ID
    byte uuidNumber[16];
    ESPTrueRandom.uuid(uuidNumber);
    client_id = ESPTrueRandom.uuidToString(uuidNumber);
    save_configuration();
  }

  // create a new access request
  DynamicJsonDocument doc(1024);
  doc["clientId"] = client_id;
  doc["description"] = String("SensESP device: ") + sensesp_app->get_hostname();
  doc["permissions"] = this->sk_permission;
  String json_req = "";
  serializeJson(doc, json_req);

  HTTPClient http;

  String url = String("http://") + server_address + ":" + server_port +
               "/signalk/v1/access/requests";
  http.begin(wifi_client, url);
  http.addHeader("Content-Type", "application/json");
  int httpCode = http.POST(json_req);
  String payload = http.getString();
  http.end();

  // if we get a response we can't handle, try to reconnect later
  if (httpCode != 202) {
    debugW("Can't handle response %d to access request.", httpCode);
    debugD("%s", payload.c_str());
    connection_state = disconnected;
    client_id = "";
    return;
  }

  // http status code 202

  deserializeJson(doc, payload.c_str());
  String state = doc["state"];

  if (state != "PENDING") {
    debugW("Got unknown state: %s", state.c_str());
    connection_state = disconnected;
    client_id = "";
    return;
  }

  String href = doc["href"];
  polling_href = href;
  save_configuration();

  debugD("Polling %s in 5 seconds", polling_href.c_str());
  app.onDelay(5000, [this, server_address, server_port, href]() {
    this->poll_access_request(server_address, server_port, this->polling_href);
  });
}

void WSClient::poll_access_request(const String server_address,
                                   const uint16_t server_port,
                                   const String href) {
  debugD("Polling SK Server for authentication token");

  HTTPClient http;

  String url = String("http://") + server_address + ":" + server_port + href;
  http.begin(wifi_client, url);
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
      polling_href = "";
      save_configuration();

      if (permission == "DENIED") {
        debugW("Permission denied");
        connection_state = disconnected;
        return;
      } else if (permission == "APPROVED") {
        debugI("Permission granted");
        String token = access_req["token"];
        auth_token = token;
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
      polling_href = "";
      save_configuration();
      connection_state = disconnected;
      return;
    }
    // any other HTTP status code
    debugW("Can't handle response %d to pending access request.\n", httpCode);
    connection_state = disconnected;
    return;
  }
}

void WSClient::connect_ws(const String host, const uint16_t port) {
  String path = "/signalk/v1/stream?subscribe=none";
  this->connection_state = connecting;
  this->client.begin(host, port, path);
  this->client.onEvent(webSocketClientEvent);
  String full_token = String("JWT ") + auth_token;
  this->client.setAuthorization(full_token.c_str());
}

void WSClient::loop() {
  if (this->connection_state == connecting ||
      this->connection_state == connected) {
    this->client.loop();
  }
}

bool WSClient::is_connected() { return connection_state == connected; }

void WSClient::restart() {
  if (connection_state == connected) {
    this->client.disconnect();
    connection_state = disconnected;
  }
}

void WSClient::send_delta() {
  String output;
  if (connection_state == connected) {
    if (sk_delta->data_available()) {
      sk_delta->get_delta(output);
      this->client.sendTXT(output);
      this->delta_cb();
    }
  }
}

void WSClient::get_configuration(JsonObject& root) {
  root["sk_address"] = this->server_address;
  root["sk_port"] = this->server_port;

  root["token"] = this->auth_token;
  root["client_id"] = this->client_id;
  root["polling_href"] = this->polling_href;
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
  if (!preset_server_address.isEmpty()) {
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

  if (!preset_server_address.isEmpty()) {
    debugI(
        "Saved Signal K server configuration ignored due to hardcoded values.");
  } else {
    this->server_address = config["sk_address"].as<String>();
    this->server_port = config["sk_port"].as<int>();
  }

  // FIXME: setting the token should not be allowed via the REST API.
  this->auth_token = config["token"].as<String>();
  this->client_id = config["client_id"].as<String>();
  this->polling_href = config["polling_href"].as<String>();

  return true;
}
