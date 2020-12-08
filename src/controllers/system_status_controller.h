#ifndef _SYSTEM_STATUS_CONTROLLER_H_
#define _SYSTEM_STATUS_CONTROLLER_H_

#include "system/system_status_consumer.h"

/**
 * @brief Base class for a controller that can react to system status events
 * 
 * Classes inheriting from SystemStatusController should override the
 * set_wifi_* and set_ws_* methods to take the relevant action when such
 * an event occurs.
 */
class SystemStatusController : public SystemStatusConsumer {
 protected:
  virtual void set_wifi_no_ap() {}
  virtual void set_wifi_disconnected() {}
  virtual void set_wifi_connected() {}
  virtual void set_wifimanager() {}

  virtual void set_ws_disconnected() {}
  virtual void set_ws_authorizing() {}
  virtual void set_ws_connecting() {}
  virtual void set_ws_connected() {}

 public:
  SystemStatusController() {}

  /// ValueConsumer interface for ValueConsumer<WifiState> (Networking object
  /// state updates)
  virtual void set_input(WifiState new_value,
                         uint8_t input_channel = 0) override;
  /// ValueConsumer interface for ValueConsumer<WSConnectionState>
  /// (WSClient object state updates)
  virtual void set_input(WSConnectionState new_value,
                         uint8_t input_channel = 0) override;
  /// ValueConsumer interface for ValueConsumer<int> (delta count producer
  /// updates)
  virtual void set_input(int new_value, uint8_t input_channel = 0) override {}
};


#endif