#include "sensesp/net/ble/ble_signalk_gateway.h"

#include <HTTPClient.h>

#include "esp_log.h"
#include "sensesp/sensesp_version.h"
#include "sensesp/system/lambda_consumer.h"
#include "sensesp/signalk/signalk_ws_client.h"
#include "sensesp_app.h"

namespace sensesp {

namespace {
constexpr const char* kTag = "ble_gw";

constexpr const char* kAdvertisementsPath =
    "/signalk/v2/api/ble/gateway/advertisements";
constexpr const char* kControlWsPathPrefix =
    "/signalk/v2/api/ble/gateway/ws?token=";

String bytes_to_hex(const std::vector<uint8_t>& data) {
  String out;
  out.reserve(data.size() * 2);
  for (uint8_t b : data) {
    char tmp[3];
    snprintf(tmp, sizeof(tmp), "%02X", b);
    out += tmp;
  }
  return out;
}

}  // namespace

BLESignalKGateway::BLESignalKGateway(std::shared_ptr<BLEProvisioner> ble,
                                     std::shared_ptr<SKWSClient> sk_client,
                                     Config config)
    : ble_provisioner_(std::move(ble)),
      sk_client_(std::move(sk_client)),
      config_(config) {
  pending_ads_mutex_ = xSemaphoreCreateMutex();
  control_ws_mutex_ = xSemaphoreCreateMutex();
  gatt_sessions_mutex_ = xSemaphoreCreateMutex();
  pending_ads_.reserve(config_.max_pending_ads);
}

BLESignalKGateway::~BLESignalKGateway() {
  stop();
  if (pending_ads_mutex_ != nullptr) {
    vSemaphoreDelete(pending_ads_mutex_);
    pending_ads_mutex_ = nullptr;
  }
  if (control_ws_mutex_ != nullptr) {
    vSemaphoreDelete(control_ws_mutex_);
    control_ws_mutex_ = nullptr;
  }
  if (gatt_sessions_mutex_ != nullptr) {
    vSemaphoreDelete(gatt_sessions_mutex_);
    gatt_sessions_mutex_ = nullptr;
  }
}

void BLESignalKGateway::start() {
  if (started_.exchange(true)) {
    return;
  }
  if (!ble_provisioner_ || !sk_client_) {
    ESP_LOGE(kTag, "start() called with null provisioner or SK client");
    started_.store(false);
    return;
  }

  ESP_LOGI(kTag, "Starting BLE SignalK gateway services");

  // Attach an observer to the BLE provisioner's ValueProducer. Each
  // received advertisement triggers on_advertisement(), which reads
  // via get() and buffers into pending_ads_ for the HTTP POST task.
  ble_provisioner_->attach([this]() { this->on_advertisement(); });

  // Hook the SK connection state producer. SKWSClient inherits from
  // ValueProducer<SKWSConnectionState> so we can connect_to() it
  // directly. When SK connects we want to (re)start the control WS;
  // when it disconnects we want to tear it down so it does not keep
  // trying to reach a dead server.
  sk_client_->connect_to(
      new LambdaConsumer<SKWSConnectionState>([this](SKWSConnectionState s) {
        bool connected = (s == SKWSConnectionState::kSKWSConnected);
        sk_connected_.store(connected);
        if (connected && control_ws_ == nullptr) {
          ESP_LOGI(kTag,
                   "SK main WS connected — starting BLE gateway control WS");
          init_control_ws();
        }
      }));

  // If the SK WS is already connected, start the control WS now.
  if (sk_client_->is_connected()) {
    sk_connected_.store(true);
    init_control_ws();
  }

  // Start background POST task.
  post_task_should_run_.store(true);
  xTaskCreate(&BLESignalKGateway::post_task_entry, "ble_gw_post", 8192, this,
              1, &post_task_);

  // Start scanning on the provided BLE provisioner.
  if (!ble_provisioner_->is_scanning()) {
    ble_provisioner_->start_scan();
  }
}

void BLESignalKGateway::stop() {
  if (!started_.exchange(false)) {
    return;
  }

  ESP_LOGI(kTag, "Stopping BLE SignalK gateway services");

  // Stop POST task. The task loop checks post_task_should_run_ on
  // each iteration and exits cleanly when it flips to false.
  post_task_should_run_.store(false);
  // Intentionally NOT calling vTaskDelete here. The task will
  // delete itself by calling vTaskDelete(nullptr) at the end of
  // post_task_loop() so we avoid races with the task still holding
  // the pending_ads_ mutex.
  post_task_ = nullptr;

  destroy_control_ws();

  // The BLE provisioner outlives the gateway — we only stop scanning
  // if we were the ones who started it. For simplicity in this first
  // cut, always stop the scanner on gateway stop. Users who want
  // scanning to continue after stop() can call ble->start_scan()
  // again afterwards.
  if (ble_provisioner_ && ble_provisioner_->is_scanning()) {
    ble_provisioner_->stop_scan();
  }

  // Note: we do not detach the ble_provisioner_ observer. The
  // Observable::attach API does not currently support detaching by
  // functor identity (only by int ID returned from attach), and the
  // cost of letting a dead-gateway callback run once is minimal —
  // on_advertisement() will see started_==false and early-return.
}

void BLESignalKGateway::on_advertisement() {
  if (!started_.load()) {
    return;
  }
  adv_received_count_.fetch_add(1, std::memory_order_relaxed);

  const BLEAdvertisement& ad = ble_provisioner_->get();

  if (xSemaphoreTake(pending_ads_mutex_, pdMS_TO_TICKS(50)) != pdTRUE) {
    // Could not grab the buffer mutex quickly enough — drop this
    // advertisement rather than block the GAP event callback.
    adv_dropped_count_.fetch_add(1, std::memory_order_relaxed);
    return;
  }
  if (pending_ads_.size() >= config_.max_pending_ads) {
    // Buffer full — drop oldest to keep newest. Cheap approximation:
    // drop the first half so we do not ping-pong on every new ad.
    const size_t keep = config_.max_pending_ads / 2;
    pending_ads_.erase(pending_ads_.begin(),
                       pending_ads_.begin() + (pending_ads_.size() - keep));
    adv_dropped_count_.fetch_add(pending_ads_.size() - keep,
                                 std::memory_order_relaxed);
  }
  pending_ads_.push_back(ad);
  xSemaphoreGive(pending_ads_mutex_);
}

void BLESignalKGateway::init_control_ws() {
  if (xSemaphoreTake(control_ws_mutex_, pdMS_TO_TICKS(100)) != pdTRUE) {
    return;
  }

  // Tear down any previous instance first.
  if (control_ws_ != nullptr) {
    esp_websocket_client_stop(control_ws_);
    esp_websocket_client_destroy(control_ws_);
    control_ws_ = nullptr;
  }

  String token = sk_client_->get_auth_token();
  String addr = sk_client_->get_server_address();
  uint16_t port = sk_client_->get_server_port();

  if (addr.length() == 0 || port == 0) {
    ESP_LOGW(kTag, "SK server address not available; deferring control WS");
    xSemaphoreGive(control_ws_mutex_);
    return;
  }

  String url = String("ws://") + addr + ":" + String(port) +
               kControlWsPathPrefix + token;

  ESP_LOGI(kTag, "Connecting control WS to ws://%s:%u%s?token=<redacted>",
           addr.c_str(), static_cast<unsigned>(port),
           "/signalk/v2/api/ble/gateway/ws");

  esp_websocket_client_config_t cfg = {};
  cfg.uri = url.c_str();
  cfg.task_stack = 4096;
  cfg.buffer_size = 1024;
  cfg.reconnect_timeout_ms = 5000;
  cfg.network_timeout_ms = 10000;

  control_ws_ = esp_websocket_client_init(&cfg);
  if (control_ws_ == nullptr) {
    ESP_LOGE(kTag, "esp_websocket_client_init failed");
    xSemaphoreGive(control_ws_mutex_);
    return;
  }

  esp_websocket_register_events(control_ws_, WEBSOCKET_EVENT_ANY,
                                &BLESignalKGateway::control_ws_event_trampoline,
                                this);
  esp_websocket_client_start(control_ws_);

  xSemaphoreGive(control_ws_mutex_);
}

void BLESignalKGateway::destroy_control_ws() {
  if (xSemaphoreTake(control_ws_mutex_, pdMS_TO_TICKS(200)) != pdTRUE) {
    return;
  }
  if (control_ws_ != nullptr) {
    esp_websocket_client_stop(control_ws_);
    esp_websocket_client_destroy(control_ws_);
    control_ws_ = nullptr;
  }
  ws_connected_.store(false);
  xSemaphoreGive(control_ws_mutex_);
}

void BLESignalKGateway::send_hello() {
  if (!ws_connected_.load() || control_ws_ == nullptr) {
    return;
  }
  JsonDocument doc;
  doc["type"] = "hello";
  doc["gateway_id"] = SensESPBaseApp::get_hostname();
  doc["firmware"] = config_.firmware_version.length() > 0
                        ? config_.firmware_version
                        : String(kSensESPVersion);
  doc["max_gatt_connections"] =
      ble_provisioner_ ? ble_provisioner_->max_gatt_connections()
                       : config_.max_gatt_sessions;
  doc["active_gatt_connections"] =
      ble_provisioner_ ? ble_provisioner_->active_gatt_connections() : 0;
  doc["mac"] = ble_provisioner_ ? ble_provisioner_->mac_address() : String("");
  doc["hostname"] = SensESPBaseApp::get_hostname();

  // IP address from the network provisioner so the server can show
  // it in the BLE manager UI.
  auto app = SensESPApp::get();
  if (app) {
    auto provisioner = app->get_network_provisioner();
    if (provisioner) {
      doc["ip_address"] = provisioner->local_ip().toString();
    }
  }

  String msg;
  serializeJson(doc, msg);
  esp_websocket_client_send_text(control_ws_, msg.c_str(), msg.length(),
                                 portMAX_DELAY);
  ESP_LOGI(kTag, "Sent hello");
}

void BLESignalKGateway::send_status() {
  if (!ws_connected_.load() || control_ws_ == nullptr) {
    return;
  }
  JsonDocument doc;
  doc["type"] = "status";
  doc["gateway_id"] = SensESPBaseApp::get_hostname();
  doc["uptime"] = millis() / 1000;
  doc["free_heap"] = ESP.getFreeHeap();
  doc["active_gatt_connections"] =
      ble_provisioner_ ? ble_provisioner_->active_gatt_connections() : 0;
  doc["max_gatt_connections"] =
      ble_provisioner_ ? ble_provisioner_->max_gatt_connections()
                       : config_.max_gatt_sessions;
  doc["scan_hits"] = ble_provisioner_ ? ble_provisioner_->scan_hit_count() : 0;
  doc["post_success"] = http_post_success_.load();
  doc["post_fail"] = http_post_fail_.load();

  String msg;
  serializeJson(doc, msg);
  esp_websocket_client_send_text(control_ws_, msg.c_str(), msg.length(),
                                 portMAX_DELAY);
}

void BLESignalKGateway::handle_control_ws_message(uint8_t* payload,
                                                  size_t length) {
  JsonDocument doc;
  DeserializationError err = deserializeJson(doc, payload, length);
  if (err) {
    ESP_LOGW(kTag, "Control WS JSON parse error: %s", err.c_str());
    return;
  }
  const char* type = doc["type"];
  if (type == nullptr) {
    return;
  }
  if (strcmp(type, "hello_ack") == 0) {
    ESP_LOGI(kTag, "Hello acknowledged by server");
  } else if (strcmp(type, "gatt_subscribe") == 0) {
    handle_gatt_subscribe(doc);
  } else if (strcmp(type, "gatt_write") == 0) {
    handle_gatt_write(doc);
  } else if (strcmp(type, "gatt_close") == 0) {
    handle_gatt_close(doc);
  } else {
    ESP_LOGD(kTag, "Unhandled control message type: %s", type);
  }
}

void BLESignalKGateway::post_pending_advertisements() {
  if (!sk_connected_.load()) {
    return;
  }

  String token = sk_client_->get_auth_token();
  String addr = sk_client_->get_server_address();
  uint16_t port = sk_client_->get_server_port();
  if (addr.length() == 0 || port == 0) {
    return;
  }
  // An empty token is valid when the server has security disabled
  // (signalk-server returns 404 on access-request and SKWSClient
  // proceeds without a token). In that case we still POST but
  // without the Authorization header.

  // Drain the pending buffer under the mutex.
  std::vector<BLEAdvertisement> to_post;
  if (xSemaphoreTake(pending_ads_mutex_, pdMS_TO_TICKS(200)) != pdTRUE) {
    return;
  }
  to_post.swap(pending_ads_);
  xSemaphoreGive(pending_ads_mutex_);

  if (to_post.empty()) {
    return;
  }

  // Build the JSON batch.
  JsonDocument doc;
  doc["gateway_id"] = SensESPBaseApp::get_hostname();
  JsonArray devices = doc["devices"].to<JsonArray>();
  for (const auto& ad : to_post) {
    JsonObject dev = devices.add<JsonObject>();
    dev["mac"] = ad.address;
    dev["rssi"] = ad.rssi;
    if (ad.name.length() > 0) {
      dev["name"] = ad.name;
    }
    if (!ad.adv_data.empty()) {
      dev["adv_data"] = bytes_to_hex(ad.adv_data);
    }
  }

  String body;
  serializeJson(doc, body);

  String url = String("http://") + addr + ":" + String(port) +
               kAdvertisementsPath;

  HTTPClient http;
  http.begin(url);
  http.addHeader("Content-Type", "application/json");
  if (token.length() > 0) {
    http.addHeader("Authorization", String("Bearer ") + token);
  }
  http.addHeader("Connection", "close");
  http.setTimeout(3000);

  int code = http.POST(body);
  if (code == 200) {
    adv_posted_count_.fetch_add(to_post.size(), std::memory_order_relaxed);
    http_post_success_.fetch_add(1, std::memory_order_relaxed);
    ESP_LOGI(kTag, "POST: forwarded %u adv, heap=%u",
             static_cast<unsigned>(to_post.size()),
             static_cast<unsigned>(ESP.getFreeHeap()));
  } else if (code == 401 || code == 403) {
    http_post_fail_.fetch_add(1, std::memory_order_relaxed);
    ESP_LOGW(kTag,
             "POST: auth rejected (HTTP %d) — restarting main SK connection",
             code);
    http.end();
    sk_client_->restart();
    return;
  } else {
    http_post_fail_.fetch_add(1, std::memory_order_relaxed);
    ESP_LOGW(kTag, "POST failed: HTTP %d, heap=%u", code,
             static_cast<unsigned>(ESP.getFreeHeap()));
  }
  http.end();
}

// -----------------------------------------------------------------
// GATT session management
// -----------------------------------------------------------------

void BLESignalKGateway::send_control_json(JsonDocument& doc) {
  if (!ws_connected_.load() || control_ws_ == nullptr) return;
  String msg;
  serializeJson(doc, msg);
  esp_websocket_client_send_text(control_ws_, msg.c_str(), msg.length(),
                                 portMAX_DELAY);
}

namespace {
// Hex-decode a string like "0102ab" into bytes.
std::vector<uint8_t> hex_decode(const char* hex) {
  std::vector<uint8_t> out;
  if (!hex) return out;
  size_t len = strlen(hex);
  out.reserve(len / 2);
  for (size_t i = 0; i + 1 < len; i += 2) {
    char buf[3] = {hex[i], hex[i + 1], 0};
    out.push_back(static_cast<uint8_t>(strtoul(buf, nullptr, 16)));
  }
  return out;
}

String hex_encode(const uint8_t* data, size_t len) {
  String out;
  out.reserve(len * 2);
  for (size_t i = 0; i < len; i++) {
    char tmp[3];
    snprintf(tmp, sizeof(tmp), "%02x", data[i]);
    out += tmp;
  }
  return out;
}
}  // namespace

// Context struct passed to FreeRTOS timer callbacks via pvTimerGetTimerID().
struct GATTTimerContext {
  BLESignalKGateway* gateway;
  String session_id;
  String char_uuid;
  std::vector<uint8_t> write_data;  // Only for periodic_write timers.
};

void BLESignalKGateway::handle_gatt_subscribe(JsonDocument& doc) {
  const char* session_id = doc["session_id"];
  const char* mac = doc["mac"];
  const char* service = doc["service"];

  if (!session_id || !mac || !service) {
    ESP_LOGW(kTag, "gatt_subscribe: missing required fields");
    return;
  }
  if (!ble_provisioner_) {
    ESP_LOGE(kTag, "gatt_subscribe: no BLE provisioner");
    return;
  }

  int max = ble_provisioner_->max_gatt_connections();
  if (max <= 0) {
    JsonDocument err;
    err["type"] = "gatt_error";
    err["session_id"] = session_id;
    err["error"] = "GATT not supported on this gateway";
    send_control_json(err);
    return;
  }

  if (xSemaphoreTake(gatt_sessions_mutex_, pdMS_TO_TICKS(100)) != pdTRUE) {
    return;
  }
  if (static_cast<int>(gatt_sessions_.size()) >= max) {
    xSemaphoreGive(gatt_sessions_mutex_);
    JsonDocument err;
    err["type"] = "gatt_error";
    err["session_id"] = session_id;
    err["error"] = "No GATT slots available";
    send_control_json(err);
    return;
  }

  auto session = std::make_unique<GATTSession>();
  session->session_id = session_id;
  session->mac = mac;
  session->service_uuid = service;

  // Parse notify array.
  JsonArray notify = doc["notify"];
  for (JsonVariant v : notify) {
    session->notify_uuids.push_back(String(v.as<const char*>()));
  }

  // Parse init writes.
  JsonArray init = doc["init"];
  for (JsonObject obj : init) {
    InitWrite iw;
    iw.char_uuid = String(obj["uuid"].as<const char*>());
    iw.data = hex_decode(obj["data"].as<const char*>());
    session->init_writes.push_back(std::move(iw));
  }

  // Parse poll descriptors.
  JsonArray poll = doc["poll"];
  for (JsonObject obj : poll) {
    PollDescriptor pd;
    pd.char_uuid = String(obj["uuid"].as<const char*>());
    pd.interval_ms = obj["interval_ms"];
    session->polls.push_back(std::move(pd));
  }

  // Parse periodic write descriptors.
  JsonArray pw = doc["periodic_write"];
  for (JsonObject obj : pw) {
    PeriodicWriteDescriptor pwd;
    pwd.char_uuid = String(obj["uuid"].as<const char*>());
    pwd.data = hex_decode(obj["data"].as<const char*>());
    pwd.interval_ms = obj["interval_ms"];
    session->periodic_writes.push_back(std::move(pwd));
  }

  String sid = session->session_id;
  GATTSession* raw_session = session.get();
  gatt_sessions_[sid] = std::move(session);
  xSemaphoreGive(gatt_sessions_mutex_);

  // Stop scanning during connection establishment.
  scan_suppressed_.store(true);
  ble_provisioner_->stop_scan();

  ESP_LOGI(kTag, "GATT subscribe: session=%s mac=%s service=%s",
           sid.c_str(), mac, service);

  GATTConnectionCallbacks callbacks;

  callbacks.on_connected = [this, sid]() {
    ESP_LOGI(kTag, "GATT connected: session=%s", sid.c_str());
    if (xSemaphoreTake(gatt_sessions_mutex_, pdMS_TO_TICKS(100)) != pdTRUE)
      return;
    auto it = gatt_sessions_.find(sid);
    if (it == gatt_sessions_.end()) {
      xSemaphoreGive(gatt_sessions_mutex_);
      return;
    }
    GATTSession* s = it->second.get();
    s->state = GATTSessionState::kInitializing;
    xSemaphoreGive(gatt_sessions_mutex_);
    gatt_run_init_writes(s);
  };

  callbacks.on_disconnected = [this, sid](const String& reason) {
    ESP_LOGW(kTag, "GATT disconnected: session=%s reason=%s",
             sid.c_str(), reason.c_str());
    JsonDocument msg;
    msg["type"] = "gatt_disconnected";
    msg["session_id"] = sid;
    msg["reason"] = reason;
    send_control_json(msg);
    gatt_cleanup_session(sid);
  };

  callbacks.on_notify = [this, sid](const String& char_uuid,
                                    const uint8_t* data, size_t len) {
    JsonDocument msg;
    msg["type"] = "gatt_data";
    msg["session_id"] = sid;
    msg["uuid"] = char_uuid;
    msg["data"] = hex_encode(data, len);
    send_control_json(msg);
  };

  callbacks.on_read = [this, sid](const String& char_uuid,
                                  const uint8_t* data, size_t len) {
    JsonDocument msg;
    msg["type"] = "gatt_data";
    msg["session_id"] = sid;
    msg["uuid"] = char_uuid;
    msg["data"] = hex_encode(data, len);
    send_control_json(msg);
  };

  callbacks.on_write_complete = [this, sid](const String& char_uuid,
                                            bool success) {
    if (!success) {
      ESP_LOGW(kTag, "GATT write failed: session=%s char=%s",
               sid.c_str(), char_uuid.c_str());
    }
    // Advance init write sequence if in init phase.
    if (xSemaphoreTake(gatt_sessions_mutex_, pdMS_TO_TICKS(50)) != pdTRUE)
      return;
    auto it = gatt_sessions_.find(sid);
    if (it != gatt_sessions_.end() &&
        it->second->state == GATTSessionState::kInitializing) {
      GATTSession* s = it->second.get();
      s->init_write_index++;
      xSemaphoreGive(gatt_sessions_mutex_);
      gatt_run_init_writes(s);
    } else {
      xSemaphoreGive(gatt_sessions_mutex_);
    }
  };

  callbacks.on_error = [this, sid](const String& error) {
    ESP_LOGE(kTag, "GATT error: session=%s: %s", sid.c_str(), error.c_str());
    JsonDocument msg;
    msg["type"] = "gatt_error";
    msg["session_id"] = sid;
    msg["error"] = error;
    send_control_json(msg);
    gatt_cleanup_session(sid);
  };

  int handle = ble_provisioner_->gatt_connect(
      String(mac), 0, String(service), std::move(callbacks));
  if (handle < 0) {
    JsonDocument err;
    err["type"] = "gatt_error";
    err["session_id"] = sid;
    err["error"] = "gatt_connect failed";
    send_control_json(err);
    gatt_cleanup_session(sid);
    return;
  }
  raw_session->conn_handle = handle;
}

void BLESignalKGateway::gatt_run_init_writes(GATTSession* session) {
  if (session->init_write_index >= session->init_writes.size()) {
    // All init writes done — move to subscribing.
    session->state = GATTSessionState::kSubscribing;
    session->subscribe_index = 0;
    gatt_run_subscribes(session);
    return;
  }
  auto& iw = session->init_writes[session->init_write_index];
  ESP_LOGI(kTag, "GATT init write %u/%u: %s",
           (unsigned)(session->init_write_index + 1),
           (unsigned)session->init_writes.size(),
           iw.char_uuid.c_str());
  ble_provisioner_->gatt_write(session->conn_handle, iw.char_uuid,
                               iw.data.data(), iw.data.size());
  // on_write_complete callback will advance init_write_index and call us again.
}

void BLESignalKGateway::gatt_run_subscribes(GATTSession* session) {
  if (session->subscribe_index >= session->notify_uuids.size()) {
    // All subscriptions done — session is active.
    session->state = GATTSessionState::kActive;
    gatt_start_timers(session);

    // Resume scanning.
    scan_suppressed_.store(false);
    if (ble_provisioner_ && !ble_provisioner_->is_scanning()) {
      ble_provisioner_->start_scan();
    }

    // Notify server.
    JsonDocument msg;
    msg["type"] = "gatt_connected";
    msg["session_id"] = session->session_id;
    msg["mac"] = session->mac;
    send_control_json(msg);

    ESP_LOGI(kTag, "GATT session %s fully active",
             session->session_id.c_str());
    return;
  }
  const String& uuid = session->notify_uuids[session->subscribe_index];
  ESP_LOGI(kTag, "GATT subscribe notify %u/%u: %s",
           (unsigned)(session->subscribe_index + 1),
           (unsigned)session->notify_uuids.size(), uuid.c_str());
  ble_provisioner_->gatt_subscribe_notify(session->conn_handle, uuid);
  session->subscribe_index++;
  // Notifications are async — just proceed to the next one immediately.
  // The Bluedroid REG_FOR_NOTIFY callback is fire-and-forget.
  gatt_run_subscribes(session);
}

void BLESignalKGateway::gatt_start_timers(GATTSession* session) {
  // Poll timers — periodically read a characteristic.
  for (auto& pd : session->polls) {
    auto* ctx = new GATTTimerContext{this, session->session_id,
                                    pd.char_uuid, {}};
    TimerHandle_t t = xTimerCreate(
        "gatt_poll", pdMS_TO_TICKS(pd.interval_ms), pdTRUE, ctx,
        &BLESignalKGateway::poll_timer_cb);
    if (t) {
      xTimerStart(t, 0);
      session->timers.push_back(t);
    }
  }

  // Periodic write timers.
  for (auto& pw : session->periodic_writes) {
    auto* ctx = new GATTTimerContext{this, session->session_id,
                                    pw.char_uuid, pw.data};
    TimerHandle_t t = xTimerCreate(
        "gatt_pw", pdMS_TO_TICKS(pw.interval_ms), pdTRUE, ctx,
        &BLESignalKGateway::periodic_write_timer_cb);
    if (t) {
      xTimerStart(t, 0);
      session->timers.push_back(t);
    }
  }
}

void BLESignalKGateway::poll_timer_cb(TimerHandle_t timer) {
  auto* ctx = static_cast<GATTTimerContext*>(pvTimerGetTimerID(timer));
  if (!ctx || !ctx->gateway || !ctx->gateway->ble_provisioner_) return;

  if (xSemaphoreTake(ctx->gateway->gatt_sessions_mutex_,
                     pdMS_TO_TICKS(50)) != pdTRUE)
    return;
  auto it = ctx->gateway->gatt_sessions_.find(ctx->session_id);
  if (it == ctx->gateway->gatt_sessions_.end() ||
      it->second->state != GATTSessionState::kActive) {
    xSemaphoreGive(ctx->gateway->gatt_sessions_mutex_);
    return;
  }
  int handle = it->second->conn_handle;
  xSemaphoreGive(ctx->gateway->gatt_sessions_mutex_);

  ctx->gateway->ble_provisioner_->gatt_read(handle, ctx->char_uuid);
}

void BLESignalKGateway::periodic_write_timer_cb(TimerHandle_t timer) {
  auto* ctx = static_cast<GATTTimerContext*>(pvTimerGetTimerID(timer));
  if (!ctx || !ctx->gateway || !ctx->gateway->ble_provisioner_) return;

  if (xSemaphoreTake(ctx->gateway->gatt_sessions_mutex_,
                     pdMS_TO_TICKS(50)) != pdTRUE)
    return;
  auto it = ctx->gateway->gatt_sessions_.find(ctx->session_id);
  if (it == ctx->gateway->gatt_sessions_.end() ||
      it->second->state != GATTSessionState::kActive) {
    xSemaphoreGive(ctx->gateway->gatt_sessions_mutex_);
    return;
  }
  int handle = it->second->conn_handle;
  xSemaphoreGive(ctx->gateway->gatt_sessions_mutex_);

  ctx->gateway->ble_provisioner_->gatt_write(
      handle, ctx->char_uuid, ctx->write_data.data(),
      ctx->write_data.size());
}

void BLESignalKGateway::handle_gatt_write(JsonDocument& doc) {
  const char* session_id = doc["session_id"];
  const char* uuid = doc["uuid"];
  const char* data_hex = doc["data"];
  if (!session_id || !uuid || !data_hex) return;

  if (xSemaphoreTake(gatt_sessions_mutex_, pdMS_TO_TICKS(100)) != pdTRUE)
    return;
  auto it = gatt_sessions_.find(String(session_id));
  if (it == gatt_sessions_.end()) {
    xSemaphoreGive(gatt_sessions_mutex_);
    return;
  }
  int handle = it->second->conn_handle;
  xSemaphoreGive(gatt_sessions_mutex_);

  auto data = hex_decode(data_hex);
  ble_provisioner_->gatt_write(handle, String(uuid), data.data(),
                               data.size());
}

void BLESignalKGateway::handle_gatt_close(JsonDocument& doc) {
  const char* session_id = doc["session_id"];
  if (!session_id) return;

  String sid(session_id);
  ESP_LOGI(kTag, "GATT close: session=%s", sid.c_str());

  if (xSemaphoreTake(gatt_sessions_mutex_, pdMS_TO_TICKS(100)) != pdTRUE)
    return;
  auto it = gatt_sessions_.find(sid);
  if (it == gatt_sessions_.end()) {
    xSemaphoreGive(gatt_sessions_mutex_);
    return;
  }
  int handle = it->second->conn_handle;
  it->second->state = GATTSessionState::kDisconnecting;
  xSemaphoreGive(gatt_sessions_mutex_);

  if (ble_provisioner_) {
    ble_provisioner_->gatt_disconnect(handle);
  }

  JsonDocument msg;
  msg["type"] = "gatt_disconnected";
  msg["session_id"] = sid;
  msg["reason"] = "closed_by_server";
  send_control_json(msg);

  gatt_cleanup_session(sid);
}

void BLESignalKGateway::gatt_cleanup_session(const String& session_id) {
  if (xSemaphoreTake(gatt_sessions_mutex_, pdMS_TO_TICKS(200)) != pdTRUE)
    return;
  auto it = gatt_sessions_.find(session_id);
  if (it != gatt_sessions_.end()) {
    // Delete timers and free timer contexts.
    for (auto t : it->second->timers) {
      auto* ctx = static_cast<GATTTimerContext*>(pvTimerGetTimerID(t));
      xTimerStop(t, 0);
      xTimerDelete(t, 0);
      delete ctx;
    }
    it->second->timers.clear();
    gatt_sessions_.erase(it);
  }
  xSemaphoreGive(gatt_sessions_mutex_);

  // Resume scanning if no GATT sessions remain.
  if (gatt_sessions_.empty()) {
    scan_suppressed_.store(false);
    if (ble_provisioner_ && !ble_provisioner_->is_scanning()) {
      ble_provisioner_->start_scan();
    }
  }
}

// -----------------------------------------------------------------

void BLESignalKGateway::post_task_entry(void* arg) {
  static_cast<BLESignalKGateway*>(arg)->post_task_loop();
}

void BLESignalKGateway::post_task_loop() {
  unsigned long last_status_ms = 0;
  uint32_t last_known_hits = 0;
  unsigned long last_hit_change_ms = millis();
  static constexpr unsigned long kScanWatchdogMs = 30000;
  int consecutive_restarts = 0;

  while (post_task_should_run_.load()) {
    vTaskDelay(pdMS_TO_TICKS(config_.post_interval_ms));
    post_pending_advertisements();

    unsigned long now = millis();

    // Scan watchdog: if no new advertisements have arrived in
    // kScanWatchdogMs, escalate recovery:
    //   Level 1: scan stop/start (HCI command level)
    //   Level 2: BT controller reset via RPC to C6
    //   Level 3: GPIO hard-reset of the C6 chip (power-cycles it)
    //   Level 4: full ESP.restart()
    if (ble_provisioner_) {
      uint32_t current_hits = ble_provisioner_->scan_hit_count();
      if (current_hits != last_known_hits) {
        last_known_hits = current_hits;
        last_hit_change_ms = now;
        consecutive_restarts = 0;
      } else if (now - last_hit_change_ms > kScanWatchdogMs &&
                 ble_provisioner_->is_scanning() &&
                 !scan_suppressed_.load()) {
        consecutive_restarts++;
        if (consecutive_restarts == 1) {
          // Level 1: simple scan restart (HCI level).
          ESP_LOGW(kTag,
                   "Scan watchdog: no hits in %lu ms — restarting scan "
                   "(level 1)",
                   now - last_hit_change_ms);
          ble_provisioner_->stop_scan();
          vTaskDelay(pdMS_TO_TICKS(500));
          ble_provisioner_->start_scan();
        } else if (consecutive_restarts == 2) {
          // Level 2: BT controller reset via RPC to C6.
          ESP_LOGW(kTag,
                   "Scan watchdog: level 1 failed — RPC BT controller "
                   "reset (level 2)");
          ble_provisioner_->stop_scan();
          vTaskDelay(pdMS_TO_TICKS(200));
          if (ble_provisioner_->reset_bt_controller()) {
            vTaskDelay(pdMS_TO_TICKS(500));
            ble_provisioner_->start_scan();
          } else {
            ESP_LOGE(kTag, "BT controller RPC reset failed");
          }
        } else if (consecutive_restarts == 3) {
          // Level 3: GPIO hard-reset of the C6 chip. This power-
          // cycles the entire C6, clearing whatever stuck HCI state
          // is silently eating advertising reports.
          ESP_LOGW(kTag,
                   "Scan watchdog: level 2 failed — GPIO hard-reset "
                   "of C6 (level 3)");
          if (ble_provisioner_->hard_reset_c6()) {
            vTaskDelay(pdMS_TO_TICKS(500));
            ble_provisioner_->start_scan();
          } else {
            ESP_LOGE(kTag, "C6 GPIO hard-reset failed");
          }
        } else {
          // Level 4: reboot the entire device. The C6 is in a state
          // that no software recovery can fix.
          ESP_LOGE(kTag,
                   "Scan watchdog: %d consecutive failures — rebooting "
                   "(level 4)",
                   consecutive_restarts);
          vTaskDelay(pdMS_TO_TICKS(1000));
          ESP.restart();
        }
        last_hit_change_ms = now;
      }
    }

    if (now - last_status_ms >= config_.status_interval_ms) {
      send_status();
      last_status_ms = now;
    }
  }
  // Self-delete so we do not leak a FreeRTOS task handle after stop().
  vTaskDelete(nullptr);
}

void BLESignalKGateway::control_ws_event_trampoline(void* handler_args,
                                                    esp_event_base_t /*base*/,
                                                    int32_t event_id,
                                                    void* event_data) {
  static_cast<BLESignalKGateway*>(handler_args)
      ->handle_control_ws_event(event_id, event_data);
}

void BLESignalKGateway::handle_control_ws_event(int32_t event_id,
                                                void* event_data) {
  auto* data = static_cast<esp_websocket_event_data_t*>(event_data);
  switch (event_id) {
    case WEBSOCKET_EVENT_CONNECTED:
      ESP_LOGI(kTag, "Control WS connected");
      ws_connected_.store(true);
      ws_connected_count_.fetch_add(1, std::memory_order_relaxed);
      send_hello();
      break;
    case WEBSOCKET_EVENT_DISCONNECTED:
      ESP_LOGW(kTag, "Control WS disconnected — will reconnect");
      ws_connected_.store(false);
      break;
    case WEBSOCKET_EVENT_CLOSED:
      ESP_LOGW(kTag, "Control WS closed — will reconnect");
      ws_connected_.store(false);
      break;
    case WEBSOCKET_EVENT_DATA:
      if (data->op_code == 0x01 && data->data_len > 0) {
        // data_ptr is const char*; cast away the const for the JSON
        // parser which does not mutate the buffer.
        handle_control_ws_message(
            reinterpret_cast<uint8_t*>(const_cast<char*>(data->data_ptr)),
            data->data_len);
      }
      break;
    case WEBSOCKET_EVENT_ERROR:
      ESP_LOGW(kTag, "Control WS error");
      ws_connected_.store(false);
      break;
    default:
      break;
  }
}

}  // namespace sensesp
