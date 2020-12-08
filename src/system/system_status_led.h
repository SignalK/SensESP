#ifndef _LED_CONTROLLER_H_
#define _LED_CONTROLLER_H_

#include "led_blinker.h"
#include "system/system_status_consumer.h"

/**
 * SystemStatusLed consumes the networking and websocket states and delta counts
 * and updates the device LED accordingly. Inherit this class and override
 * the methods to customize the behavior.
 */
class SystemStatusLed : public SystemStatusConsumer {
 protected:
  PatternBlinker* blinker;

  virtual void set_wifi_no_ap();
  virtual void set_wifi_disconnected();
  virtual void set_wifi_connected();
  virtual void set_wifimanager();

  virtual void set_ws_disconnected();
  virtual void set_ws_authorizing();
  virtual void set_ws_connecting();
  virtual void set_ws_connected();

 public:
  SystemStatusLed(int pin);

  // ValueConsumer interface for ValueConsumer<WifiState> (Networking object
  // state updates)
  virtual void set_input(WifiState new_value,
                         uint8_t input_channel = 0) override;
  // ValueConsumer interface for ValueConsumer<WSConnectionState>
  // (WSClient object state updates)
  virtual void set_input(WSConnectionState new_value,
                         uint8_t input_channel = 0) override;
  // ValueConsumer interface for ValueConsumer<int> (delta count producer
  // updates)
  virtual void set_input(int new_value, uint8_t input_channel = 0) override;
};

#endif