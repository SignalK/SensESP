#ifndef _ws_client_H_
#define _ws_client_H_
#include <WebSocketsClient.h>

#include <functional>
#include <set>

#include "sensesp.h"
#include "signalk/signalk_delta.h"
#include "system/configurable.h"
#include "system/valueproducer.h"
#include "system/observablevalue.h"

static const char* NULL_AUTH_TOKEN = "";

enum WSConnectionState {
  kWSDisconnected,
  kWSAuthorizing,
  kWSConnecting,
  kWSConnected
};

class WSClient : public Configurable, public ValueProducer<WSConnectionState> {
 public:
  WSClient(String config_path, SKDelta* sk_delta, String server_address,
           uint16_t server_port, String sk_permission = "readwrite");
  void enable();
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

  const String get_server_address() const { return server_address; }
  const uint16_t get_server_port() const { return server_port; }
  const WSConnectionState get_connection_state() const {
    return connection_state.get();
  }

  virtual void get_configuration(JsonObject& doc) override final;
  virtual bool set_configuration(const JsonObject& config) override final;
  virtual String get_config_schema() override;

  /*
   * Return a delta update ValueProducer that produces the number of sent deltas
   * (ordinarily always 1)
   */
  ValueProducer<int>& get_delta_count_producer() { return delta_count_producer; };

 private:
  String server_address = "";
  uint16_t server_port = 80;
  String preset_server_address = "";
  uint16_t preset_server_port = 0;
  String client_id = "";
  String polling_href = "";
  String auth_token = NULL_AUTH_TOKEN;
  String sk_permission;
  bool server_detected = false;
  bool token_test_success = false;
  
  ObservableValue<WSConnectionState> connection_state = kWSDisconnected;
  WiFiClient wifi_client;
  WebSocketsClient client;
  SKDelta* sk_delta;
  ObservableValue<int> delta_count_producer = 0;
  void connect_loop();
  void test_token(const String host, const uint16_t port);
  void send_access_request(const String host, const uint16_t port);
  void poll_access_request(const String host, const uint16_t port,
                           const String href);
  void connect_ws(const String host, const uint16_t port);
  void subscribe_listeners();
  bool get_mdns_service(String& server_address, uint16_t& server_port);
};

#endif
