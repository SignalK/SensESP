#include "system_status_controller.h"

namespace sensesp {

void SystemStatusController::set(WiFiState new_value,
                                       uint8_t input_channel) {
  // FIXME: If pointers to member functions would be held in an array,
  // this would be a simple array dereferencing
  switch (new_value) {
    case WiFiState::kWifiNoAP:
      this->update_state(SystemStatus::kWifiNoAP);
      break;
    case WiFiState::kWifiDisconnected:
      this->update_state(SystemStatus::kWifiDisconnected);
      break;
    case WiFiState::kWifiConnectedToAP:
    case WiFiState::kWifiAPModeActivated:
      this->update_state(SystemStatus::kWSDisconnected);
      break;
    case WiFiState::kWifiManagerActivated:
      this->update_state(SystemStatus::kWifiManagerActivated);
      break;
  }
}

void SystemStatusController::set(WSConnectionState new_value,
                                       uint8_t input_channel) {
  switch (new_value) {
    case WSConnectionState::kWSDisconnected:
      if (current_state_ != SystemStatus::kWifiDisconnected &&
          current_state_ != SystemStatus::kWifiNoAP &&
          current_state_ != SystemStatus::kWifiManagerActivated) {
        // Wifi disconnection states override the higher level protocol state
        this->update_state(SystemStatus::kWSDisconnected);
      }
      break;
    case WSConnectionState::kWSConnecting:
      this->update_state(SystemStatus::kWSConnecting);
      break;
    case WSConnectionState::kWSAuthorizing:
      this->update_state(SystemStatus::kWSAuthorizing);
      break;
    case WSConnectionState::kWSConnected:
      this->update_state(SystemStatus::kWSConnected);
      break;
  }
}

}  // namespace sensesp
