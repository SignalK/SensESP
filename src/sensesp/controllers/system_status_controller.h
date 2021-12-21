#ifndef _SYSTEM_STATUS_CONTROLLER_H_
#define _SYSTEM_STATUS_CONTROLLER_H_

#include "sensesp/net/networking.h"
#include "sensesp/net/ws_client.h"
#include "sensesp/system/valueproducer.h"

namespace sensesp {

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

 protected:
  void update_state(const SystemStatus new_state) {
    current_state_ = new_state;
    this->emit(new_state);
  }

 private:
  SystemStatus current_state_ = SystemStatus::kWifiNoAP;
};

}  // namespace sensesp

#endif
