#ifndef _SYSTEM_STATUS_CONTROLLER_H_
#define _SYSTEM_STATUS_CONTROLLER_H_

#include "net/networking.h"
#include "net/ws_client.h"
#include "system/valueproducer.h"

enum class SystemStatus {
  kWifiNoAP = 100,
  kWifiDisconnected,
  kWifiManagerActivated,
  kWSDisconnected,
  kWSAuthorizing,
  kWSConnecting,
  kWSConnected
};

/**
 * @brief Base class for a controller that can react to system status events
 *
 * Classes inheriting from SystemStatusController should override the
 * set_wifi_* and set_ws_* methods to take the relevant action when such
 * an event occurs.
 */
class SystemStatusController : public ValueConsumer<WifiState>,
                               public ValueConsumer<WSConnectionState>,
                               public ValueProducer<SystemStatus> {
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
};

#endif