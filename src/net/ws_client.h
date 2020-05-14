#ifndef _ws_client_H_
#define _ws_client_H_
#include <set>
#include <functional>

#include <WebSocketsClient.h>

#include "sensesp.h"
#include "system/configurable.h"
#include "signalk/signalk_delta.h"

static const char* NULL_AUTH_TOKEN = "";

enum ConnectionState { disconnected, connecting, connected };

class WSClient : public Configurable {
 public:
  WSClient(String config_path, SKDelta* sk_delta,
            std::function<void(bool)> connected_cb,
            void_cb_func delta_cb);
  void enable();
  void on_disconnected();
  void on_error();
  void on_connected(uint8_t * payload);
  void on_receive_delta(uint8_t * payload);
  void connect();
  void loop();
  bool is_connected();
  void restart();
  void send_delta();

  virtual JsonObject& get_configuration(JsonBuffer& buf) override final;
  virtual bool set_configuration(const JsonObject& config) override final;
  virtual String get_config_schema() override;

 private:
  String server_address = "";
  uint16_t server_port = 80;
  String client_id = "";
  String polling_href = "";
  String auth_token = NULL_AUTH_TOKEN;
  bool server_detected = false;

  // FIXME: replace with a single connection_state enum
  ConnectionState connection_state = disconnected;
  WebSocketsClient client;
  SKDelta* sk_delta;
  void connect_loop();
  void test_token(const String host, const uint16_t port);
  void send_access_request(const String host, const uint16_t port);
  void poll_access_request(const String host, const uint16_t port, const String href);
  void connect_ws(const String host, const uint16_t port);
  void subscribe_listeners();
  std::function<void(bool)> connected_cb;
  void_cb_func delta_cb;
  bool get_mdns_service(String &server_address, uint16_t& server_port);
};

#endif
