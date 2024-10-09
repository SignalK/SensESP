#ifndef _SYSTEM_STATUS_CONTROLLER_H_
#define _SYSTEM_STATUS_CONTROLLER_H_

#include "sensesp/net/networking.h"
#include "sensesp/signalk/signalk_ws_client.h"
#include "sensesp/system/lambda_consumer.h"
#include "sensesp/system/valueproducer.h"

namespace sensesp {

enum class SystemStatus {
  kWifiNoAP = 100,
  kWifiDisconnected,
  kWifiManagerActivated,
  kSKWSDisconnected,
  kSKWSAuthorizing,
  kSKWSConnecting,
  kSKWSConnected
};

/**
 * @brief Base class for a controller that can react to system status events
 *
 * Classes inheriting from SystemStatusController should override the
 * set_wifi_* and set_ws_* methods to take the relevant action when such
 * an event occurs.
 */
class SystemStatusController : public ValueProducer<SystemStatus> {
 public:
  SystemStatusController() {}

  ValueConsumer<WiFiState>& get_wifi_state_consumer() {
    return wifi_state_consumer_;
  }

  ValueConsumer<SKWSConnectionState>& get_ws_connection_state_consumer() {
    return ws_connection_state_consumer_;
  }

 protected:
  void set_wifi_state(const WiFiState& new_value) {
    switch (new_value) {
      case WiFiState::kWifiNoAP:
        this->update_state(SystemStatus::kWifiNoAP);
        break;
      case WiFiState::kWifiDisconnected:
        this->update_state(SystemStatus::kWifiDisconnected);
        break;
      case WiFiState::kWifiConnectedToAP:
      case WiFiState::kWifiAPModeActivated:
        this->update_state(SystemStatus::kSKWSDisconnected);
        break;
      case WiFiState::kWifiManagerActivated:
        this->update_state(SystemStatus::kWifiManagerActivated);
        break;
    }
  }

  void set_sk_ws_connection_state(const SKWSConnectionState& new_value) {
    switch (new_value) {
      case SKWSConnectionState::kSKWSDisconnected:
        if (current_state_ != SystemStatus::kWifiDisconnected &&
            current_state_ != SystemStatus::kWifiNoAP &&
            current_state_ != SystemStatus::kWifiManagerActivated) {
          // Wifi disconnection states override the higher level protocol state
          this->update_state(SystemStatus::kSKWSDisconnected);
        }
        break;
      case SKWSConnectionState::kSKWSConnecting:
        this->update_state(SystemStatus::kSKWSConnecting);
        break;
      case SKWSConnectionState::kSKWSAuthorizing:
        this->update_state(SystemStatus::kSKWSAuthorizing);
        break;
      case SKWSConnectionState::kSKWSConnected:
        this->update_state(SystemStatus::kSKWSConnected);
        break;
    }
  }

  LambdaConsumer<WiFiState> wifi_state_consumer_{
      [this](const WiFiState& new_value) { this->set_wifi_state(new_value); }};
  LambdaConsumer<SKWSConnectionState> ws_connection_state_consumer_{
      [this](const SKWSConnectionState& new_value) {
        this->set_sk_ws_connection_state(new_value);
      }};

  void update_state(const SystemStatus new_state) {
    current_state_ = new_state;
    this->emit(new_state);
  }

 private:
  SystemStatus current_state_ = SystemStatus::kWifiNoAP;
};

}  // namespace sensesp

#endif
