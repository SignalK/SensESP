#ifndef SENSESP_SRC_SENSESP_SIGNALK_SIGNALK_WS_CLIENT_H_
#define SENSESP_SRC_SENSESP_SIGNALK_SIGNALK_WS_CLIENT_H_

#include "sensesp.h"

#include <algorithm>

#include <ArduinoJson.h>
#include <esp_websocket_client.h>
#include <functional>
#include <list>
#include <set>

#include "sensesp/signalk/signalk_delta_queue.h"
#include "sensesp/system/observablevalue.h"
#include "sensesp/system/task_queue_producer.h"
#include "sensesp/system/valueproducer.h"
#include "sensesp/transforms/integrator.h"
#include "sensesp_base_app.h"

namespace sensesp {

static const char* NULL_AUTH_TOKEN = "";

enum class SKWSConnectionState {
  kSKWSDisconnected,
  kSKWSAuthorizing,
  kSKWSConnecting,
  kSKWSConnected
};

/**
 * @brief The websocket connection to the Signal K server.
 * @see SensESPApp
 */
class SKWSClient : public FileSystemSaveable,
                   virtual public ValueProducer<SKWSConnectionState> {
 public:
  /////////////////////////////////////////////////////////
  // main task methods

  SKWSClient(const String& config_path,
             std::shared_ptr<SKDeltaQueue> sk_delta_queue,
             const String& server_address, uint16_t server_port,
             bool use_mdns = true);

  const String get_server_address() const { return server_address_; }
  uint16_t get_server_port() const { return server_port_; }

  virtual bool to_json(JsonObject& root) override final;
  virtual bool from_json(const JsonObject& config) override final;

  /**
   * Return a delta update ValueProducer that produces the number of sent
   * deltas.
   */
  ValueProducer<int>& get_delta_tx_count_producer() {
    return delta_tx_count_producer_;
  };

  /**
   * @brief Get the delta rx count producer object.
   *
   * @return ValueProducer<int>&
   */
  ValueProducer<int>& get_delta_rx_count_producer() {
    return delta_rx_count_producer_;
  };

  String get_connection_status();

  /////////////////////////////////////////////////////////
  // SKWSClient task methods

  void on_disconnected();
  void on_error();
  void on_connected();
  void on_receive_delta(uint8_t* payload, size_t length);
  void on_receive_updates(JsonDocument& message);
  void on_receive_put(JsonDocument& message);
  void connect();
  void loop();
  bool is_connected();
  void restart();
  bool is_connect_due() const { return millis() >= next_attempt_ms_; }
  void send_delta();

  /**
   * Sends the specified payload to the server over the websocket
   * this client is connected to. If no connection currently exist,
   * the call is safely ignored.
   */
  void sendTXT(String& payload);

  /**
   * @brief Check if SSL/TLS is enabled.
   */
  bool is_ssl_enabled() const { return ssl_enabled_; }

  /**
   * @brief Current Signal K access token, or an empty string if none.
   *
   * Returned as a bare JWT without the "Bearer " prefix. Intended for
   * subsystems that need to piggy-back on the same SK credentials as
   * the main delta websocket — for example, a BLE gateway that also
   * needs to authenticate against signalk-server's provider API but
   * does not have its own access-request flow.
   *
   * Returns "" when SKWSClient has not yet obtained a token (either
   * because the server does not require auth, or because the
   * access-request flow is still in progress). Callers that need
   * auth must handle the empty-string case, typically by deferring
   * their own connect attempts until the main SK websocket is
   * connected (which is a precondition for a valid token anyway).
   */
  String get_auth_token() const { return auth_token_; }

  /**
   * @brief Enable or disable SSL/TLS manually.
   *
   * Note: SSL is normally auto-detected. Use this only if you need
   * to override the auto-detection behavior.
   */
  void set_ssl_enabled(bool enabled) {
    ssl_enabled_ = enabled;
    save();
  }

  /**
   * @brief Check if TOFU certificate verification is enabled.
   */
  bool is_tofu_enabled() const { return tofu_enabled_; }

  /**
   * @brief Enable or disable TOFU certificate verification.
   *
   * When enabled, the server certificate fingerprint is captured on first
   * connection and verified on subsequent connections.
   */
  void set_tofu_enabled(bool enabled) {
    tofu_enabled_ = enabled;
    save();
  }

  /**
   * @brief Check if a TOFU fingerprint is stored.
   */
  bool has_tofu_fingerprint() const { return !tofu_fingerprint_.isEmpty(); }

  /**
   * @brief Get the stored TOFU fingerprint.
   */
  const String& get_tofu_fingerprint() const { return tofu_fingerprint_; }

  /**
   * @brief Reset the stored TOFU certificate fingerprint.
   *
   * Call this when the server certificate has changed legitimately
   * and you want to trust the new certificate.
   */
  void reset_tofu_fingerprint() {
    tofu_fingerprint_ = "";
    save();
  }

  /**
   * @brief Set the TOFU fingerprint (called from verify callback).
   */
  void set_tofu_fingerprint(const String& fingerprint) {
    tofu_fingerprint_ = fingerprint;
    save();
  }

 protected:
  // these are the actually used values
  String server_address_ = "";
  uint16_t server_port_ = 80;
  // these are the hardcoded and/or conf file values
  String conf_server_address_ = "";
  uint16_t conf_server_port_ = 0;
  bool use_mdns_ = true;

  String client_id_ = "";
  String polling_href_ = "";
  String auth_token_ = NULL_AUTH_TOKEN;
  bool server_detected_ = false;
  bool token_test_success_ = false;

  unsigned long next_attempt_ms_ = 0;
  unsigned long connect_interval_ms_ = 2000;

  // SSL/TLS configuration
  bool ssl_enabled_ = false;
  bool tofu_enabled_ = true;  // TOFU enabled by default
  String tofu_fingerprint_ = "";  // SHA256 fingerprint in hex (64 chars)

  TaskQueueProducer<SKWSConnectionState> connection_state_{
      SKWSConnectionState::kSKWSDisconnected, event_loop()};

  /// task_connection_state is used to track the internal task state which might
  /// be out of sync with the published connection state.
  SKWSConnectionState task_connection_state_ =
      SKWSConnectionState::kSKWSDisconnected;

  esp_websocket_client_handle_t client_ = nullptr;
  std::shared_ptr<SKDeltaQueue> sk_delta_queue_;
  /// @brief Emits the number of deltas sent since last report
  TaskQueueProducer<int> delta_tx_tick_producer_{0, event_loop(), 990};
  Integrator<int, int> delta_tx_count_producer_{1, 0, ""};
  Integrator<int, int> delta_rx_count_producer_{1, 0, ""};

  StaticSemaphore_t received_updates_semaphore_buffer_;
  SemaphoreHandle_t received_updates_semaphore_ =
      xSemaphoreCreateRecursiveMutexStatic(&received_updates_semaphore_buffer_);
  std::list<JsonDocument> received_updates_{};

  /////////////////////////////////////////////////////////
  // methods for all tasks

  bool take_received_updates_semaphore(unsigned long int timeout_ms = 0) {
    if (timeout_ms == 0) {
      return xSemaphoreTakeRecursive(received_updates_semaphore_,
                                     portMAX_DELAY) == pdTRUE;
    } else {
      return xSemaphoreTakeRecursive(received_updates_semaphore_,
                                     timeout_ms) == pdTRUE;
    }
  }
  void release_received_updates_semaphore() {
    xSemaphoreGiveRecursive(received_updates_semaphore_);
  }

  /////////////////////////////////////////////////////////
  // main task methods

  void process_received_updates();

  /////////////////////////////////////////////////////////
  // SKWSClient task methods

  void test_token(const String host, const uint16_t port);
  void send_access_request(const String host, const uint16_t port);
  void poll_access_request(const String host, const uint16_t port,
                           const String href);
  void connect_ws(const String& host, const uint16_t port);
  void subscribe_listeners();
  bool get_mdns_service(String& server_address, uint16_t& server_port);
  bool detect_ssl();

  void set_connection_state(SKWSConnectionState state) {
    task_connection_state_ = state;
    connection_state_.set(state);
  }
  SKWSConnectionState get_connection_state() { return task_connection_state_; }

  void schedule_reconnect() {
    next_attempt_ms_ = millis() + connect_interval_ms_ +
                       (esp_random() % (connect_interval_ms_ / 4 + 1));
    connect_interval_ms_ =
        std::min(connect_interval_ms_ * 2, (unsigned long)60000);
  }

  void reset_reconnect_interval() {
    connect_interval_ms_ = 2000;
  }
};

inline const String ConfigSchema(const SKWSClient& obj) {
  return "{\"type\":\"object\",\"properties\":{"
         "\"ssl_enabled\":{\"title\":\"SSL/TLS Enabled\",\"type\":\"boolean\"},"
         "\"tofu_enabled\":{\"title\":\"TOFU Verification\",\"type\":\"boolean\"},"
         "\"tofu_fingerprint\":{\"title\":\"Server Fingerprint\",\"type\":\"string\",\"readOnly\":true}"
         "}}";
}

inline bool ConfigRequiresRestart(const SKWSClient& obj) { return true; }

}  // namespace sensesp

#endif
