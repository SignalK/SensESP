#ifndef _LED_CONTROLLER_H_
#define _LED_CONTROLLER_H_

#include "led_blinker.h"
#include "net/networking.h"
#include "net/ws_client.h"
#include "valueconsumer.h"

class LedController : public ValueConsumer<WifiState>,
                      public ValueConsumer<WSConnectionState>,
                      public ValueConsumer<int> {
 private:
  PatternBlinker* blinker;

  void set_wifi_no_ap();
  void set_wifi_disconnected();
  void set_wifi_connected();
  void set_wifimanager();

  void set_ws_disconnected();
  void set_ws_authorizing();
  void set_ws_connecting();
  void set_ws_connected();

 public:
  LedController(int pin);

  // ValueConsumer interface for ValueConsumer<WifiState> (Networking object
  // state updates)
  void set_input(WifiState new_value, uint8_t input_channel = 0) override;
  // ValueConsumer interface for ValueConsumer<WSConnectionState>
  // (WSClient object state updates)
  void set_input(WSConnectionState new_value,
                 uint8_t input_channel = 0) override;
  // ValueConsumer interface for ValueConsumer<int> (delta count producer
  // updates)
  void set_input(int new_value, uint8_t input_channel = 0) override;
};

#endif