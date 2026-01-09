#include "sensesp.h"

#include "signalk_ws_client.h"

#include <ArduinoJson.h>
#include <ESPmDNS.h>
#include <esp_http_client.h>

#ifdef SENSESP_SSL_SUPPORT
#include <mbedtls/sha256.h>
#include <mbedtls/ssl.h>
#include <mbedtls/x509_crt.h>
#endif

#include "Arduino.h"
#include "elapsedMillis.h"
#include "esp_arduino_version.h"
#include "sensesp/signalk/signalk_listener.h"
#include "sensesp/signalk/signalk_put_request.h"
#include "sensesp/signalk/signalk_put_request_listener.h"
#include "sensesp/system/uuid.h"
#include "sensesp_app.h"

namespace sensesp {

constexpr int kWsClientTaskStackSize = 8192;  // Reduced from 16KB to save heap

SKWSClient* ws_client;

static const char* kRequestPermission = "readwrite";

#ifdef SENSESP_SSL_SUPPORT
// Convert a SHA256 hash to hex string
static void sha256_to_hex(const uint8_t* sha256, char* hex) {
  for (int i = 0; i < 32; i++) {
    sprintf(hex + (i * 2), "%02x", sha256[i]);
  }
  hex[64] = '\0';
}

// TOFU verification callback - called during SSL handshake
// Returns 0 to allow connection, non-zero to reject
static int tofu_verify_callback(void* ctx, mbedtls_x509_crt* crt,
                                int depth, uint32_t* flags) {
  // Only check the server certificate (depth 0), not the CA chain
  if (depth != 0) {
    *flags = 0;  // Clear errors for intermediate certs
    return 0;
  }

  SKWSClient* client = static_cast<SKWSClient*>(ctx);
  if (client == nullptr) {
    ESP_LOGW("SKWSClient", "TOFU: No client context, allowing connection");
    *flags = 0;
    return 0;
  }

  // Compute SHA256 of the certificate
  uint8_t sha256[32];
  mbedtls_sha256_context sha256_ctx;
  mbedtls_sha256_init(&sha256_ctx);
  mbedtls_sha256_starts(&sha256_ctx, 0);  // 0 = SHA256 (not SHA224)
  mbedtls_sha256_update(&sha256_ctx, crt->raw.p, crt->raw.len);
  mbedtls_sha256_finish(&sha256_ctx, sha256);
  mbedtls_sha256_free(&sha256_ctx);

  char hex[65];
  sha256_to_hex(sha256, hex);
  String current_fingerprint = String(hex);

  ESP_LOGD("SKWSClient", "Server certificate fingerprint: %s", hex);

  if (!client->is_tofu_enabled()) {
    // TOFU disabled, allow any certificate
    ESP_LOGD("SKWSClient", "TOFU disabled, allowing connection");
    *flags = 0;
    return 0;
  }

  if (!client->has_tofu_fingerprint()) {
    // First connection - capture the fingerprint
    ESP_LOGI("SKWSClient", "TOFU: First connection, capturing fingerprint: %s", hex);
    client->set_tofu_fingerprint(current_fingerprint);
    *flags = 0;
    return 0;
  }

  // Verify against stored fingerprint
  if (client->get_tofu_fingerprint() == current_fingerprint) {
    ESP_LOGD("SKWSClient", "TOFU: Fingerprint verified successfully");
    *flags = 0;
    return 0;
  }

  // Fingerprint mismatch!
  ESP_LOGE("SKWSClient", "TOFU: Fingerprint mismatch!");
  ESP_LOGE("SKWSClient", "  Expected: %s", client->get_tofu_fingerprint().c_str());
  ESP_LOGE("SKWSClient", "  Received: %s", hex);
  // Return error to reject the connection
  return MBEDTLS_ERR_X509_CERT_VERIFY_FAILED;
}

// Certificate bundle attach function for TOFU verification
// Sets up a custom verification callback that implements Trust On First Use
static esp_err_t tofu_crt_bundle_attach(void* conf) {
  mbedtls_ssl_config* ssl_conf = static_cast<mbedtls_ssl_config*>(conf);
  // Use OPTIONAL so we can handle verification ourselves
  mbedtls_ssl_conf_authmode(ssl_conf, MBEDTLS_SSL_VERIFY_OPTIONAL);
  mbedtls_ssl_conf_verify(ssl_conf, tofu_verify_callback, ws_client);
  ESP_LOGD("SKWSClient", "TOFU verification callback installed");
  return ESP_OK;
}
#endif  // SENSESP_SSL_SUPPORT

static void websocket_event_handler(void* handler_args,
                                    esp_event_base_t base,
                                    int32_t event_id, void* event_data) {
  esp_websocket_event_data_t* data = (esp_websocket_event_data_t*)event_data;
  switch (event_id) {
    case WEBSOCKET_EVENT_CONNECTED:
      ws_client->on_connected();
      break;
    case WEBSOCKET_EVENT_DISCONNECTED:
      ws_client->on_disconnected();
      break;
    case WEBSOCKET_EVENT_DATA:
      ws_client->on_receive_delta((uint8_t*)data->data_ptr, data->data_len);
      break;
    case WEBSOCKET_EVENT_ERROR:
      ws_client->on_error();
      break;
  }
}

void ExecuteWebSocketTask(void* /*parameter*/) {
  elapsedMillis connect_loop_elapsed = 0;
  elapsedMillis delta_loop_elapsed = 0;

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
    delay(20);
  }
}


SKWSClient::SKWSClient(const String& config_path,
                       std::shared_ptr<SKDeltaQueue> sk_delta_queue,
                       const String& server_address, uint16_t server_port,
                       bool use_mdns)
    : FileSystemSaveable{config_path},
      conf_server_address_{server_address},
      conf_server_port_{server_port},
      use_mdns_{use_mdns},
      sk_delta_queue_{sk_delta_queue} {
  // a SKWSClient object observes its own connection_state_ member
  // and simply passes through any notification it emits. As a result,
  // whenever the value of connection_state_ is updated, observers of the
  // SKWSClient object get automatically notified.
  this->connection_state_.attach(
      [this]() { this->emit(this->connection_state_.get()); });

  // process any received updates in the main task
  event_loop()->onRepeat(1, [this]() { this->process_received_updates(); });

  // set the singleton object pointer
  ws_client = this;

  load();

  // Connect the counters
  delta_tx_tick_producer_.connect_to(&delta_tx_count_producer_);

  event_loop()->onDelay(0, [this]() {
    ESP_LOGD(__FILENAME__, "Starting SKWSClient");
    xTaskCreate(ExecuteWebSocketTask, "SKWSClient", kWsClientTaskStackSize,
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
    ESP_LOGW(__FILENAME__, "Bad access token detected. Setting token to null.");
    auth_token_ = NULL_AUTH_TOKEN;
    save();
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
  ESP_LOGW(__FILENAME__, "Websocket client error.");
}

/**
 * @brief Called when the websocket connection is established.
 *
 * Called in the websocket task context.
 */
void SKWSClient::on_connected() {
  this->set_connection_state(SKWSConnectionState::kSKWSConnected);
  this->sk_delta_queue_->reset_meta_send();
  ESP_LOGI(__FILENAME__, "Subscribing to Signal K listeners...");
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

      JsonObject subscribe_path = subscribe.add<JsonObject>();

      subscribe_path["path"] = sk_path;
      subscribe_path["period"] = listen_delay;
      ESP_LOGI(__FILENAME__, "Adding %s subscription with listen_delay %d\n",
               sk_path.c_str(), listen_delay);
    }
  }
  SKListener::release_semaphore();

  if (output_available) {
    String json_message;

    serializeJson(subscription, json_message);
    ESP_LOGI(__FILENAME__, "Subscription JSON message:\n %s",
             json_message.c_str());
    esp_websocket_client_send_text(client_, json_message.c_str(),
                                   json_message.length(), portMAX_DELAY);
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
  ESP_LOGD(__FILENAME__, "Websocket payload received: %s", (char*)buf);
#endif

  JsonDocument message;
  // JsonObject message = jsonDoc.as<JsonObject>();
  auto error = deserializeJson(message, buf);

  if (!error) {
    if (message["updates"].is<JsonVariant>()) {
      on_receive_updates(message);
    }

    if (message["put"].is<JsonVariant>()) {
      on_receive_put(message);
    }

    // Putrequest contains also requestId Key  GA
    if (message["requestId"].is<JsonVariant>() &&
        !message["put"].is<JsonVariant>()) {
      SKRequest::handle_response(message);
    }
  } else {
    ESP_LOGE(__FILENAME__, "deserializeJson error: %s", error.c_str());
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
      JsonDocument value_doc =
          static_cast<JsonDocument>(static_cast<JsonObject>((values[vi])));

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
    String str_val = value["value"].as<String>();

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
    esp_websocket_client_send_text(client_, response_text.c_str(),
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
    esp_websocket_client_send_text(client_, payload.c_str(), payload.length(),
                                   portMAX_DELAY);
  }
}

bool SKWSClient::get_mdns_service(String& server_address,
                                  uint16_t& server_port) {
  // get IP address using an mDNS query
  // Try SSL service first, then fall back to non-SSL
  int num = MDNS.queryService("signalk-wss", "tcp");
  if (num > 0) {
    // Found SSL-enabled server
    ssl_enabled_ = true;
    ESP_LOGI(__FILENAME__, "Found Signal K server via mDNS (signalk-wss)");
  } else {
    // Try non-SSL service
    num = MDNS.queryService("signalk-ws", "tcp");
    if (num == 0) {
      // no service found
      return false;
    }
    // Found non-SSL server, disable SSL
    ssl_enabled_ = false;
    ESP_LOGI(__FILENAME__, "Found Signal K server via mDNS (signalk-ws)");
  }

#if ESP_ARDUINO_VERSION_MAJOR < 3
  server_address = MDNS.IP(0).toString();
#else
  server_address = MDNS.address(0).toString();
#endif
  server_port = MDNS.port(0);
  ESP_LOGI(__FILENAME__, "Found server %s (port %d)", server_address.c_str(),
           server_port);
  return true;
}

bool SKWSClient::detect_ssl() {
  // Try to detect if the server requires SSL by checking for HTTP->HTTPS
  // redirects
  String url =
      String("http://") + server_address_ + ":" + server_port_ + "/signalk";

  ESP_LOGD(__FILENAME__, "Probing for SSL redirect at %s", url.c_str());

  esp_http_client_config_t config = {};
  config.url = url.c_str();
  config.disable_auto_redirect = true;
  config.timeout_ms = 10000;

  esp_http_client_handle_t client = esp_http_client_init(&config);
  if (client == nullptr) {
    ESP_LOGE(__FILENAME__, "Failed to initialize HTTP client");
    return false;
  }

  esp_err_t err = esp_http_client_perform(client);
  if (err != ESP_OK) {
    ESP_LOGD(__FILENAME__, "HTTP request failed: %s", esp_err_to_name(err));
    esp_http_client_cleanup(client);
    return false;
  }

  int http_code = esp_http_client_get_status_code(client);

  if (http_code == 301 || http_code == 302 || http_code == 307 ||
      http_code == 308) {
    // Check Location header for HTTPS redirect
    char* location = nullptr;
    esp_http_client_get_header(client, "Location", &location);
    esp_http_client_cleanup(client);

    if (location != nullptr && strncmp(location, "https://", 8) == 0) {
      ESP_LOGI(__FILENAME__, "SSL redirect detected to %s, enabling HTTPS/WSS",
               location);
      ssl_enabled_ = true;
      save();
      return true;
    }
  }

  esp_http_client_cleanup(client);
  return false;
}


void SKWSClient::connect() {
  if (get_connection_state() != SKWSConnectionState::kSKWSDisconnected) {
    return;
  }

  if (!WiFi.isConnected() && WiFi.getMode() != WIFI_MODE_AP) {
    ESP_LOGI(
        __FILENAME__,
        "WiFi is disconnected. SignalK client connection will be initiated "
        "when WiFi is connected.");
    return;
  }

  ESP_LOGI(__FILENAME__, "Initiating websocket connection with server...");

  set_connection_state(SKWSConnectionState::kSKWSAuthorizing);
  if (use_mdns_) {
    if (!get_mdns_service(this->server_address_, this->server_port_)) {
      ESP_LOGE(__FILENAME__,
               "No Signal K server found in network when using mDNS service!");
    } else {
      ESP_LOGI(__FILENAME__,
               "Signal K server has been found at address %s:%d by mDNS.",
               this->server_address_.c_str(), this->server_port_);
    }
  } else {
    this->server_address_ = this->conf_server_address_;
    this->server_port_ = this->conf_server_port_;
  }

  if (!this->server_address_.isEmpty() && this->server_port_ > 0) {
    ESP_LOGD(__FILENAME__,
             "Websocket is connecting to Signal K server on address %s:%d",
             this->server_address_.c_str(), this->server_port_);

    // Detect if server requires SSL (check for HTTP->HTTPS redirects)
    if (!ssl_enabled_) {
      detect_ssl();
    }
  } else {
    // host and port not defined - don't try to connect
    ESP_LOGD(__FILENAME__,
             "Websocket is not connecting to Signal K server because host and "
             "port are not defined.");
    set_connection_state(SKWSConnectionState::kSKWSDisconnected);
    return;
  }

  if (this->polling_href_.length() > 0 && this->polling_href_.startsWith("/")) {
    // existing pending request
    this->poll_access_request(this->server_address_, this->server_port_,
                              this->polling_href_);
    return;
  }

  if (this->auth_token_ == NULL_AUTH_TOKEN) {
    // initiate HTTP authentication
    ESP_LOGD(__FILENAME__, "No prior authorization token present.");
    this->send_access_request(this->server_address_, this->server_port_);
    return;
  }

  // Test the validity of the authorization token
  this->test_token(this->server_address_, this->server_port_);
}

void SKWSClient::loop() {
  // No-op: esp_websocket_client handles data via event callbacks
}

void SKWSClient::test_token(const String server_address,
                            const uint16_t server_port) {
  String protocol = ssl_enabled_ ? "https://" : "http://";
  String url = protocol + server_address + ":" + server_port +
               "/signalk/v1/stream";
  ESP_LOGD(__FILENAME__, "Testing token with url %s", url.c_str());

  const String full_token = String("Bearer ") + auth_token_;
  ESP_LOGD(__FILENAME__, "Authorization: %s", full_token.c_str());

  esp_http_client_config_t config = {};
  config.url = url.c_str();
  config.timeout_ms = 10000;
#ifdef SENSESP_SSL_SUPPORT
  if (ssl_enabled_) {
    config.crt_bundle_attach = tofu_crt_bundle_attach;
    config.skip_cert_common_name_check = true;
  }
#endif

  esp_http_client_handle_t client = esp_http_client_init(&config);
  if (client == nullptr) {
    ESP_LOGE(__FILENAME__, "Failed to initialize HTTP client");
    set_connection_state(SKWSConnectionState::kSKWSDisconnected);
    return;
  }

  esp_http_client_set_header(client, "Authorization", full_token.c_str());

  // Use streaming API for GET request
  esp_err_t err = esp_http_client_open(client, 0);
  if (err != ESP_OK) {
    ESP_LOGE(__FILENAME__, "Failed to open HTTP connection: %s", esp_err_to_name(err));
    esp_http_client_cleanup(client);
    set_connection_state(SKWSConnectionState::kSKWSDisconnected);
    return;
  }

  int content_length = esp_http_client_fetch_headers(client);
  int http_code = esp_http_client_get_status_code(client);

  ESP_LOGD(__FILENAME__, "Testing resulted in http status %d", http_code);

  // Read response body if present
  String payload;
  if (content_length > 0 && content_length < 1024) {
    char* buffer = new char[content_length + 1];
    int read_len = esp_http_client_read(client, buffer, content_length);
    buffer[read_len > 0 ? read_len : 0] = '\0';
    payload = String(buffer);
    delete[] buffer;
  }

  esp_http_client_close(client);
  esp_http_client_cleanup(client);

  if (payload.length() > 0) {
    ESP_LOGD(__FILENAME__, "Returned payload (length %d): %s",
             payload.length(), payload.c_str());
  }

  if (http_code == 426) {
    // HTTP status 426 is "Upgrade Required", which is the expected
    // response for a websocket connection.
    ESP_LOGD(__FILENAME__, "Attempting to connect to Signal K Websocket...");
    server_detected_ = true;
    token_test_success_ = true;
    this->connect_ws(server_address, server_port);
  } else if (http_code == 401) {
    // Token is invalid/expired - clear it and request new access
    // Keep client_id_ so we reuse the same device identity
    ESP_LOGW(__FILENAME__, "Token rejected (401), requesting new access");
    this->auth_token_ = NULL_AUTH_TOKEN;
    this->save();
    this->send_access_request(server_address, server_port);
  } else if (http_code > 0) {
    set_connection_state(SKWSConnectionState::kSKWSDisconnected);
  } else {
    ESP_LOGE(__FILENAME__, "HTTP request failed with code %d", http_code);
    set_connection_state(SKWSConnectionState::kSKWSDisconnected);
  }
}

void SKWSClient::send_access_request(const String server_address,
                                     const uint16_t server_port) {
  ESP_LOGI(__FILENAME__, "Sending access request (client_id=%s, ssl=%d)",
           client_id_.c_str(), ssl_enabled_);
  if (client_id_ == "") {
    // generate a client ID
    client_id_ = generate_uuid4();
    save();
  }

  // create a new access request
  JsonDocument doc;
  doc["clientId"] = client_id_;
  doc["description"] =
      String("SensESP device: ") + SensESPBaseApp::get_hostname();
  doc["permissions"] = kRequestPermission;
  String json_req = "";
  serializeJson(doc, json_req);

  ESP_LOGD(__FILENAME__, "Access request: %s", json_req.c_str());

  String protocol = ssl_enabled_ ? "https://" : "http://";
  String url = protocol + server_address + ":" + server_port +
               "/signalk/v1/access/requests";
  ESP_LOGD(__FILENAME__, "Access request url: %s", url.c_str());

  esp_http_client_config_t config = {};
  config.url = url.c_str();
  config.method = HTTP_METHOD_POST;
  config.timeout_ms = 10000;
#ifdef SENSESP_SSL_SUPPORT
  if (ssl_enabled_) {
    config.crt_bundle_attach = tofu_crt_bundle_attach;
    config.skip_cert_common_name_check = true;
  }
#endif

  esp_http_client_handle_t client = esp_http_client_init(&config);
  if (client == nullptr) {
    ESP_LOGE(__FILENAME__, "Failed to initialize HTTP client");
    set_connection_state(SKWSConnectionState::kSKWSDisconnected);
    // Don't clear client_id_ - keep device identity for retry
    return;
  }

  esp_http_client_set_header(client, "Content-Type", "application/json");

  // Use streaming API: open -> write request -> fetch headers -> read response
  esp_err_t err = esp_http_client_open(client, json_req.length());
  if (err != ESP_OK) {
    ESP_LOGE(__FILENAME__, "Failed to open HTTP connection: %s", esp_err_to_name(err));
    esp_http_client_cleanup(client);
    set_connection_state(SKWSConnectionState::kSKWSDisconnected);
    return;
  }

  int write_len = esp_http_client_write(client, json_req.c_str(), json_req.length());
  if (write_len < 0) {
    ESP_LOGE(__FILENAME__, "Failed to write request body");
    esp_http_client_close(client);
    esp_http_client_cleanup(client);
    set_connection_state(SKWSConnectionState::kSKWSDisconnected);
    return;
  }

  int content_length = esp_http_client_fetch_headers(client);
  int http_code = esp_http_client_get_status_code(client);

  ESP_LOGD(__FILENAME__, "HTTP response: code=%d, content_length=%d", http_code, content_length);

  // Read response body
  String payload;
  char buffer[512];
  int read_len;
  while ((read_len = esp_http_client_read(client, buffer, sizeof(buffer) - 1)) > 0) {
    buffer[read_len] = '\0';
    payload += String(buffer);
    if (payload.length() > 4096) break;
  }
  ESP_LOGI(__FILENAME__, "Response payload (%d bytes): %s",
           payload.length(), payload.c_str());

  esp_http_client_close(client);
  esp_http_client_cleanup(client);

  // Parse JSON response for both 202 and 400 status codes
  deserializeJson(doc, payload.c_str());
  String state = doc["state"].is<const char*>() ? doc["state"].as<String>() : "";
  String href = doc["href"].is<const char*>() ? doc["href"].as<String>() : "";
  String message = doc["message"].is<const char*>() ? doc["message"].as<String>() : "";

  ESP_LOGI(__FILENAME__, "Access request response: http=%d, state=%s, href=%s",
           http_code, state.c_str(), href.c_str());
  if (message.length() > 0) {
    ESP_LOGI(__FILENAME__, "Server message: %s", message.c_str());
  }

  // HTTP 400 with href means "already requested" - we can poll the existing request
  if (http_code == 400 && href.length() > 0 && href.startsWith("/")) {
    ESP_LOGI(__FILENAME__, "Existing request found, polling href: %s", href.c_str());
    polling_href_ = href;
    save();
    delay(5000);
    this->poll_access_request(server_address, server_port, this->polling_href_);
    return;
  }

  // HTTP 202 with href means new request pending
  if (http_code == 202 && href.length() > 0 && href.startsWith("/")) {
    polling_href_ = href;
    save();
    delay(5000);
    this->poll_access_request(server_address, server_port, this->polling_href_);
    return;
  }

  // Can't proceed - disconnect and retry later
  ESP_LOGW(__FILENAME__, "Cannot handle response: http=%d, state=%s", http_code, state.c_str());
  set_connection_state(SKWSConnectionState::kSKWSDisconnected);
}

void SKWSClient::poll_access_request(const String server_address,
                                     const uint16_t server_port,
                                     const String href) {
  ESP_LOGD(__FILENAME__, "Polling SK Server for authentication token");

  String protocol = ssl_enabled_ ? "https://" : "http://";
  String url = protocol + server_address + ":" + server_port + href;

  esp_http_client_config_t config = {};
  config.url = url.c_str();
  config.timeout_ms = 10000;
#ifdef SENSESP_SSL_SUPPORT
  if (ssl_enabled_) {
    config.crt_bundle_attach = tofu_crt_bundle_attach;
    config.skip_cert_common_name_check = true;
  }
#endif

  esp_http_client_handle_t client = esp_http_client_init(&config);
  if (client == nullptr) {
    ESP_LOGE(__FILENAME__, "Failed to initialize HTTP client");
    set_connection_state(SKWSConnectionState::kSKWSDisconnected);
    return;
  }

  // Use streaming API for GET request
  esp_err_t err = esp_http_client_open(client, 0);
  if (err != ESP_OK) {
    ESP_LOGE(__FILENAME__, "Failed to open HTTP connection: %s", esp_err_to_name(err));
    esp_http_client_cleanup(client);
    set_connection_state(SKWSConnectionState::kSKWSDisconnected);
    return;
  }

  int content_length = esp_http_client_fetch_headers(client);
  int http_code = esp_http_client_get_status_code(client);

  // Read response body
  String payload;
  if (content_length > 0 && content_length < 4096) {
    char* buffer = new char[content_length + 1];
    int read_len = esp_http_client_read(client, buffer, content_length);
    buffer[read_len > 0 ? read_len : 0] = '\0';
    payload = String(buffer);
    delete[] buffer;
  } else if (content_length < 0) {
    // Chunked encoding - read in chunks
    char buffer[512];
    int read_len;
    while ((read_len = esp_http_client_read(client, buffer, sizeof(buffer) - 1)) > 0) {
      buffer[read_len] = '\0';
      payload += String(buffer);
      if (payload.length() > 4096) break;
    }
  }

  ESP_LOGD(__FILENAME__, "Poll response: http=%d, payload=%s", http_code, payload.c_str());

  esp_http_client_close(client);
  esp_http_client_cleanup(client);

  if (http_code == 200 || http_code == 202) {
    JsonDocument doc;
    auto error = deserializeJson(doc, payload.c_str());
    if (error) {
      ESP_LOGW(__FILENAME__, "WARNING: Could not deserialize http payload.");
      ESP_LOGW(__FILENAME__, "DeserializationError: %s", error.c_str());
      return;
    }
    String state = doc["state"];
    ESP_LOGD(__FILENAME__, "%s", state.c_str());
    if (state == "PENDING") {
      set_connection_state(SKWSConnectionState::kSKWSDisconnected);
      delay(5000);
      return;
    }
    if (state == "COMPLETED") {
      JsonObject access_req = doc["accessRequest"];
      String permission = access_req["permission"];

      polling_href_ = "";
      save();

      if (permission == "DENIED") {
        ESP_LOGW(__FILENAME__, "Permission denied");
        set_connection_state(SKWSConnectionState::kSKWSDisconnected);
        return;
      }

      if (permission == "APPROVED") {
        ESP_LOGI(__FILENAME__, "Permission granted");
        String token = access_req["token"];
        auth_token_ = token;
        save();
        this->connect_ws(server_address, server_port);
        return;
      }
    }
  } else {
    if (http_code == 500) {
      // this is probably the server barfing due to
      // us polling a non-existing request. Just
      // delete the polling href.
      ESP_LOGD(__FILENAME__, "Got 500, probably a non-existing request.");
      polling_href_ = "";
      save();
      set_connection_state(SKWSConnectionState::kSKWSDisconnected);
      return;
    }
    // any other HTTP status code
    ESP_LOGW(__FILENAME__,
             "Can't handle response %d to pending access request.\n",
             http_code);
    set_connection_state(SKWSConnectionState::kSKWSDisconnected);
    return;
  }
}

void SKWSClient::connect_ws(const String& host, const uint16_t port) {
  set_connection_state(SKWSConnectionState::kSKWSConnecting);

  String protocol = ssl_enabled_ ? "wss" : "ws";
  String path = "/signalk/v1/stream?subscribe=none";
  String url = protocol + "://" + host + ":" + String(port) + path;

  ESP_LOGD(__FILENAME__, "Connecting WebSocket to %s", url.c_str());

  // Configure WebSocket client
  esp_websocket_client_config_t config = {};
  config.uri = url.c_str();
  config.task_stack = 8192;
  config.buffer_size = 1024;

#ifdef SENSESP_SSL_SUPPORT
  if (ssl_enabled_) {
    // Use custom crt_bundle_attach to disable SSL verification
    // This directly configures mbedTLS to skip certificate verification
    config.crt_bundle_attach = tofu_crt_bundle_attach;
    config.skip_cert_common_name_check = true;
  }
#endif

  // Destroy any existing client
  if (client_ != nullptr) {
    esp_websocket_client_stop(client_);
    esp_websocket_client_destroy(client_);
    client_ = nullptr;
  }

  client_ = esp_websocket_client_init(&config);
  if (client_ == nullptr) {
    ESP_LOGE(__FILENAME__, "Failed to initialize WebSocket client");
    set_connection_state(SKWSConnectionState::kSKWSDisconnected);
    return;
  }

  // Register event handler
  esp_websocket_register_events(client_, WEBSOCKET_EVENT_ANY,
                                websocket_event_handler, nullptr);

  // Add authorization header if we have a token
#ifdef SENSESP_SSL_SUPPORT
  if (auth_token_ != NULL_AUTH_TOKEN) {
    esp_websocket_client_append_header(client_, "Authorization",
                                       (String("Bearer ") + auth_token_).c_str());
  }
#endif

  // Start the client
  esp_err_t err = esp_websocket_client_start(client_);
  if (err != ESP_OK) {
    ESP_LOGE(__FILENAME__, "Failed to start WebSocket client: %s",
             esp_err_to_name(err));
    esp_websocket_client_destroy(client_);
    client_ = nullptr;
    set_connection_state(SKWSConnectionState::kSKWSDisconnected);
    return;
  }

  ESP_LOGD(__FILENAME__, "WebSocket client started, waiting for connection...");
}

bool SKWSClient::is_connected() {
  return get_connection_state() == SKWSConnectionState::kSKWSConnected;
}

void SKWSClient::restart() {
  if (client_ != nullptr) {
    esp_websocket_client_stop(client_);
    esp_websocket_client_destroy(client_);
    client_ = nullptr;
  }
  set_connection_state(SKWSConnectionState::kSKWSDisconnected);
}

void SKWSClient::send_delta() {
  String output;
  if (get_connection_state() == SKWSConnectionState::kSKWSConnected) {
    if (sk_delta_queue_->data_available()) {
      sk_delta_queue_->get_delta(output);
      esp_websocket_client_send_text(client_, output.c_str(), output.length(),
                                     portMAX_DELAY);
      // This automatically notifies the observers
      this->delta_tx_tick_producer_.set(1);
    }
  }
}

bool SKWSClient::to_json(JsonObject& root) {
  root["sk_address"] = this->conf_server_address_;
  root["sk_port"] = this->conf_server_port_;
  root["use_mdns"] = this->use_mdns_;

  root["token"] = this->auth_token_;
  root["client_id"] = this->client_id_;
  root["polling_href"] = this->polling_href_;

  root["ssl_enabled"] = this->ssl_enabled_;
  root["tofu_enabled"] = this->tofu_enabled_;
  root["tofu_fingerprint"] = this->tofu_fingerprint_;
  return true;
}

bool SKWSClient::from_json(const JsonObject& config) {
  if (config["sk_address"].is<String>()) {
    this->conf_server_address_ = config["sk_address"].as<String>();
  }
  if (config["sk_port"].is<int>()) {
    this->conf_server_port_ = config["sk_port"].as<int>();
  }
  if (config["use_mdns"].is<bool>()) {
    this->use_mdns_ = config["use_mdns"].as<bool>();
  }
  if (config["token"].is<String>()) {
    this->auth_token_ = config["token"].as<String>();
  }
  if (config["client_id"].is<String>()) {
    this->client_id_ = config["client_id"].as<String>();
  }
  if (config["polling_href"].is<String>()) {
    String href = config["polling_href"].as<String>();
    // Only accept valid hrefs (must start with /)
    this->polling_href_ = href.startsWith("/") ? href : "";
  }

  if (config["ssl_enabled"].is<bool>()) {
    this->ssl_enabled_ = config["ssl_enabled"].as<bool>();
  }
  if (config["tofu_enabled"].is<bool>()) {
    this->tofu_enabled_ = config["tofu_enabled"].as<bool>();
  }
  if (config["tofu_fingerprint"].is<String>()) {
    this->tofu_fingerprint_ = config["tofu_fingerprint"].as<String>();
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
