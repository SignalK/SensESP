#ifndef SENSESP_NET_BLE_BLE_SIGNALK_GATEWAY_H_
#define SENSESP_NET_BLE_BLE_SIGNALK_GATEWAY_H_

#include <Arduino.h>
#include <ArduinoJson.h>

#include <atomic>
#include <map>
#include <memory>
#include <mutex>
#include <vector>

#include "esp_websocket_client.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

#include "sensesp/net/ble/gatt_session.h"

#include "sensesp/net/ble/ble_advertisement.h"
#include "sensesp/net/ble/ble_provisioner.h"
#include "sensesp/signalk/signalk_ws_client.h"

namespace sensesp {

/**
 * @brief Configuration for BLESignalKGateway.
 *
 * Defined at namespace scope rather than nested inside the class so
 * that default member initializers on its fields are fully available
 * when the BLESignalKGateway constructor declaration is parsed.
 * (C++ forbids a class's default member initializers from being used
 * inside the enclosing class's other default-argument expressions
 * while the class is still incomplete.)
 */
struct BLESignalKGatewayConfig {
  /// HTTP POST interval in ms. Default 2000.
  uint32_t post_interval_ms = 2000;

  /// Control WS status-message interval in ms. Default 30000.
  uint32_t status_interval_ms = 30000;

  /// Max pending advertisements buffered between POST flushes.
  /// If more arrive than this before the next flush, the buffer
  /// is drained to drop_oldest_keep_newest to avoid unbounded
  /// memory growth. Default 500.
  size_t max_pending_ads = 500;

  /// Gateway firmware version string sent in the hello message.
  /// Empty string means use the SensESP library version.
  String firmware_version = "";

  /// Max GATT sessions advertised in the hello message. Currently
  /// always 0 because GATT support is not implemented yet — the
  /// server will not send gatt_subscribe commands to a gateway
  /// that advertises 0 slots.
  int max_gatt_sessions = 0;
};

/**
 * @brief Bridge between a BLEProvisioner and signalk-server's
 *        ble-provider-api.
 *
 * Takes a BLEProvisioner (the source of advertisements and — in
 * future — GATT sessions) plus an SKWSClient (the main SensESP SK
 * delta connection, used only for credentials piggybacking) and
 * speaks the signalk-server ble-provider-api dual-channel protocol:
 *
 *   * HTTP POST /signalk/v2/api/ble/gateway/advertisements
 *     Bearer-authenticated JSON batches of recently-seen BLE
 *     advertisements. Sent periodically from a background task.
 *
 *   * WebSocket /signalk/v2/api/ble/gateway/ws?token=<JWT>
 *     Separate long-lived WebSocket for the gateway control
 *     protocol: hello handshake on connect, periodic status frames,
 *     and (in future) gatt_subscribe / gatt_write / gatt_close
 *     commands from the server.
 *
 * Both channels reuse the JWT that SKWSClient obtained via the
 * main access-request flow (via SKWSClient::get_auth_token()) so the
 * gateway does not have to run a second parallel access request.
 *
 * ## Lifecycle
 *
 * Construct with a BLEProvisioner and SKWSClient shared_ptr. Call
 * start() to begin gateway services. Gateway services are gated on
 * SK connection state: the background HTTP POST task only posts
 * when the main SK websocket is connected, and the control
 * WebSocket is (re)connected whenever the main SK websocket comes
 * up, disconnected when it goes down.
 *
 * Advertisement collection is independent of SK state — the
 * provisioner emits advertisements into an internal buffer as soon
 * as the scanner is running, and they wait there until the HTTP
 * POST task gets a chance to forward them (or drops them if the
 * buffer grows too large).
 *
 * ## GATT session handling
 *
 * gatt_subscribe / gatt_write / gatt_close commands received on the
 * control WS are currently logged and ignored. Full GATT client
 * support is out of scope for the first cut and will land in a
 * follow-up. The control WS and HTTP POST channels are still
 * useful on their own for running signalk-server against a gateway
 * that only relays advertisements (e.g. for Ruuvi / Victron beacons
 * that do not need GATT connections).
 */
class BLESignalKGateway {
 public:
  using Config = BLESignalKGatewayConfig;

  BLESignalKGateway(std::shared_ptr<BLEProvisioner> ble,
                    std::shared_ptr<SKWSClient> sk_client,
                    BLESignalKGatewayConfig config = BLESignalKGatewayConfig{});
  ~BLESignalKGateway();

  BLESignalKGateway(const BLESignalKGateway&) = delete;
  BLESignalKGateway& operator=(const BLESignalKGateway&) = delete;

  /**
   * @brief Start gateway services.
   *
   * Attaches observers to the BLE provisioner (to receive
   * advertisements) and to the SKWSClient's connection state
   * producer (to start/stop the control WS and HTTP POST task).
   * Also starts the BLE scan on the provided provisioner.
   *
   * Safe to call once; subsequent calls are no-ops.
   */
  void start();

  /**
   * @brief Stop gateway services.
   *
   * Stops the HTTP POST task, tears down the control WS, detaches
   * observers, and stops the BLE scan. The BLE provisioner itself
   * is not destroyed — it remains usable by other subscribers.
   */
  void stop();

  // --- debug counters ---
  uint32_t advertisements_received() const { return adv_received_count_; }
  uint32_t advertisements_posted() const { return adv_posted_count_; }
  uint32_t advertisements_dropped() const { return adv_dropped_count_; }
  uint32_t http_post_success() const { return http_post_success_; }
  uint32_t http_post_fail() const { return http_post_fail_; }
  uint32_t control_ws_connected_count() const { return ws_connected_count_; }
  bool control_ws_connected() const { return ws_connected_.load(); }

 private:
  // Attached to ble_provisioner_'s ValueProducer<BLEAdvertisement>
  // via attach(). Called synchronously from the provisioner's GAP
  // event callback for each advertisement.
  void on_advertisement();

  // Starts the control WS if it is not already running and the SK
  // server address + token are available.
  void init_control_ws();

  // Tears down the control WS.
  void destroy_control_ws();

  // Build and send a hello JSON message on the control WS.
  void send_hello();

  // Build and send a status JSON message on the control WS.
  void send_status();

  // Dispatch an incoming control WS message.
  void handle_control_ws_message(uint8_t* payload, size_t length);

  // GATT command handlers.
  void handle_gatt_subscribe(JsonDocument& doc);
  void handle_gatt_write(JsonDocument& doc);
  void handle_gatt_close(JsonDocument& doc);

  // Send a JSON message on the control WS (thread-safe).
  void send_control_json(JsonDocument& doc);

  // GATT session state machine progression.
  void gatt_run_init_writes(GATTSession* session);
  void gatt_run_subscribes(GATTSession* session);
  void gatt_start_timers(GATTSession* session);
  void gatt_cleanup_session(const String& session_id);

  // FreeRTOS timer callbacks for poll and periodic write.
  static void poll_timer_cb(TimerHandle_t timer);
  static void periodic_write_timer_cb(TimerHandle_t timer);

  // Drain pending_ads_ and POST them to signalk-server.
  void post_pending_advertisements();

  // FreeRTOS task that runs post_pending_advertisements() on a
  // timer. Started in start(), stopped in stop().
  static void post_task_entry(void* arg);
  void post_task_loop();

  // esp_websocket_client event handler trampoline + instance method.
  static void control_ws_event_trampoline(void* handler_args,
                                          esp_event_base_t base,
                                          int32_t event_id, void* event_data);
  void handle_control_ws_event(int32_t event_id, void* event_data);

  std::shared_ptr<BLEProvisioner> ble_provisioner_;
  std::shared_ptr<SKWSClient> sk_client_;
  Config config_;

  std::atomic<bool> started_{false};
  std::atomic<bool> sk_connected_{false};
  std::atomic<bool> ws_connected_{false};

  // Pending advertisements waiting to be POSTed. Guarded by
  // pending_ads_mutex_ because the producer (BLE GAP event) and the
  // consumer (background HTTP POST task) run on different FreeRTOS
  // tasks.
  std::vector<BLEAdvertisement> pending_ads_;
  SemaphoreHandle_t pending_ads_mutex_ = nullptr;

  // Control WebSocket client.
  esp_websocket_client_handle_t control_ws_ = nullptr;
  SemaphoreHandle_t control_ws_mutex_ = nullptr;

  // Background POST task handle.
  TaskHandle_t post_task_ = nullptr;
  std::atomic<bool> post_task_should_run_{false};

  // Active GATT sessions keyed by session_id.
  std::map<String, std::unique_ptr<GATTSession>> gatt_sessions_;
  SemaphoreHandle_t gatt_sessions_mutex_ = nullptr;
  // Suppress scan watchdog while GATT connections are being established.
  std::atomic<bool> scan_suppressed_{false};

  // Counters.
  std::atomic<uint32_t> adv_received_count_{0};
  std::atomic<uint32_t> adv_posted_count_{0};
  std::atomic<uint32_t> adv_dropped_count_{0};
  std::atomic<uint32_t> http_post_success_{0};
  std::atomic<uint32_t> http_post_fail_{0};
  std::atomic<uint32_t> ws_connected_count_{0};
};

}  // namespace sensesp

#endif  // SENSESP_NET_BLE_BLE_SIGNALK_GATEWAY_H_
