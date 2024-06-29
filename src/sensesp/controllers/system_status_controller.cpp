#include "system_status_controller.h"

namespace sensesp {

void SystemStatusController::set(const WiFiState& new_value) {
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
      this->update_state(SystemStatus::kSKWSDisconnected);
      break;
    case WiFiState::kWifiManagerActivated:
      this->update_state(SystemStatus::kWifiManagerActivated);
      break;
  }
}

void SystemStatusController::set(const SKWSConnectionState& new_value) {
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

}  // namespace sensesp
