#ifndef _ws_client_H_
#define _ws_client_H_
#include <WebSocketsClient.h>

#include <functional>
#include <set>

#include "sensesp.h"
#include "sensesp/signalk/signalk_delta_queue.h"
#include "sensesp/system/configurable.h"
#include "sensesp/system/observablevalue.h"
#include "sensesp/system/startable.h"
#include "sensesp/system/valueproducer.h"

namespace sensesp {

static const char* NULL_AUTH_TOKEN = "";

enum class WSConnectionState {
  kWSDisconnected,
  kWSAuthorizing,
  kWSConnecting,
  kWSConnected
};

/**
 * @brief The websocket connection to the Signal K server.
 * @see SensESPApp
 */
class WSClient : public Configurable,
                 public Startable,
                 public ValueProducer<WSConnectionState> {
 public:
  WSClient(String config_path, SKDeltaQueue* sk_delta_queue,
           String server_address, uint16_t server_port);
  virtual void start() override;
  void on_disconnected();
  void on_error();
  void on_connected(uint8_t* payload);
  void on_receive_delta(uint8_t* payload);
  void on_receive_updates(DynamicJsonDocument& message);
  void on_receive_put(DynamicJsonDocument& message);
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

  const String get_server_address() const { return server_address_; }
  const uint16_t get_server_port() const { return server_port_; }
  const WSConnectionState get_connection_state() const {
    return connection_state_.get();
  }

  virtual void get_configuration(JsonObject& doc) override final;
  virtual bool set_configuration(const JsonObject& config) override final;
  virtual String get_config_schema() override;

  /*
   * Return a delta update ValueProducer that produces the number of sent deltas
   * (ordinarily always 1)
   */
  ValueProducer<int>& get_delta_count_producer() {
    return delta_count_producer_;
  };

 private:
  String server_address_ = "";
  uint16_t server_port_ = 80;
  String preset_server_address_ = "";
  uint16_t preset_server_port_ = 0;
  String client_id_ = "";
  String polling_href_ = "";
  String auth_token_ = NULL_AUTH_TOKEN;
  bool server_detected_ = false;
  bool token_test_success_ = false;

  ObservableValue<WSConnectionState> connection_state_ =
      WSConnectionState::kWSDisconnected;
  WiFiClient wifi_client_;
  WebSocketsClient client_;
  SKDeltaQueue* sk_delta_queue_;
  ObservableValue<int> delta_count_producer_ = 0;
  void connect_loop();
  void test_token(const String host, const uint16_t port);
  void send_access_request(const String host, const uint16_t port);
  void poll_access_request(const String host, const uint16_t port,
                           const String href);
  void connect_ws(const String host, const uint16_t port);
  void subscribe_listeners();
  bool get_mdns_service(String& server_address, uint16_t& server_port);
};

}  // namespace sensesp

#endif
