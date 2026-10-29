#ifndef SENSESP_SRC_SENSESP_SIGNALK_SIGNALK_WS_CLIENT_H_
#define SENSESP_SRC_SENSESP_SIGNALK_SIGNALK_WS_CLIENT_H_

#include "sensesp.h"

#include <algorithm>

#include <ArduinoJson.h>
#include <esp_websocket_client.h>
#include <atomic>
#include <functional>
#include <list>
#include <set>

#include "sensesp/signalk/signalk_delta_queue.h"
#include "sensesp/signalk/signalk_ws_connect_due.h"
#include "sensesp/system/observablevalue.h"
#include "sensesp/system/task_queue_producer.h"
#include "sensesp/system/valueproducer.h"
#include "sensesp/transforms/integrator.h"
#include "sensesp_base_app.h"

// Maximum number of received value/put deltas buffered for processing on the
// main task. Oldest entries are dropped when the buffer is full.
#ifndef SENSESP_MAX_RECEIVED_VALUE_UPDATES
#define SENSESP_MAX_RECEIVED_VALUE_UPDATES 20
#endif

// Maximum number of received meta deltas buffered for processing on the main
// task, budgeted independently of value deltas so a metadata burst (~one entry
// per subscribed path at subscribe time when sendMeta=all is enabled) cannot
// evict pending values, and vice versa. Override per board via build_flags if
// you subscribe many paths and need the full burst delivered (more paths =>
// more buffered meta => more RAM).
#ifndef SENSESP_MAX_RECEIVED_META_UPDATES
#define SENSESP_MAX_RECEIVED_META_UPDATES 20
#endif

// Signal K WebSocket buffer size in bytes, allocated by esp_websocket_client for
// both the tx and rx buffer. send_delta drops any delta longer than this rather
// than let esp_websocket_client split it across non-blocking writes and abort
// the connection (see signalk_ws_delta_size.h). Override per board via
// build_flags to hold the largest delta a device sends (e.g. a GNSS receiver's
// full-sky satellitesInView); kept small by default for memory-constrained
// boards.
#ifndef SENSESP_SK_WS_BUFFER_SIZE
#define SENSESP_SK_WS_BUFFER_SIZE 1024
#endif

namespace sensesp {

static const char* NULL_AUTH_TOKEN = "";

// HTTP status returned on a WebSocket upgrade when the Signal K server rejects
// the auth token.
static constexpr int kHttpUnauthorized = 401;

/**
 * @brief Decide whether a failed WebSocket upgrade should clear the auth token.
 *
 * A 401 on the upgrade means the server rejected the presented token, and the
 * device must re-request access to recover; the caller acts on it only when a
 * token was actually sent. The decision is transport-independent: over
 * plaintext the status is unauthenticated, so anything that can answer for the
 * server (an on-path attacker, or any same-network device when mDNS discovery
 * is enabled) can inject a 401 to force a token wipe and a manual re-approval.
 * But gating on TLS leaves a plaintext deployment with no recovery route at
 * all, locked out indefinitely on a stale token. Recovering on its own is the
 * better failure mode. TLS deployments with a pinned TOFU anchor are
 * unaffected, since their 401 is authenticated; with TOFU disabled or before
 * an anchor is pinned, the status is as spoofable as plaintext.
 */
bool should_clear_token_on_status(int handshake_status);

enum class SKWSConnectionState {
  kSKWSDisconnected,
  kSKWSAuthorizing,
  kSKWSConnecting,
  kSKWSConnected,
  // TLS certificate pinning rejected the server certificate. Distinct from a
  // plain disconnect so the UI can tell a cert problem from a network drop.
  kSKWSCertificateError
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
  // SK websocket connection methods

  void on_disconnected();
  // handshake_status is the HTTP status of a failed WebSocket upgrade (0 if not
  // applicable); a 401 means the auth token was rejected.
  void on_error(int handshake_status);
  void on_connected();
  void on_receive_delta(uint8_t* payload, size_t length);
  void on_receive_updates(JsonDocument& message);
  void on_receive_put(JsonDocument& message);
  void connect();
  void loop();
  bool is_connected();
  /// @brief Drop the current connection (detached, non-blocking teardown) and
  /// let the reconnect path rebuild it.
  ///
  /// @warning Must run on the event-loop (SK connection) context only. It does
  /// not check auth_job_running_, so calling it concurrently with a running
  /// connect worker could interleave its client_.exchange(nullptr) with the
  /// worker's client_.store(h) and orphan a live client (leak + dropped events).
  /// Has no callers today. See #1033.
  void restart();
  bool is_connect_due() const {
    return sk_connect_due(millis(), last_attempt_ms_, attempt_wait_ms_);
  }
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
   * @brief Whether the WS subscribes with `sendMeta=all`.
   *
   * When true, the signalk-server pushes metadata deltas (units,
   * zones, displayName, displayUnits, etc.) over the same WebSocket
   * connection as value deltas, so consumers don't have to make
   * separate REST `/meta` requests per path.
   *
   * Defaults to true. When enabled, meta-deltas can be consumed via
   * `SKMetadataListener`. Disable only if you have a constrained client
   * that doesn't want meta-deltas at all (they are otherwise ignored by
   * `SKValueListener` since meta updates carry no `values` array).
   */
  bool is_send_meta_enabled() const { return send_meta_enabled_; }
  void set_send_meta_enabled(bool enabled) {
    send_meta_enabled_ = enabled;
    save();
  }

  /**
   * @brief Check if TOFU certificate verification is enabled.
   */
  bool is_tofu_enabled() const { return tofu_enabled_; }

  /**
   * @brief Enable or disable TOFU certificate verification.
   *
   * When enabled, the issuing CA (or, for servers that present no CA, the leaf
   * certificate) is captured on first connection and verified on subsequent
   * connections.
   */
  void set_tofu_enabled(bool enabled) {
    tofu_enabled_ = enabled;
    save();
  }

  /// @brief True if any trust anchor (pinned CA or leaf fingerprint) is stored.
  bool has_tofu_anchor() const {
    return !tofu_ca_pem_.isEmpty() || !tofu_fingerprint_.isEmpty();
  }

  /// @brief True if a pinned CA certificate is stored (CA-anchor mode).
  bool has_tofu_ca() const { return !tofu_ca_pem_.isEmpty(); }
  const String& get_tofu_ca() const { return tofu_ca_pem_; }

  /// @brief True if a pinned leaf fingerprint is stored (leaf-fingerprint mode).
  bool has_tofu_fingerprint() const { return !tofu_fingerprint_.isEmpty(); }
  const String& get_tofu_fingerprint() const { return tofu_fingerprint_; }

  /// @brief Identity (CN) of whatever is pinned, and whether it is a CA.
  const String& get_tofu_pin_cn() const { return tofu_pin_cn_; }
  bool is_tofu_pin_ca() const { return tofu_pin_is_ca_; }

  /// @brief TOFU'd leaf identity (normalized DNS SAN set) bound in CA-anchor
  /// mode. A reconnecting leaf must chain to the pinned CA AND present this same
  /// identity, so a different leaf from the same CA is rejected. Empty in
  /// leaf-fingerprint mode (the fingerprint already binds identity).
  const String& get_tofu_san() const { return tofu_san_; }

  /**
   * @brief Reset the stored trust anchor (CA or leaf fingerprint).
   *
   * Call this when the server's CA has changed legitimately so the device
   * re-captures it on the next connection. Re-capture is trust-on-first-use;
   * trigger it on a trusted network.
   */
  void reset_tofu() {
    tofu_ca_pem_ = "";
    tofu_fingerprint_ = "";
    tofu_pin_cn_ = "";
    tofu_pin_is_ca_ = false;
    tofu_san_ = "";
    save();
  }

  /// @brief Stash a candidate anchor seen during a handshake. Persisted only
  /// after the connection succeeds (commit_pending_tofu), so an unauthenticated
  /// MITM handshake cannot plant a trust anchor.
  void stash_pending_ca(const String& ca_pem, const String& cn) {
    pending_ca_pem_ = ca_pem;
    pending_fingerprint_ = "";
    pending_cn_ = cn;
    pending_is_ca_ = true;
    pending_valid_ = true;
  }
  void stash_pending_leaf(const String& fingerprint, const String& cn) {
    pending_ca_pem_ = "";
    pending_fingerprint_ = fingerprint;
    pending_cn_ = cn;
    pending_is_ca_ = false;
    pending_valid_ = true;
  }
  /// @brief Record the leaf's identity (SAN set) for the pending CA anchor,
  /// captured at depth 0 and committed alongside the CA on success.
  void set_pending_san(const String& san) { pending_san_ = san; }
  void clear_pending_tofu() {
    pending_ca_pem_ = "";
    pending_fingerprint_ = "";
    pending_cn_ = "";
    pending_san_ = "";
    pending_is_ca_ = false;
    pending_valid_ = false;
  }
  /// @brief True if a candidate CA has already been stashed this handshake.
  bool has_pending_ca() const { return pending_valid_ && pending_is_ca_; }
  /// @brief Persist a stashed anchor after a successful (authenticated)
  /// connection. Called from on_connected().
  void commit_pending_tofu() {
    if (!pending_valid_) {
      return;
    }
    tofu_pin_cn_ = pending_cn_;
    tofu_pin_is_ca_ = pending_is_ca_;
    if (pending_is_ca_) {
      tofu_ca_pem_ = pending_ca_pem_;
      tofu_fingerprint_ = "";
      tofu_san_ = pending_san_;  // bind the leaf identity in CA-anchor mode
    } else {
      tofu_fingerprint_ = pending_fingerprint_;
      tofu_ca_pem_ = "";
      tofu_san_ = "";  // leaf mode: the fingerprint binds identity
    }
    clear_pending_tofu();
    save();
  }

  /// @brief Flag a certificate rejection from the verify callback so the next
  /// disconnect surfaces as kSKWSCertificateError rather than a plain disconnect.
  void flag_cert_error() { cert_error_.store(true); }

  /// @brief Generation tag of the currently-valid client. The event handler
  /// compares the generation it was registered with against this to drop
  /// callbacks from a client that has been handed off for destruction.
  uint32_t client_generation() const { return client_generation_.load(); }

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

  unsigned long last_attempt_ms_ = 0;
  unsigned long attempt_wait_ms_ = 0;
  unsigned long connect_interval_ms_ = 2000;

  // SSL/TLS configuration
  bool ssl_enabled_ = false;
  bool tofu_enabled_ = true;  // TOFU enabled by default

  // Subscribe with ?sendMeta=all so metadata (zones, units, etc) is
  // pushed in-stream rather than requiring REST /meta polls.
  bool send_meta_enabled_ = true;

  // TOFU trust anchor. At most one of these is non-empty once captured:
  //   tofu_ca_pem_      — PEM of the pinned issuing CA (CA-anchor mode)
  //   tofu_fingerprint_ — SHA256 hex of the pinned leaf (leaf-fingerprint mode,
  //                       and the legacy on-disk format the migration reads)
  String tofu_fingerprint_ = "";  // SHA256 fingerprint in hex (64 chars)
  String tofu_ca_pem_ = "";       // PEM of pinned CA (CA-anchor mode)
  // TOFU'd leaf identity (normalized DNS SAN set) bound in CA-anchor mode, so a
  // reconnecting leaf must present the same identity AND chain to the pinned CA.
  // This is what makes CA pinning safe against a public CA (e.g. Let's Encrypt):
  // a valid leaf for a different name from the same CA fails the identity check.
  // Empty in leaf-fingerprint mode.
  String tofu_san_ = "";
  // Display-only identity of the pinned cert, captured at pin time (X.509
  // parsing is only available during the handshake). The CN is attacker-
  // controlled at capture and is bounded + sanitized before storage.
  String tofu_pin_cn_ = "";
  bool tofu_pin_is_ca_ = false;

  // Candidate anchor seen during the current connection attempt, persisted only
  // after the connection succeeds (commit_pending_tofu) so an unauthenticated
  // MITM handshake cannot plant a trust anchor.
  String pending_ca_pem_ = "";
  String pending_fingerprint_ = "";
  String pending_cn_ = "";
  String pending_san_ = "";
  bool pending_is_ca_ = false;
  bool pending_valid_ = false;

  // Set by the verify callback (transport task) on certificate rejection; read
  // by set_connection_state to surface kSKWSCertificateError. Atomic because the
  // callback and state changes may run on different cores.
  std::atomic<bool> cert_error_{false};

  TaskQueueProducer<SKWSConnectionState> connection_state_{
      SKWSConnectionState::kSKWSDisconnected, event_loop()};

  /// task_connection_state is used to track the internal task state which might
  /// be out of sync with the published connection state.
  SKWSConnectionState task_connection_state_ =
      SKWSConnectionState::kSKWSDisconnected;

  // Atomic for pointer-atomicity: the connect worker builds/stores it; the
  // SK/event-loop context loads it to send and reaps it via exchange(nullptr)
  // (single check-and-null, so no double-free). Writers are serialized
  // single-owner-at-a-time by auth_job_running_ / teardown_in_progress_.
  // NOTE: the atomic confers pointer-atomicity, not lifetime safety. send_delta
  // and sendTXT run on the event loop, serialized with detach_teardown (same
  // context), so they cannot hold a handle across its reap. Transport-task
  // senders (on_connected -> subscribe_listeners, on_receive_put's response)
  // are made safe by esp_websocket_client_stop() joining the transport task
  // before destroy() frees the struct. See #1033.
  std::atomic<esp_websocket_client_handle_t> client_{nullptr};
  // Bumped on every teardown so the (singleton) event handler can drop late
  // callbacks from a client that has been handed off for destruction. The
  // handler is registered with the generation current at build time; an event
  // whose generation != client_generation_ comes from a torn-down client.
  std::atomic<uint32_t> client_generation_{0};
  // True while a detached task is stopping+destroying a previous client.
  // Bring-up is deferred until it clears, so at most one client ever exists.
  std::atomic<bool> teardown_in_progress_{false};
  // True while a one-shot worker is running a (blocking) connect attempt
  // (mDNS / SSL-detect / access-request / poll legs). The dispatcher skips
  // while it is set, so at most one attempt runs at a time.
  std::atomic<bool> auth_job_running_{false};
  // Holds a handle whose detached reaper task failed to spawn (OOM); the event
  // loop retries the spawn each tick rather than reaping it synchronously (which
  // would block the loop). At most one at a time (teardown_in_progress_ defers
  // bring-up until it is reaped).
  std::atomic<esp_websocket_client_handle_t> pending_teardown_{nullptr};
  std::shared_ptr<SKDeltaQueue> sk_delta_queue_;
  /// @brief Emits the number of deltas sent since last report
  TaskQueueProducer<int> delta_tx_tick_producer_{0, event_loop(), 990};
  Integrator<int, int> delta_tx_count_producer_{1, 0, ""};
  Integrator<int, int> delta_rx_count_producer_{1, 0, ""};

  /// @brief millis() timestamp of the last oversize-delta-drop warning, used to
  /// rate-limit it when a device keeps producing a delta larger than the buffer.
  /// 0 = never logged, so the next drop logs immediately.
  uint32_t last_oversize_log_ms_ = 0;

  /// @brief A single received delta entry awaiting dispatch on the main task.
  ///
  /// `is_meta` is an out-of-band discriminator: value/put entries and meta
  /// entries are indistinguishable by JSON shape (a legitimate value can also
  /// be an object, e.g. `navigation.position`), so the kind is carried
  /// alongside the document rather than stamped into it.
  struct ReceivedUpdate {
    bool is_meta = false;
    JsonDocument doc;
  };

  StaticSemaphore_t received_updates_semaphore_buffer_;
  SemaphoreHandle_t received_updates_semaphore_ =
      xSemaphoreCreateRecursiveMutexStatic(&received_updates_semaphore_buffer_);
  std::list<ReceivedUpdate> received_updates_{};

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

  /// @brief Push a received delta entry onto the queue, enforcing a per-kind
  /// budget so a metadata burst (one meta-delta per subscribed path at
  /// subscribe time when `sendMeta=all` is enabled) cannot evict pending value
  /// deltas, and vice versa. Caller must hold `received_updates_semaphore_`.
  void enqueue_received_update(ReceivedUpdate&& update);

  /////////////////////////////////////////////////////////
  // main task methods

  void process_received_updates();

  /////////////////////////////////////////////////////////
  // SK websocket connection methods

#ifndef SENSESP_SSL_SUPPORT
  // Validate the auth token over plain HTTP before opening the WebSocket
  // stream. Only used on non-SSL builds; SSL builds validate it on the upgrade
  // itself (see connect()) to avoid a second TLS handshake that fragments the
  // heap.
  void test_token(const String host, const uint16_t port);
#endif
  void send_access_request(const String host, const uint16_t port);
  void poll_access_request(const String host, const uint16_t port,
                           const String href);
  void connect_ws(const String& host, const uint16_t port);
  /// @brief Hand client_ to a detached one-shot task that stops+destroys it, so
  /// the blocking teardown never runs on the caller's context. Nulls client_ and
  /// bumps client_generation_ / sets teardown_in_progress_ immediately; no-op if
  /// client_ is null. If the reaper task can't be spawned (OOM), the handle is
  /// stashed in pending_teardown_ and the spawn is retried on the next event-loop
  /// tick -- never reaped synchronously, which would block the loop.
  void detach_teardown();
  /// @brief Spawn the detached reaper for `old`; on spawn failure (OOM) stash it
  /// in pending_teardown_ for a later retry instead of blocking the loop with a
  /// synchronous reap. No-op if `old` is null. Call on the event-loop context.
  void reap_async(esp_websocket_client_handle_t old);
  /// @brief Body of the detached teardown task (stop+destroy+self-delete).
  static void teardown_task(void* arg);
  /// @brief The (blocking) connect attempt — mDNS resolve, SSL detect, and the
  /// access-request / poll / connect_ws leg. Run on a one-shot worker task so
  /// none of it blocks the SK/event-loop context. connect() is the dispatcher.
  void run_connect_attempt();
  static void connect_worker(void* arg);
  void subscribe_listeners();
  bool get_mdns_service(String& server_address, uint16_t& server_port);
  bool detect_ssl();

  void set_connection_state(SKWSConnectionState state) {
    // A certificate rejection during this attempt (flagged by the verify
    // callback) is surfaced distinctly instead of as a generic disconnect.
    // This is the single chokepoint, so it covers all four TLS paths — the
    // three esp_http_client requests and the websocket — without touching each
    // call site. A fresh attempt (authorizing/connecting) or success clears the
    // flag.
    if (state == SKWSConnectionState::kSKWSDisconnected && cert_error_.load()) {
      state = SKWSConnectionState::kSKWSCertificateError;
    } else if (state != SKWSConnectionState::kSKWSDisconnected &&
               state != SKWSConnectionState::kSKWSCertificateError) {
      cert_error_.store(false);
    }
    task_connection_state_ = state;
    connection_state_.set(state);
  }
  SKWSConnectionState get_connection_state() { return task_connection_state_; }

  void schedule_reconnect() {
    last_attempt_ms_ = millis();
    attempt_wait_ms_ =
        connect_interval_ms_ + (esp_random() % (connect_interval_ms_ / 4 + 1));
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
         "\"tofu_pin_cn\":{\"title\":\"Pinned Certificate\",\"type\":\"string\",\"readOnly\":true},"
         "\"tofu_pin_is_ca\":{\"title\":\"Pinned as CA\",\"type\":\"boolean\",\"readOnly\":true},"
         "\"send_meta_enabled\":{\"title\":\"Subscribe with sendMeta=all\","
         "\"description\":\"Request metadata deltas (units, zones, displayName, displayUnits) over the WS stream. Disable only for constrained clients that ignore them.\","
         "\"type\":\"boolean\"}"
         "}}";
}

inline bool ConfigRequiresRestart(const SKWSClient& obj) { return true; }

}  // namespace sensesp

#endif
