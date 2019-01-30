#include "ws_client.h"

#include "Arduino.h"

#ifdef ESP8266
  #include <ESP8266mDNS.h>        // Include the mDNS library
#elif defined(ESP32)
  #include <ESPmDNS.h>
#endif


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

WSClient::WSClient(String id, String schema, SKDelta* sk_delta,
                   std::function<void(bool)> connected_cb,
                   void_cb_func delta_cb) : Configurable{id, schema} {
  this->sk_delta = sk_delta;
  this->connected_cb = connected_cb;
  this->delta_cb = delta_cb;

  // set the singleton object pointer
  ws_client = this;

  load_configuration();
}

void WSClient::enable() {
  this->connect();
  app.onRepeat(20, [this](){ this->loop(); });
  app.onRepeat(100, [this](){ this->send_delta(); });
}

void WSClient::on_disconnected() {
  this->connected = false;
  app.onDelay(10000, [this](){ this->connect(); });
  Serial.println("Websocket client disconnected.");
  this->connected_cb(false);
}

void WSClient::on_error() {
  this->connected = false;
  app.onDelay(10000, [this](){ this->connect(); });
  Serial.println("Websocket client error.");
  this->connected_cb(false);
}

void WSClient::on_connected(uint8_t * payload) {
  this->connected = true;
  Serial.printf("Websocket client connected to URL: %s\n", payload);
  this->connected_cb(true);
}

void WSClient::on_receive_delta(uint8_t * payload) {
  Serial.println("Delta receiving not implemented");
}

bool WSClient::get_mdns_service(String& host, uint16_t& port) {
  // get IP address using an mDNS query
  int n = MDNS.queryService("signalk-ws", "tcp");
  if (n==0) {
    // no service found
    return false;
  } else {
    host = MDNS.IP(0).toString();
    port = MDNS.port(0);
    Serial.print(F("Found server with IP/Port: "));
    Serial.print(host); Serial.print(":"); Serial.println(port);
    return true;
  }
}

void WSClient::connect() {
  String host = "";
  uint16_t port = 80;

  String url_args = "?subscribe=none";

  if (this->host.length() == 0) {
    get_mdns_service(host, port);
  } else {
    host = this->host;
    port = this->port;
  }

  if ( (host.length() > 0) &&
       (port > 0) &&
       (this->path.length() > 0) ) {
    Serial.println(F("Websocket client starting"));
  } else {
      app.onDelay(10000, [this](){ this->connect(); });
      return;
  }

  if (this->auth_token != "") {
    url_args = url_args + "&token=" + this->auth_token;
  }

  this->client.begin(host, port, this->path + url_args);
  this->connected_cb(true);
}

void WSClient::loop() {
  this->client.loop();
}

bool WSClient::is_connected() {
  return this->connected;
}

void WSClient::restart() {
  if (this->connected) {
    this->client.disconnect();
  }
}

void WSClient::send_delta() {
  String output;
  if (sk_delta->data_available()) {
    sk_delta->get_delta(output);
    this->client.sendTXT(output);
    this->delta_cb();
  }
}

JsonObject& WSClient::get_configuration(JsonBuffer& buf) {
  JsonObject& root = buf.createObject();
  root["sk_host"] = this->host;
  root["sk_port"] = this->port;
  root["sk_path"] = this->path;
  root["token"] = this->auth_token;
  return root;
}

void WSClient::set_configuration(const JsonObject& config) {
  this->host = config["hostname"].as<String>();
  this->port = config["sk_port"].as<int>();
  this->path = config["sk_path"].as<String>();
  this->auth_token = config["token"].as<String>();
  this->save_configuration();
  this->restart();
}
