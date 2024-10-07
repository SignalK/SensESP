#ifndef SENSESP_SRC_SENSESP_SIGNALK_SIGNALK_WS_CLIENT_H_
#define SENSESP_SRC_SENSESP_SIGNALK_SIGNALK_WS_CLIENT_H_

#include "sensesp.h"

#include <WiFi.h>
#include <esp_websocket_client.h>
#include <functional>
#include <set>
#include <list>

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
class SKWSClient : virtual public FileSystemSaveable,
                   virtual public ValueProducer<SKWSConnectionState> {
 public:
  /////////////////////////////////////////////////////////
  // main task methods

  SKWSClient(const String& config_path, SKDeltaQueue* sk_delta_queue,
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
  void send_delta();

  /**
   * Sends the specified payload to the server over the websocket
   * this client is connected to. If no connection currently exist,
   * the call is safely ignored.
   */
  void sendTXT(String& payload);

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

  TaskQueueProducer<SKWSConnectionState> connection_state_ =
      TaskQueueProducer<SKWSConnectionState>(
          SKWSConnectionState::kSKWSDisconnected,
          event_loop());

  /// task_connection_state is used to track the internal task state which might
  /// be out of sync with the published connection state.
  SKWSConnectionState task_connection_state_ =
      SKWSConnectionState::kSKWSDisconnected;

  WiFiClient wifi_client_{};
  esp_websocket_client_handle_t client_{};
  SKDeltaQueue* sk_delta_queue_;
  /// @brief Emits the number of deltas sent since last report
  TaskQueueProducer<int> delta_tx_tick_producer_ =
      TaskQueueProducer<int>(0, event_loop(), 990);
  Integrator<int, int> delta_tx_count_producer_{1, 0, ""};
  Integrator<int, int> delta_rx_count_producer_{1, 0, ""};

  SemaphoreHandle_t received_updates_semaphore_ =
      xSemaphoreCreateRecursiveMutex();
  std::list<JsonDocument> received_updates_{};

  /////////////////////////////////////////////////////////
  // methods for all tasks

  bool take_received_updates_semaphore(unsigned long int timeout_ms = 0) {
    if (timeout_ms == 0) {
      return xSemaphoreTake(received_updates_semaphore_, portMAX_DELAY) ==
             pdTRUE;
    } else {
      return xSemaphoreTake(received_updates_semaphore_, timeout_ms) == pdTRUE;
    }
  }
  void release_received_updates_semaphore() {
    xSemaphoreGive(received_updates_semaphore_);
  }

  /////////////////////////////////////////////////////////
  // main task methods

  void process_received_updates();

  /////////////////////////////////////////////////////////
  // SKWSClient task methods

  void connect_loop();
  void test_token(const String host, const uint16_t port);
  void send_access_request(const String host, const uint16_t port);
  void poll_access_request(const String host, const uint16_t port,
                           const String href);
  void connect_ws(const String& host, const uint16_t port);
  void subscribe_listeners();
  bool get_mdns_service(String& server_address, uint16_t& server_port);

  void set_connection_state(SKWSConnectionState state) {
    task_connection_state_ = state;
    connection_state_.set(state);
  }
  SKWSConnectionState get_connection_state() { return task_connection_state_; }
};

inline bool ConfigRequiresRestart(const SKWSClient& obj) {
  return true;
}

}  // namespace sensesp

#endif
