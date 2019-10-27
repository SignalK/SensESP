#include "ws_client.h"

#include "Arduino.h"

#include <ArduinoJson.h>
#include <ESP8266HTTPClient.h>
#ifdef ESP8266
  #include <ESP8266mDNS.h>        // Include the mDNS library
#elif defined(ESP32)
  #include <ESPmDNS.h>
#endif

#include <ESP8266TrueRandom.h>

#include <WiFiClient.h>

#include "sensesp_app.h"

WSClient* ws_client;

void webSocketClientEvent(WStype_t type, uint8_t * payload, size_t length) {
  switch(type) {
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
   }
}

WSClient::WSClient(String config_path, SKDelta* sk_delta,
                   std::function<void(bool)> connected_cb,
                   void_cb_func delta_cb) : Configurable{config_path} {
  this->sk_delta = sk_delta;
  this->connected_cb = connected_cb;
  this->delta_cb = delta_cb;

  // set the singleton object pointer
  ws_client = this;

  load_configuration();
}

void WSClient::enable() {
  app.onDelay(0, [this](){ this->connect(); });
  app.onRepeat(20, [this](){ this->loop(); });
  app.onRepeat(100, [this](){ this->send_delta(); });
  app.onRepeat(10000, [this](){ this->connect_loop(); });
}

void WSClient::connect_loop() {
  if (this->connection_state==disconnected) {
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

void WSClient::on_connected(uint8_t * payload) {
  this->connection_state = connected;
  debugI("Websocket client connected to URL: %s\n", payload);
  this->connected_cb(true);
}

void WSClient::on_receive_delta(uint8_t * payload) {
  debugD("Websocket payload received: %s", (char*)payload);
}

bool WSClient::get_mdns_service(String& server_address, uint16_t& server_port) {
  // get IP address using an mDNS query
  int n = MDNS.queryService("signalk-ws", "tcp");
  if (n==0) {
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
  if (connection_state!=disconnected) {
    return;
  }
  debugD("Initiating connection");

  connection_state = connecting;

  String server_address = "";
  uint16_t server_port = 80;

  if (this->server_address.length() == 0) {
    get_mdns_service(server_address, server_port);
  } else {
    server_address = this->server_address;
    server_port = this->server_port;
  }

  if ((server_address.length() > 0) && (server_port > 0)) {
    debugD("Websocket client starting");
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

void WSClient::test_token(const String server_address, const uint16_t server_port) {

  // FIXME: implement async HTTP client!
  WiFiClient client;
  HTTPClient http;

  String url = String("http://") + server_address + ":" + server_port + "/signalk/v1/api/";
  debugD("Testing token with url %s", url.c_str());
  http.begin(client, url);
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
    }
    else {
      debugD("Returned payload is empty");
    }
    if (httpCode == 200) {
      // our token is valid, go ahead and connect
      debugD("Attempting to connect to SignalK Websocket...");
      server_detected = true;
      this->connect_ws(server_address, server_port);
    } else if (httpCode == 401) {
      this->send_access_request(server_address, server_port);
    } else {
      connection_state = disconnected;
    }
  } else {
    debugE("GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
    connection_state = disconnected;
  }
}

void WSClient::send_access_request(const String server_address, const uint16_t server_port) {
  debugD("Preparing a new access request");
  if (client_id == "") {
    // generate a client ID
    byte uuidNumber[16];
    ESP8266TrueRandom.uuid(uuidNumber);
    client_id = ESP8266TrueRandom.uuidToString(uuidNumber);
    save_configuration();
  }

  // create a new access request
  DynamicJsonBuffer buf;
  JsonObject& req = buf.createObject();
  req["clientId"] = client_id;
  req["description"] =
    String("SensESP device: ") + sensesp_app->get_hostname();
  String json_req = "";
  req.printTo(json_req);

  WiFiClient client;
  HTTPClient http;

  String url = String("http://") + server_address + ":" + server_port
      + "/signalk/v1/access/requests";
  http.begin(client, url);
  http.addHeader("Content-Type", "application/json");
  int httpCode = http.POST(json_req);
  String payload = http.getString();
  http.end();

  // if we get a response we can't handle, try to reconnect later
  if (httpCode != 202) {
    debugW("Can't handle response %d to access request.", httpCode);
    debugD("%s", payload.c_str());
    connection_state = disconnected;
    return;
  }

  // http status code 202

  JsonObject& resp = buf.parseObject(payload);
  String state = resp["state"];

  if (state != "PENDING") {
    debugW("Got unknown state: %s", state.c_str());
    connection_state = disconnected;
    return;
  }

  String href = resp["href"];
  polling_href = href;
  save_configuration();

  debugD("Polling %s in 5 seconds", polling_href.c_str());
  app.onDelay(5000, [this, server_address, server_port](){
    this->poll_access_request(server_address, server_port, this->polling_href);
  });
}

void WSClient::poll_access_request(const String server_address, const uint16_t server_port, const String href) {
  debugD("Polling SK Server for authentication token");

  WiFiClient client;
  HTTPClient http;

  String url = String("http://") + server_address + ":" + server_port + href;
  http.begin(client, url);
  int httpCode = http.GET();
  if (httpCode == 200 or httpCode == 202) {
    String payload = http.getString();
    http.end();
    DynamicJsonBuffer buf;
    JsonObject& resp = buf.parseObject(payload);
    String state = resp["state"];
    debugD("%s", state.c_str());
    if (state == "PENDING") {
      app.onDelay(5000, [this, server_address, server_port, href](){ this->poll_access_request(server_address, server_port, href); });
      return;
    } else if (state == "COMPLETED") {
      JsonObject& access_req = resp["accessRequest"];
      String permission = access_req["permission"];
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
        app.onDelay(0, [this, server_address, server_port](){ this->connect_ws(server_address, server_port); });
        return;
      }
    }
  } else {
    http.end();
    if (httpCode==500) {
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

  this->client.begin(host, port, path);
  this->client.onEvent(webSocketClientEvent);
  String full_token = String("JWT ") + auth_token;
  this->client.setAuthorization(full_token.c_str());
}

void WSClient::loop() {
  this->client.loop();
}

bool WSClient::is_connected() {
  return connection_state==connected;
}

void WSClient::restart() {
  if (connection_state==connected) {
    this->client.disconnect();
    connection_state = disconnected;
  }
}

void WSClient::send_delta() {
  String output;
  if (sk_delta->data_available()) {
    sk_delta->get_delta(output);
    if (connection_state==connected) {
      this->client.sendTXT(output);
      this->delta_cb();
    }
  }
}

JsonObject& WSClient::get_configuration(JsonBuffer& buf) {
  JsonObject& root = buf.createObject();
  root["sk_address"] = this->server_address;
  root["sk_port"] = this->server_port;
  root["token"] = this->auth_token;
  root["client_id"] = this->client_id;
  root["polling_href"] = this->polling_href;
  return root;
}

static const char SCHEMA[] PROGMEM = R"({
    "type": "object",
    "properties": {
        "sk_address": { "title": "SignalK server address", "type": "string" },
        "sk_port": { "title": "SignalK server port", "type": "integer" },
        "client_id": { "title": "Client ID", "type": "string", "readOnly": true },
        "token": { "title": "Server authorization token", "type": "string" },
        "polling_href": { "title": "Server authorization polling href", "type": "string", "readOnly": true }
    }
  })";

String WSClient::get_config_schema() {
  return FPSTR(SCHEMA);
}


bool WSClient::set_configuration(const JsonObject& config) {
  String expected[] = {"sk_address", "sk_port", "token", "client_id"};
  for (auto str : expected) {
    if (!config.containsKey(str)) {
      return false;
    }
  }
  this->server_address = config["sk_address"].as<String>();
  this->server_port = config["sk_port"].as<int>();
  // FIXME: setting the token should not be allowed via the REST API.
  this->auth_token = config["token"].as<String>();
  this->client_id = config["client_id"].as<String>();
  this->polling_href = config["polling_href"].as<String>();
  return true;
}
