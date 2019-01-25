#ifndef _ws_client_H_
#define _ws_client_H_

#include "../system/signal_k.h"
#include "../sensesp.h"

#include <WebSocketsClient.h>


class WSClient {
    String host = "";
    uint16_t port = 80;
    String path = "/signalk/v1/stream";
    String auth_token;
    bool connected = false;
    WebSocketsClient client;
    SKDelta* sk_delta;
    void_cb_func connected_cb;
    void_cb_func disconnected_cb;
    void_cb_func delta_cb;
    bool get_mdns_service(String &host, uint16_t& port);

  public:
    WSClient(SKDelta* sk_delta,
             void_cb_func connected_cb,
             void_cb_func disconnected_cb,
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
};

#endif
